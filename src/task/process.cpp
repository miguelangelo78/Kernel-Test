/*
 * process.cpp
 *
 *  Created on: 13/07/2016
 *      Author: Miguel
 */

#include <system.h>
#include <module.h>
#include <libc/tree.h>
#include <errno.h>

namespace Kernel {
namespace Task {

/****************************************************************************/
/************ Tasking/Process prototype functions / externs *****************/
/****************************************************************************/
extern void task_return_grave(int retval);
extern char is_tasking_initialized;
extern void task_reap(task_t * task);
/****************************************************************************/

/**********************************************************/
/************ Tasking / Process Variables *****************/
/**********************************************************/
uint16_t next_pid = 1;
tree_t * task_tree;
list_t * task_list;
list_t * task_queue;
list_t * sleep_queue;

/* Locks: */
static spin_lock_t tree_lock = { 0 };
static spin_lock_t task_queue_lock = { 0 };
static spin_lock_t wait_lock_tmp = { 0 };
static spin_lock_t sleep_lock = { 0 };
/**********************************************************/

/**************************************/
/***** Task queueing and sleeping *****/
/**************************************/

int task_is_ready(task_t * task) {
	return task->sched_node.owner != 0;
}

/* Check if there's a task to dequeue from the queue: */
uint8_t task_available(void) {
	return task_queue->head != 0;
}

/* Fetch and dequeue task: */
task_t * fetch_next_task(void) {
	if(!task_available())
		return main_task;
	return (task_t*)list_dequeue(task_queue)->value;
}

/* Insert task back into the queue: */
void make_task_ready(task_t * task) {
	if(task->sleep_node.owner != 0) {
		if (task->sleep_node.owner == sleep_queue) {
			if (task->timed_sleep_node) {
				IRQ_OFF();
				spin_lock(sleep_lock);
				list_delete(sleep_queue, task->timed_sleep_node);
				spin_unlock(sleep_lock);
				IRQ_RES();
				task->sleep_node.owner = 0;
				free(task->timed_sleep_node->value);
			}
		} else {
			task->sleep_interrupted = 1;
			spin_lock(wait_lock_tmp);
			list_delete((list_t*)task->sleep_node.owner, &task->sleep_node);
			spin_unlock(wait_lock_tmp);
		}
	}

	spin_lock(task_queue_lock);
	list_append(task_queue, &task->sched_node);
	spin_unlock(task_queue_lock);
}

int wakeup_queue(list_t * queue) {
	int awoken_processes = 0;
	while (queue->length > 0) {
		spin_lock(wait_lock_tmp);
		node_t * node = list_pop(queue);
		spin_unlock(wait_lock_tmp);
		if (!((task_t *)node->value)->finished)
			make_task_ready((task_t*)node->value);
		awoken_processes++;
	}
	return awoken_processes;
}
EXPORT_SYMBOL(wakeup_queue);

int wakeup_queue_interrupted(list_t * queue) {
	int awoken_processes = 0;
	while (queue->length > 0) {
		spin_lock(wait_lock_tmp);
		node_t * node = list_pop(queue);
		spin_unlock(wait_lock_tmp);
		if (!((task_t *)node->value)->finished) {
			task_t * task = (task_t*)node->value;
			task->sleep_interrupted = 1;
			make_task_ready(task);
		}
		awoken_processes++;
	}
	return awoken_processes;
}

void wakeup_sleepers(unsigned long seconds, unsigned long subseconds) {
	IRQ_OFF();
	spin_lock(sleep_lock);
	if (sleep_queue->length) {
		sleeper_t * proc = ((sleeper_t *)sleep_queue->head->value);
		while (proc && (proc->end_tick < seconds || (proc->end_tick == seconds && proc->end_subtick <= subseconds))) {
			task_t * task = proc->task;
			task->sleep_node.owner = 0;
			task->timed_sleep_node = 0;
			//if (!task_is_ready(task))
			//	make_task_ready(task);
			free(proc);
			free(list_dequeue(sleep_queue));
			if (sleep_queue->length)
				proc = ((sleeper_t *)sleep_queue->head->value);
			else
				break;
		}
	}
	spin_unlock(sleep_lock);
	IRQ_RES();
}

static int wait_candidate(task_t * parent, int pid, int options, task_t * task) {
	(void)options; /* there is only one option that affects candidacy, and we don't support it yet */

	if (!task) return 0;

	if (pid < -1) {
		if (task->group == -pid || task->pid == -pid) return 1;
	} else if (pid == 0) {
		/* Matches our group ID */
		if (task->group == parent->pid) return 1;
	} else if (pid > 0) {
		/* Specific pid */
		if (task->pid == pid) return 1;
	} else {
		return 1;
	}
	return 0;
}

int waitpid(int pid, int * status, int options) {
	task_t * task = (task_t*)current_task;
	if (task->group)
		task = task_from_pid(task->group);

	do {
		task_t * candidate = 0;
		int has_children = 0;

		/* First, find out if there is anyone to reap */
		foreach(node, task->tree_entry->children) {
			if (!node->value)
				continue;
			task_t * child = (task_t*)((tree_node_t *)node->value)->value;
			if (wait_candidate(task, pid, options, child)) {
				has_children = 1;
				if (child->finished) {
					candidate = child;
					break;
				}
			}
		}

		if (!has_children) {
			/* No valid children matching this description */
			return -ECHILD;
		}

		if (candidate) {
			if (status)
				*status = candidate->status;
			int pid = candidate->pid;
			task_reap(candidate);
			return pid;
		} else {
			if (options & 1)
				return 0;
			/* Wait */
			if (sleep_on(task->wait_queue) != 0)
				return -EINTR;
		}
	} while (1);
}

int sleep_on(list_t * queue) {
	if(current_task->sleep_node.owner) {
		switch_task(0);
		return 0;
	}
	current_task->sleep_interrupted = 0;
	spin_lock(wait_lock_tmp);
	list_append(queue, (node_t*)&current_task->sleep_node);
	spin_unlock(wait_lock_tmp);
	switch_task(0);
	return current_task->sleep_interrupted;
}
EXPORT_SYMBOL(sleep_on);

void sleep_until(task_t * task, unsigned long seconds, unsigned long subseconds) {
	if(current_task->sleep_node.owner)
		return; /* Can't sleep. Already sleeping */

	task->sleep_node.owner = sleep_queue;

	IRQ_OFF();
	spin_lock(sleep_lock);

	node_t * before;
	foreach(node, sleep_queue) {
		sleeper_t * candidate = ((sleeper_t *)node->value);
		if (candidate->end_tick > seconds || (candidate->end_tick == seconds && candidate->end_subtick > subseconds))
			break;
		before = node;
	}

	sleeper_t * proc = (sleeper_t*)new sleeper_t;
	proc->task = task;
	proc->end_tick = seconds;
	proc->end_subtick = subseconds;
	task->timed_sleep_node = list_insert_after(sleep_queue, before, proc);

	spin_unlock(sleep_lock);
	IRQ_RES();
}
EXPORT_SYMBOL(sleep_until);
/**************************************/

/*************************************************************/
/*************** Task tree getters and setters ***************/
/*************************************************************/

/* Clean up task: */
void task_free(task_t * task, int retval) {
	task->status = retval;
	task->finished = 1;

	list_free(task->wait_queue);
	free(task->wait_queue);
	list_free(task->signal_queue);
	free(task->signal_queue);

	free(task->work_dirpath);

	/* TODO: Release shared memory */
	free(task->shm_mappings);

	if(task->signal_kstack)
		free(task->signal_kstack);

	if(--task->fds->refs == 0) {
		release_directory(task->thread.page_dir);
		for(uint32_t i = 0; i < task->fds->length; i++) {
			fclose(task->fds->entries[i]);
			task->fds->entries[i] = 0;
		}
		free(task->fds->entries);
		free(task->fds);
		free((void *)(task->image.stack - TASK_STACK_SIZE));
		free(task->syscall_regs);
	}
}

/* Remove the task from the list and tree: */
void task_removefromtree(task_t * task) {
	if(!task->tree_entry) return;
	tree_node_t * tree_entry = task->tree_entry;

	if(task_tree->root == tree_entry)
		return; /* We shall not allow the root task to be removed */

	/* Remove it: */
	spin_lock(tree_lock);
	tree_remove_reparent_root(task_tree, tree_entry);
	list_delete(task_list, list_find(task_list, task));
	spin_unlock(tree_lock);
	free(task);
}

void task_addtotree(task_t * parent_task, task_t * new_task) {
	/* Add task to tree: */
	tree_node_t * tree_entry = tree_node_create(new_task);
	new_task->tree_entry = tree_entry;

	/* Insert it: */
	spin_lock(tree_lock);
	tree_node_insert_child_node(task_tree, parent_task->tree_entry, tree_entry);
	list_insert(task_list, new_task);
	spin_unlock(tree_lock);
}

uint8_t task_compare_lambdafunc(void * task_v, void * pid_v) {
	pid_t pid = (*(pid_t *)pid_v);
	task_t * task = (task_t*)task_v;
	return (uint8_t)(task->pid == pid);
}

task_t * task_from_pid(pid_t pid) {
	if(pid < 0) return 0;

	spin_lock(tree_lock);
	tree_node_t * tree_entry = tree_find(task_tree, &pid, task_compare_lambdafunc);
	spin_unlock(tree_lock);
	if(tree_entry)
		return (task_t*)tree_entry->value;
	return 0;
}

task_t * task_get_parent(task_t * task) {
	task_t * ret = 0;
	spin_lock(tree_lock);
	tree_node_t * tree_entry = task->tree_entry;
	if(tree_entry->parent)
		ret = (task_t*)tree_entry->parent->value;
	spin_unlock(tree_lock);
	return ret;
}

uint32_t current_task_getpid(void) {
	return current_task->pid;
}
EXPORT_SYMBOL(current_task_getpid);

task_t * current_task_get(void) {
	return (task_t*)current_task;
}
EXPORT_SYMBOL(current_task_get);

task_t * main_task_get(void) {
	return main_task;
}
EXPORT_SYMBOL(main_task_get);

pid_t get_next_pid(char restart_pid) {
	if(restart_pid) next_pid = 1;
	return next_pid++;
}
/*************************************************************/

/*********************************/
/***** Task file descriptors *****/
/*********************************/

/*
 * Append a file descriptor to a process.
 *
 * @param proc Process to append to
 * @param node The VFS node
 * @return The actual fd, for use in userspace
 */
uint32_t task_append_fd(task_t * task, FILE * node) {
	/* Fill gaps */
	for (unsigned int i = 0; i < task->fds->length; ++i) {
		if (!task->fds->entries[i]) {
			task->fds->entries[i] = node;
			return i;
		}
	}
	/* No gaps, expand */
	if (task->fds->length == task->fds->capacity) {
		task->fds->capacity *= 2;
		task->fds->entries = (FILE **)realloc(task->fds->entries, sizeof(FILE *) * task->fds->capacity);
	}
	task->fds->entries[task->fds->length] = node;
	task->fds->length++;
	return task->fds->length - 1;
}

/*
 * dup2() -> Move the file pointed to by `s(ou)rc(e)` into
 *           the slot pointed to be `dest(ination)`.
 *
 * @param proc  Process to do this for
 * @param src   Source file descriptor
 * @param dest  Destination file descriptor
 * @return The destination file descriptor, -1 on failure
 */
uint32_t process_move_fd(task_t * task, int src, int dest) {
	if ((size_t)src > task->fds->length || (size_t)dest > task->fds->length)
		return -1;
	if (task->fds->entries[dest] != task->fds->entries[src]) {
		fclose(task->fds->entries[dest]);
		task->fds->entries[dest] = task->fds->entries[src];
		fopen(task->fds->entries[dest], 0);
	}
	return dest;
}
/*********************************/

void task_allocate_stack(task_t * task, uintptr_t stack_size) {
	task->thread.esp = (uint32_t) kvmalloc(stack_size) + stack_size;
}

void task_allocate_image_stack(task_t * task, uintptr_t stack_size) {
	task->image.stack = (uint32_t) kvmalloc(stack_size) + stack_size;
}
/**********************/

/***************************************************/
/****************** Task creation ******************/
/***************************************************/

void set_task_environment(task_t * task, paging_directory_t * pagedir) {
	task->thread.page_dir = pagedir;
}

task_t * spawn_rootproc(void) {
	IRQ_OFF();

	task_t * root = new task_t;
	memset(root, 0, sizeof(task_t));

	tree_set_root(task_tree, (void*)root);
	root->tree_entry = task_tree->root;
	list_insert(task_list, (void*)root);

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

	/* Set paging directory: */
	set_task_environment(root, Kernel::Memory::Man::curr_dir);

	root->desc = strdup("[kinit]");

	IRQ_RES();
	return root;
}

task_t * spawn_proc(task_t * parent, char addtotree, paging_directory_t * pagedir) {
	if(!is_tasking_initialized) return 0;

	IRQ_OFF();

	task_t * task = new task_t;
	memset(task, 0, sizeof(task_t));
	task->syscall_regs = new Kernel::CPU::regs_t;

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
		set_task_environment(task, pagedir);
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
	task_allocate_image_stack(task, TASK_STACK_SIZE);
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
	task->wait_queue    = list_create();
	task->shm_mappings  = list_create();
	task->signal_queue  = list_create();
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
	return spawn_proc(parent, 1, 0);
}
/***************************************************/

/***************************************************/
/******* Task creation bootstrap functions *********/
/***************************************************/

extern "C" { void return_to_userspace(void); }

/*
 * Fork. This is supposed to be called by the user in usermode!
 *
 * @return To the parent: PID of the child; to the child: 0
 */
uint32_t fork(void) {
	IRQ_OFF();

	current_task->syscall_regs->eax = 0;

	/* Cast from volatile type: */
	task_t * parent = (task_t*)current_task;
	/* Clone paging directory: */
	paging_directory_t * dirclone = clone_directory(curr_dir);
	/* Spawn child from parent: */
	task_t * new_task = spawn_childproc(parent);
	/* Set just the directory: */
	new_task->thread.page_dir = (paging_directory_t*)dirclone;

	/* Store syscall registers: */
	Kernel::CPU::regs_t r;
	memcpy(&r, current_task->syscall_regs, sizeof(Kernel::CPU::regs_t));
	new_task->syscall_regs = &r;

	/* Push normal registers: */
	uintptr_t esp = new_task->image.stack;
	uintptr_t ebp = esp;
	new_task->syscall_regs->eax = 0;
	esp -= sizeof(Kernel::CPU::regs_t);
	memcpy((Kernel::CPU::regs_t *) esp, &r, sizeof(Kernel::CPU::regs_t));

	new_task->thread.esp = esp;
	new_task->thread.ebp = ebp;

	new_task->is_tasklet = parent->is_tasklet;

	/* Jump location: */
	new_task->thread.eip = (uintptr_t)&return_to_userspace;

	/* And make it run: */
	make_task_ready(new_task);

	IRQ_RES();
	return new_task->pid;
}
EXPORT_SYMBOL(fork);

/*
 * clone the current thread and create a new one in the same
 * memory space with the given pointer as its new stack.
 */
uint32_t task_clone(uintptr_t new_stack, uintptr_t thread_function, uintptr_t arg) {
	IRQ_OFF();
	current_task->syscall_regs->eax = 0;
	/* Cast from volatile type: */
	task_t * parent = (task_t*)current_task;
	/* Clone paging directory: */
	paging_directory_t * dirclone = clone_directory(curr_dir);
	/* Spawn child from parent: */
	task_t * new_task = spawn_childproc(parent);
	/* Set just the directory: */
	new_task->thread.page_dir = dirclone;

	/* Store syscall registers: */
	Kernel::CPU::regs_t r;
	memcpy(&r, current_task->syscall_regs, sizeof(Kernel::CPU::regs_t));
	new_task->syscall_regs = &r;

	uintptr_t esp = new_task->image.stack;
	uintptr_t ebp = esp;

	if(current_task->group)
		new_task->group = current_task->group;
	else
		new_task->group = current_task->pid;

	new_task->syscall_regs->ebp = new_stack;
	new_task->syscall_regs->eip = thread_function;

	PUSH(new_stack, uintptr_t, arg);
	PUSH(new_stack, uintptr_t, THREAD_RETURN);

	new_task->syscall_regs->esp = new_stack;
	new_task->syscall_regs->useresp = new_stack;

	/* Push registers: */
	esp -= sizeof(Kernel::CPU::regs_t);
	memcpy((Kernel::CPU::regs_t *) esp, &r, sizeof(Kernel::CPU::regs_t));

	new_task->thread.esp = esp;
	new_task->thread.ebp = ebp;

	new_task->is_tasklet = parent->is_tasklet;

	free(new_task->fds);
	new_task->fds = current_task->fds;
	new_task->fds->refs++;

	new_task->thread.eip = (uintptr_t)&return_to_userspace;

	make_task_ready(new_task);

	IRQ_RES();
	return new_task->pid;
}
EXPORT_SYMBOL(task_clone);

int task_create_tasklet(tasklet_t tasklet, char * name, void * argp) {
	IRQ_OFF();
	if (current_task->syscall_regs)
		current_task->syscall_regs->eax = 0;

	paging_directory_t * dir = kernel_directory;
	task_t * new_task = spawn_childproc((task_t*)current_task);
	new_task->thread.page_dir = dir;

	/* Store syscall registers: */
	if(current_task->syscall_regs) {
		Kernel::CPU::regs_t r;
		memcpy(&r, current_task->syscall_regs, sizeof(Kernel::CPU::regs_t));
		new_task->syscall_regs = &r;
	}

	uintptr_t esp = new_task->image.stack;
	uintptr_t ebp = esp;

	if(current_task->syscall_regs)
		new_task->syscall_regs->eax = 0;

	new_task->is_tasklet = 1;
	new_task->name = name;

	PUSH(esp, uintptr_t, (uintptr_t)name);
	PUSH(esp, uintptr_t, (uintptr_t)argp);
	PUSH(esp, uintptr_t, (uintptr_t)&task_return_grave);

	new_task->thread.esp = esp;
	new_task->thread.ebp = ebp;

	new_task->thread.eip = (uintptr_t)tasklet;

	/* All done: */
	make_task_ready(new_task);

	IRQ_RES();
	return new_task->pid;
}
EXPORT_SYMBOL(task_create_tasklet);
/***************************************************/

/**********************************/
/****** Process initializers ******/
/**********************************/
void initialize_process_tree() {
	/* Initialize task list and tree: */
	task_tree   = tree_create();
	task_list   = list_create();
	task_queue  = list_create();
	sleep_queue = list_create();
}
/**********************************/

}
}
