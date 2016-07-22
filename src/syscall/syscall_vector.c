/*
 * syscall_vector.c
 *
 *  Created on: 22/07/2016
 *      Author: Miguel
 */

#include <stdint.h>
#include "syscall_nums.h"
#include <time.h>
#include <utsname.h>

int sys_exit(int retval);
int sys_open(const char * file, int flags, int mode);
int sys_read(int fd, char * ptr, int len);
int sys_write(int fd, char * ptr, int len);
int sys_close(int fd);
int sys_gettimeofday(time_t * tv, void * tz);
int sys_execve(const char * filename, char *const argv[], char *const envp[]);
int sys_fork(void);
int sys_getpid(void);
int sys_sbrk(int size);
int sys_uname(utsname_t * name);
int sys_openpty(int * master, int * slave, char * name, void * _ign0, void * size);
int sys_seek(int fd, int offset, int whence);
int sys_stat(int fd, uintptr_t st);
int sys_mkpipe(void);
int sys_dup2(int old, int _new);
int sys_getuid(void);
int sys_setuid(unsigned int new_uid);
int sys_reboot(void);
int sys_readdir(int fd, int index, struct dirent * entry);
int sys_chdir(char * newdir);
int sys_getcwd(char * buf, size_t size);
int sys_clone(uintptr_t new_stack, uintptr_t thread_func, uintptr_t arg);
int sys_sethostname(char * new_hostname);
int sys_gethostname(char * buffer);
int sys_mkdir(char * path, uint32_t mode);
int sys_shm_obtain(char * path, size_t * size);
int sys_shm_release(char * path);
int sys_kill(signed int process, uint32_t signal);
int sys_signal(uint32_t signum, uintptr_t handler);
int sys_gettid(void);
int sys_yield(void);
int sys_sysfunc(int fn, char ** args);
int sys_sleepabs(unsigned long seconds, unsigned long subseconds);
int sys_sleep(unsigned long seconds, unsigned long subseconds);
int sys_ioctl(int fd, int request, void * argp);
int sys_access(const char * file, int flags);
int sys_statf(char * file, uintptr_t st);
int sys_chmod(char * file, int mode);
int sys_umask(int mode);
int sys_unlink(char * file);
int sys_waitpid(int pid, int * status, int options);
int sys_pipe(int pipes[2]);
int sys_mount(char * arg, char * mountpoint, char * type, unsigned long flags, void * data);
int sys_symlink(char * target, char * name);
int sys_readlink(const char * file, char * ptr, int len);
int sys_lstat(char * file, uintptr_t st);

/* System Call Default Vector: */
int (*syscalls[])() =
{
		[SYS_EXT]          = sys_exit,
		[SYS_OPEN]         = sys_open,
		[SYS_READ]         = sys_read,
		[SYS_WRITE]        = sys_write,
		[SYS_CLOSE]        = sys_close,
		[SYS_GETTIMEOFDAY] = sys_gettimeofday,
		[SYS_EXECVE]       = sys_execve,
		[SYS_FORK]         = sys_fork,
		[SYS_GETPID]       = sys_getpid,
		[SYS_SBRK]         = sys_sbrk,
		[SYS_UNAME]        = sys_uname,
		[SYS_OPENPTY]      = sys_openpty,
		[SYS_SEEK]         = sys_seek,
		[SYS_STAT]         = sys_stat,
		[SYS_MKPIPE]       = sys_mkpipe,
		[SYS_DUP2]         = sys_dup2,
		[SYS_GETUID]       = sys_getuid,
		[SYS_SETUID]       = sys_setuid,
		[SYS_REBOOT]       = sys_reboot,
		[SYS_READDIR]      = sys_readdir,
		[SYS_CHDIR]        = sys_chdir,
		[SYS_GETCWD]       = sys_getcwd,
		[SYS_CLONE]        = sys_clone,
		[SYS_SETHOSTNAME]  = sys_sethostname,
		[SYS_GETHOSTNAME]  = sys_gethostname,
		[SYS_MKDIR]        = sys_mkdir,
		[SYS_SHM_OBTAIN]   = sys_shm_obtain,
		[SYS_SHM_RELEASE]  = sys_shm_release,
		[SYS_KILL]         = sys_kill,
		[SYS_SIGNAL]       = sys_signal,
		[SYS_GETTID]       = sys_gettid,
		[SYS_YIELD]        = sys_yield,
		[SYS_SYSFUNC]      = sys_sysfunc,
		[SYS_SLEEPABS]     = sys_sleepabs,
		[SYS_SLEEP]        = sys_sleep,
		[SYS_IOCTL]        = sys_ioctl,
		[SYS_ACCESS]       = sys_access,
		[SYS_STATF]        = sys_statf,
		[SYS_CHMOD]        = sys_chmod,
		[SYS_UMASK]        = sys_umask,
		[SYS_UNLINK]       = sys_unlink,
		[SYS_WAITPID]      = sys_waitpid,
		[SYS_PIPE]         = sys_pipe,
		[SYS_MOUNT]        = sys_mount,
		[SYS_SYMLINK]      = sys_symlink,
		[SYS_READLINK]     = sys_readlink,
		[SYS_LSTAT]        = sys_lstat
};

uint32_t num_syscalls = sizeof(syscalls) / sizeof(*syscalls);

