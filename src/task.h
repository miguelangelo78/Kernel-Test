/*
 * task.h
 *
 *  Created on: 15/07/2016
 *      Author: Miguel
 */
#include <system.h>
#include <libc/tree.h>
#include <libc/list.h>

#ifndef SRC_TASK_H_
#define SRC_TASK_H_
namespace Kernel {
/* Tasking: */
namespace Task {

/* Max number of processes the kernel will switch: */
#define MAX_PID 32768
#define MAX_TTL 2000 /* Maximum allowable time to live (expressed in switching count, not by ms or seconds) */
#define TASK_STACK_SIZE (PAGE_SIZE * 2)
#define USER_ROOT_UID ((user_t)0)

#define THREAD_RETURN 0xFFFFB00F

typedef signed int    pid_t;
typedef unsigned int  user_t;
typedef unsigned char status_t;

typedef void (*tasklet_t)(void *, char *);

typedef struct {
	unsigned int gs, fs, es, ds;
	unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
	unsigned int int_no, err_code;
	unsigned int eip, cs, eflags, useresp, ss;
} regs_t;

enum task_state {
	TASKST_CRADLE,
	TASKST_READY,
	TASKST_RUNNING,
	TASKST_BLOCKED,
	TASKST_GRAVE
};

enum wait_option {
	WCONTINUED,
	WNOHANG,
	WUNTRACED
};

/* Thread struct definition: */
typedef struct thread {
	uintptr_t esp, ebp, eip;
	uint8_t fpu_enabled;
	uint8_t fp_regs[512];
	uint8_t padding[32];
	paging_directory_t * page_dir;
} thread_t;

/* Portable image struct: */
typedef struct image {
	size_t size;           /* Image size */
	uintptr_t entry;       /* Binary entry point */
	uintptr_t heap;        /* Heap pointer */
	uintptr_t heap_actual; /* Actual heap location */
	uintptr_t stack;       /* Process kernel stack */
	uintptr_t user_stack;  /* User stack */
	uintptr_t start;
	uintptr_t shm_heap;
	volatile int lock[2];
} image_t;

typedef struct file_descriptor_table {
	FILE ** entries;
	size_t length;
	size_t capacity;
	size_t refs;
} fd_table_t;

typedef struct signal_table {
	uintptr_t functions[NUMSIGNALS + 1];
} sig_table_t;

/* Task struct definition: */
typedef struct task {
	/* Identification: */
	pid_t pid;
	char * name;
	char * desc;
	pid_t group;
	pid_t job;
	pid_t session;

	/* Execution mode: */
	user_t user;
	int mask;
	char ** cmdline;

	/* Context: */
	regs_t * syscall_regs;
	image_t image;

	/* Tasking: */
	thread_t thread;

	/* Procfs/tree structure and Files: */
	tree_node_t * tree_entry;
	FILE * wd_node;
	char * work_dirpath; /* Working directory */
	fd_table_t * fds;

	/* TTL: */
	char ttl_pwm_mode;
	int ttl;
	int ttl_start;
	int ttl_fscale; /* Switching frequency scale */

	/* States:*/
	status_t status;
	uint8_t finished;
	uint8_t started;
	uint8_t running;
	int exitcode;

	/* Signals: */
	sig_table_t signals;
	list_t * signal_queue;
	thread_t signal_state;
	char * signal_kstack;

	/* Waiting: */
	list_t * wait_queue;
	node_t sched_node;
	node_t sleep_node;
	node_t * timed_sleep_node;
	volatile uint8_t sleep_interrupted;

	/* Shared memory: */
	list_t * shm_mappings;

	/* Process type: */
	uint8_t is_tasklet;
} task_t;

typedef struct sleeper {
	unsigned long end_tick;
	unsigned long end_subtick;
	task_t * task;
} sleeper_t;

extern volatile task_t * current_task;
extern task_t * main_task;

void tasking_install(void);
void switch_task(status_t new_process_state);
void tasking_enable(char enable);
void task_set_ttl(task_t * task, int duty_cycle_or_preload);
void task_set_ttl(int pid, int duty_cycle_or_preload);
void task_set_ttl_fscale(task_t * task, int fscale);
void task_set_ttl_fscale(int pid, int fscale);
void task_set_ttl_mode(task_t * task, char pwm_or_pulse_mode);
void task_set_ttl_mode(int pid, char pwm_or_pulse_mode);
void task_exit(int pid);
void kexit(int retval);
void task_free(task_t * task_to_free, int retval);

void set_task_environment(task_t * task, paging_directory_t * pagedir);

task_t * spawn_rootproc(void);
task_t * spawn_childproc(task_t * parent);
task_t * spawn_proc(task_t * parent, char addtotree, paging_directory_t * pagedir);

uint32_t fork(void);
uint32_t task_clone(uintptr_t new_stack, uintptr_t thread_function, uintptr_t arg);
int task_create_tasklet(tasklet_t tasklet, char * name, void * argp);

task_t * current_task_get(void);
uint32_t current_task_getpid(void);
task_t * task_from_pid(pid_t pid);
task_t * task_get_parent(task_t * task);

void make_task_ready(task_t * task);
int wakeup_queue(list_t * queue);
int wakeup_queue_interrupted(list_t * queue);
void wakeup_sleepers(unsigned long seconds, unsigned long subseconds);
int sleep_on(list_t * queue);
void sleep_until(task_t * task, unsigned long seconds, unsigned long subseconds);

uint32_t task_append_fd(task_t * task, FILE * node);
uint32_t process_move_fd(task_t * task, int src, int dest);
}
}
#endif /* SRC_TASK_H_ */
