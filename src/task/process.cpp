/*
 * process.cpp
 *
 *  Created on: 13/07/2016
 *      Author: Miguel
 */

#include <system.h>
#include <module.h>
#include <libc/tree.h>

namespace Kernel {
namespace Task {

/****************************************************************************/
/************ Tasking/Process prototype functions / externs *****************/
/****************************************************************************/
extern void task_return_grave(void);
extern char is_tasking_initialized;
/****************************************************************************/

/**********************************************************/
/************ Tasking / Process Variables *****************/
/**********************************************************/
uint16_t next_pid = 1;
tree_t * tasktree;
list_t * tasklist;
list_t * taskqueue;

/* Locks: */
static spin_lock_t tree_lock = { 0 };
static spin_lock_t process_queue_lock = { 0 };
static spin_lock_t wait_lock_tmp = { 0 };
static spin_lock_t sleep_lock = { 0 };
/**********************************************************/

/*************************************************************/
/*************** Task tree getters and setters ***************/
/*************************************************************/
int fetch_index = 0;
int tasklist_size = 0;

void task_removefromtree(pid_t pid) {
	/* Remove the task from the list, then cleanup the rest (remove process from tree, deallocate task's stack and more) */
	list_remove(tasklist, pid);
	tasklist_size--;
}

void task_addtotree(task_t * parent_task, task_t * new_task) {
	/* Add task to tree: */
	tree_node_t * tree_entry = tree_node_create(new_task);
	new_task->tree_entry = tree_entry;

	spin_lock(tree_lock);

	tree_node_insert_child_node(tasktree, parent_task->tree_entry, tree_entry);
	/* Insert into switch queue: */
	list_insert(tasklist, new_task);
	tasklist_size++;

	spin_unlock(tree_lock);
}

task_t * fetch_next_task(void) {
	if(fetch_index > tasklist_size)
		fetch_index = 0;
	return (task_t*)list_get(tasklist, fetch_index++)->value;
}

task_t * task_from_pid(pid_t pid) {
	if(pid < 0) return 0;
	return (task_t*)list_get(tasklist, pid)->value;
}

uint32_t current_task_getpid(void) {
	return current_task->pid;
}

task_t * current_task_get(void) {
	return (task_t*)current_task;
}

pid_t get_next_pid(char restart_pid) {
	if(restart_pid) next_pid = 1;
	return next_pid++;
}
/*************************************************************/

/**********************/
/***** x86 Stack: *****/
/**********************/
void x86_set_stack(uint32_t * stack_ptr, task_t * task) {
	/* Parse it and configure it: */
	Kernel::CPU::x86_stack_t * stack = (Kernel::CPU::x86_stack_t*)stack_ptr;
	stack->regs2.ebp = (uint32_t)(stack_ptr + 28);
	stack->old_ebp = (uint32_t)(stack_ptr + 32);
	stack->old_addr = (unsigned)task_return_grave;
	stack->ds = X86_SEGMENT_USER_DATA;
	stack->cs = X86_SEGMENT_USER_CODE;
	stack->eip = task->regs->eip;
	stack->eflags.interrupt = 1;
	stack->eflags.iopl = 3;
	stack->esp = task->regs->esp;
	stack->ss = X86_SEGMENT_USER_DATA;
	stack->regs2.eax = (uint32_t)task_return_grave; /* Return address of a task */
}
/**********************/

/***************************************************/
/****************** Task creation ******************/
/***************************************************/
void task_allocate_stack(task_t * task, uintptr_t stack_size) {
	task->regs->esp = (uint32_t) malloc(stack_size) + stack_size;
	task->image.stack = task->regs->esp;
}

void set_task_environment(task_t * task, void (*entry)(void), uint32_t eflags, uint32_t pagedir) {
	if(!entry) {
		/* Setting root's environment: */
		asm volatile("mov %%esp, %0" : "=r" (task->regs->esp)); /* Save ESP */
	} else {
		/* Setting normal environment: */
		/* Prepare X86 stack: */
		x86_set_stack((uint32_t*)(task->regs->esp), task);
	}

	task->regs->eax = task->regs->ebx = task->regs->ecx =
			task->regs->edx = task->regs->esi = task->regs->edi = 0;
	task->regs->eflags    = eflags;
	task->regs->cr3       = pagedir;
	task->image.entry     = (uintptr_t)entry;
	task->regs->eip       = (uint32_t) entry;
	task->thread.eip      = (uintptr_t)entry;
	task->thread.page_dir = (paging_directory_t*)pagedir;
	task->thread.esp      = task->regs->esp;
}

task_t * spawn_rootproc(void) {
	IRQ_OFF();

	task_t * root = new task_t;
	memset(root, 0, sizeof(task_t));
	root->regs = new regs_t;

	tree_set_root(tasktree, (void*)root);
	root->tree_entry = tasktree->root;
	list_insert(tasklist, (void*)root);

	root->pid               = get_next_pid(1);
	root->group             = 0;
	root->name              = strdup("kinit");
	root->cmdline           = 0;
	root->user              = 0;
	root->mask              = 022;
	root->group             = 0;
	root->status            = TASKST_CRADLE;

	root->fds               = new fd_table_t;
	root->fds->refs         = 1;
	root->fds->length       = 0;
	root->fds->capacity     = 4;
	root->fds->entries      = (FILE**)malloc(sizeof(FILE *) * root->fds->capacity);

	root->wd_node           = fs_clone(fs_root);
	root->work_dirpath      = strdup("/");

	root->image.entry       = 0;
	root->image.heap        = 0;
	root->image.heap_actual = 0;
	root->image.stack       = init_esp + 1;
	root->image.user_stack  = 0;
	root->image.size        = 0;
	root->image.shm_heap    = SHM_START;

	spin_init(root->image.lock);

	root->finished          = 0;
	root->started           = 1;
	root->running           = 1;
	root->wait_queue        = list_create();
	root->shm_mappings      = list_create();
	root->signal_queue      = list_create();
	root->signal_kstack     = 0;

	root->sched_node.prev   = 0;
	root->sched_node.next   = 0;
	root->sched_node.value  = root;

	root->timed_sleep_node  = 0;

	root->is_tasklet        = 0;

	root->ttl = root->ttl_start = 0;
	root->ttl_fscale            = MAX_TTL;
	root->ttl_pwm_mode          = 1;

	set_task_environment(root, 0, Kernel::CPU::read_reg(Kernel::CPU::eflags), (uint32_t)Kernel::Memory::Man::curr_dir->table_entries);

	root->desc = strdup("[kinit]");

	IRQ_RES();
	return root;
}

task_t * spawn_proc(task_t * parent, char addtotree, void (*entry)(void), uint32_t eflags, uint32_t pagedir) {
	if(!is_tasking_initialized) return 0;

	IRQ_OFF();

	task_t * task = new task_t;
	memset(task, 0, sizeof(task_t));
	task->regs = new regs_t;
	task->syscall_regs = new regs_t;

	task->pid = get_next_pid(0);
	task->group = parent->group;
	task->name = strdup(parent->name);
	task->desc = 0;
	task->cmdline = parent->cmdline;
	task->user = parent->user;
	task->mask = parent->mask;
	task->group = parent->group;

	task->ttl = task->ttl_start = 0;
	task->ttl_fscale = MAX_TTL;
	task->ttl_pwm_mode = 1;

	if(pagedir) {
		set_task_environment(task, entry, eflags, pagedir);
	} else {
		task->thread.esp = 0;
		task->thread.ebp = 0;
		task->thread.eip = 0;
		task->thread.fpu_enabled = 0;
	}

	task->image.entry = parent->image.entry;
	task->image.heap = parent->image.heap;
	task->image.heap_actual = parent->image.heap_actual;
	task->image.size = parent->image.size;
	task_allocate_stack(task, TASK_STACK_SIZE);
	task->image.user_stack = parent->image.user_stack;
	task->image.shm_heap = SHM_START;

	spin_init(task->image.lock);

	/* Clone the file descriptors from the parent process */
	task->fds = new fd_table_t;
	task->fds->refs = 1;
	task->fds->length = parent->fds->length;
	task->fds->capacity = parent->fds->capacity;
	task->fds->entries  = (FILE**)malloc(sizeof(FILE *) * task->fds->capacity);
	for(uint32_t i = 0; i < parent->fds->length; i++)
		task->fds->entries[i] = fs_clone(parent->fds->entries[i]);

	task->wd_node = fs_clone(parent->wd_node);
	task->work_dirpath = strdup(parent->work_dirpath);

	task->status = TASKST_CRADLE;
	task->finished = 0;
	task->started = 0;
	task->running = 0;
	memset(task->signals.functions, 0, sizeof(uintptr_t) * NUMSIGNALS);
	task->wait_queue = list_create();
	task->shm_mappings = list_create();
	task->signal_queue = list_create();
	task->signal_kstack = 0;

	task->sched_node.prev = 0;
	task->sched_node.next = 0;
	task->sched_node.value = task;

	task->sleep_node.prev = 0;
	task->sleep_node.next = 0;
	task->sleep_node.value = task;

	task->timed_sleep_node = 0;

	task->is_tasklet = 0;
	if(addtotree)
		task_addtotree((task_t*)parent, task);
	IRQ_RES();
	return task;
}

task_t * spawn_childproc(task_t * parent) {
	return spawn_proc(parent, 1, 0, 0, 0);
}
/***************************************************/

/***************************************************/
/******* Task creation bootstrap functions *********/
/***************************************************/
uint32_t fork(void) {

	return 0;
}

uint32_t task_clone(uintptr_t new_stack, uintptr_t thread_function, uintptr_t arg) {

	return 0;
}

int task_create_tasklet(void) {

	return 0;
}
/***************************************************/


/**********************************/
/****** Process initializers ******/
/**********************************/
void initialize_process_tree(task_t * main_task) {
	/* Initialize task list and tree: */
	tasklist = list_create();
	tasktree = tree_create();
	tree_set_root(tasktree, main_task);
	list_insert(tasklist, main_task);
}
/**********************************/

}
}
