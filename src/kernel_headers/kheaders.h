/*
 * kheaders.h
 *
 *  Created on: 25/03/2016
 *      Author: Miguel
 */

#ifndef SRC_KERNEL_HEADERS_KHEADERS_H_
#define SRC_KERNEL_HEADERS_KHEADERS_H_

#include <numarg.h>
#include <libc.h>
#include <module.h>
#include <fs.h>

#ifdef MODULE

#define syscall_schedule_install(syscall_name, no) FCASTF(SYF("syscall_schedule_install"), char, char*, int, uintptr_t)((char*)# syscall_name, no, (uintptr_t)syscall_name)
#define syscall_uninstall(syscall_addr) FCASTF(SYF("syscall_uninstall"), void, uintptr_t)((uintptr_t)syscall_addr)
#define syscall_run_n(intno) FCASTF(SYF("syscall_run_n"), void, int)(intno)
#define syscall_run_s(syscall_name) FCASTF(SYF("syscall_run_s"), void, char*)(syscall_name)
#define syscall_run_ss(syscall_name) FCASTF(SYF("syscall_run_s"), void, char*)(# syscall_name)

#define irq_install_handler(irq_num, irq_handler) FCASTF(SYF("irq_install_handler"), void, size_t, irq_handler_t)(irq_num, irq_handler)
#define irq_ack(irq_num) FCASTF(SYF("irq_ack"), void, size_t)(irq_num)

#define is_kernel_init() FCASTF(SYF("is_kernel_init"), char, void)()

#define kprintf(fmt, ...) symbol_call_args_("mod_kprintf", PP_NARG(__VA_ARGS__) + 1, (char*)fmt, ##__VA_ARGS__)
#define scwn() SYC("mod_term_scrolldown")
#define scup() SYC("mod_term_scrollup")
#define scbot() SYC("mod_term_scrollbot")
#define sctop() SYC("mod_term_scrolltop")

#define malloc(bytecount) FCASTF(SYF("malloc"), void *, uintptr_t)(bytecount)
#define realloc(ptr, bytecount) FCASTF(SYF("realloc"), void *, void*, uintptr_t)(ptr, bytecount)
#define calloc(nmemb, bytecount) FCASTF(SYF("calloc"), void *, uintptr_t, uintptr_t)(nmemb, bytecount)
#define valloc(bytecount) FCASTF(SYF("valloc"), void *, uintptr_t)(bytecount)

#define free(ptr) FCASTF(SYF("free"), void, void *)(ptr);

#define list_create() FCASTF(SYF("list_create"), list_t *, void)()
#define list_insert(list, item) FCASTF(SYF("list_insert"), node_t *, list_t *, void *)(list, item)
#define list_free(list) FCASTF(SYF("list_free"), void, list_t *)(list)

#define hashmap_create(count) FCASTF(SYF("hashmap_create"), hashmap_t *, int)(count)
#define hashmap_set(map, key, value) FCASTF(SYF("hashmap_set"), void *, hashmap_t *, void *, void *)(map, key, value)
#define hashmap_remove(map, key) FCASTF(SYF("hashmap_remove"), void *, hashmap_t*, void*)(map, key)
#define hashmap_get_i(map, idx) FCASTF(SYF("hashmap_get_i"), void *, hashmap_t *, int)(map, idx)
#define hashmap_has(map, key) FCASTF(SYF("hashmap_has"), int, hashmap_t *, void*)(map, key)

#define strlen(str) FCASTF(SYF("strlen"), unsigned long, char*)(str)
#define strcpy(dst, src) FCASTF(SYF("strcpy"), char*, char*, char*)(dst, src)
#define strchr(s,c) FCASTF(SYF("strchr"), char*, char*, int)(s, c)
#define strdup(s) FCASTF(SYF("strdup"), char*, char*)(s)

#define isxdigit(ch) FCASTF(SYF("isxdigit"), int, int)(ch)
#define isdigit(ch) FCASTF(SYF("isdigit"), int, int)(ch)
#define isspace(ch) FCASTF(SYF("isspace"), int, int)(ch)
#define tolower(c) FCASTF(SYF("tolower"), int, int)(c)
#define toupper(c) FCASTF(SYF("toupper"), int, int)(c)

#define memset(dest, c, n) FCASTF(SYF("memset"), void *, void *, int, size_t)(dest, c, n)
#define memcpy(dest, src, n) FCASTF(SYF("memcpy"), void *, void *, void *, size_t)(dest, src, n)

#define split(str, deli) FCASTF(SYF("split"), split_t, char *, char)(str, deli)
#define free_split(str) FCASTF(SYF("free_split"), void, split_t)(str)

#define sprintf(buf, fmt, ...) FCASTF(SYF("sprintf"), int, char *, char *, ...)(buf, fmt, __VA_ARGS__)

#define ring_buffer_unread(ring_buffer) FCASTF(SYF("ring_buffer_unread"), size_t, ring_buffer_t *)(ring_buffer)
#define ring_buffer_read(ring_buffer, size, buffer) FCASTF(SYF("ring_buffer_read"), size_t, ring_buffer_t *, size_t, uint8_t*)(ring_buffer, size, buffer)
#define ring_buffer_write(ring_buffer, size, buffer) FCASTF(SYF("ring_buffer_write"), size_t, ring_buffer_t *, size_t, uint8_t*)(ring_buffer, size, buffer)
#define ring_buffer_interrupt(ring_buffer) FCASTF(SYF("ring_buffer_interrupt"), void, ring_buffer_t *)(ring_buffer)
#define ring_buffer_create(size) FCASTF(SYF("ring_buffer_create"), ring_buffer_t *, size_t)(size)

#define IRQ_OFF() SYC("int_disable")
#define IRQ_RES() SYC("int_resume")
#define IRQ_ON()  SYC("int_enable")

#define spin_init(lock)   FCASTF(SYF("spin_init"),   void, spin_lock_t)(lock)
#define spin_lock(lock)   FCASTF(SYF("spin_lock"),   void, spin_lock_t)(lock)
#define spin_unlock(lock) FCASTF(SYF("spin_unlock"), void, spin_lock_t)(lock)

/* Virtual File System library functions: */
#define vfs_mount(path, local_root) FCASTF(SYF("vfs_mount"), void *, char *, FILE *)(path, local_root)
#define vfs_register(filesystem_name, cback) FCASTF(SYF("vfs_register"), int, char *, vfs_mount_callback)(filesystem_name, cback)
#define vfs_mount_type(type, arg, mountpoint) FCASTF(SYF("vfs_mount_type"), int, char *, char *, char *)(type, arg, mountpoint)
#define vfs_lock(node) FCASTF(SYF("vfs_lock"), void, FILE *)(node)
#define map_vfs_directory(c) FCASTF(SYF("map_vfs_directory"), void, char *)(c)
#define fread(node, offset, size, buffer) FCASTF(SYF("fread"), uint32_t, FILE *, uint32_t, uint32_t, uint8_t *)(node, offset, size, buffer)
#define fwrite(node, offset, size, buffer) FCASTF(SYF("fwrite"), uint32_t, FILE *, uint32_t, uint32_t, uint8_t *)(node, offset, size, buffer)
#define fopen(node, flags) FCASTF(SYF("fopen"), uint32_t, FILE *, unsigned int)(node, flags)
#define fclose(node) FCASTF(SYF("fclose"), uint32_t, FILE *)(node)
#define fs_readdir(node, index) FCASTF(SYF("fs_readdir"), struct dirent *, FILE *, uint32_t)(node, index)
#define fs_finddir(node, name) FCASTF(SYF("fs_finddir"), FILE *, FILE *, char *)(node, name)
#define fs_filesize(node) FCASTF(SYF("fs_filesize"), uint32_t, FILE *)(node)
#define fs_is_dir(node) FCASTF(SYF("fs_is_dir"), char, FILE *)(node)
#define fs_mkdir(dirname, permission) FCASTF(SYF("fs_mkdir"), int, char *, uint16_t)(dirname, permission)
#define fs_create_file(filename, permission) FCASTF(SYF("fs_create_file"), int, char *, uint16_t)(filename, permission)
#define kopen(filename, flags) FCASTF(SYF("kopen"), FILE *, char *, uint32_t)(filename, flags)
#define canonicalize_path(cwd, input) FCASTF(SYF("canonicalize_path"), char *, char *, char *)(cwd, input)
#define fs_clone(source) FCASTF(SYF("fs_clone"), FILE *, FILE *)(source)
#define fs_ioctl(node, request, argp) FCASTF(SYF("fs_ioctl"), int, FILE *, int, void *)(node, request, argp)
#define fs_chmod(node, mode) FCASTF(SYF("fs_chmod"), int, FILE *, int)(node, mode)
#define fs_unlink(filename) FCASTF(SYF("fs_unlink"), int, char *)(filename)
#define fs_symlink(target, filename) FCASTF(SYF("fs_symlink"), int, char *, char *)(target, filename)
#define fs_readlink(node, buff, size) FCASTF(SYF("fs_readlink"), int, FILE *, char *, size_t)(node, buff, size)

/* Tasking library functions: */
#define switch_task(new_process_state) FCASTF(SYF("switch_task"), void, status_t)(new_process_state)
#define fork() FCASTF(SYF("fork"), uint32_t, void)()
#define task_clone(new_stack, thread_function, arg) FCASTF(SYF("task_clone"), uint32_t, uintptr_t, uintptr_t, uintptr_t)(new_stack, thread_function, arg)
#define task_create_tasklet(tasklet, name, argp) FCASTF(SYF("task_create_tasklet"), int, tasklet_t, char *, void *)(tasklet, name, argp)
#define wakeup_queue(queue) FCASTF(SYF("wakeup_queue"), int, list_t *)(queue)
#define sleep_on(queue) FCASTF(SYF("sleep_on"), int, list_t *)(queue)
#define sleep_until(task, seconds, subseconds) FCASTF(SYF("sleep_until"), void, task_t*, unsigned long, unsigned long)(task, seconds, subseconds)
#define kexit(retval) FCASTF(SYF("kexit"), void, int)(retval)
#define current_task_get() FCASTF(SYF("current_task_get"), task_t*, void)()
#define main_task_get() FCASTF(SYF("main_task_get"), task_t*, void)()
#define current_task_getpid() FCASTF(SYF("current_task_getpid"), uint32_t, void)()

#define send_signal(task, signal) FCASTF(SYF("send_signal"), int, pid_t, uint32_t)(task, signal)
#define ksend_signal(task, signal) FCASTF(SYF("ksend_signal"), int, pid_t, uint32_t)(task, signal)
#define handle_signal(proc, sig) FCASTF(SYF("handle_signal"), void, task_t*, signal_t*)(proc, sig)

#endif

typedef int(*irq_handler_t)(Kernel::CPU::regs_t *);

using namespace Kernel;

#endif /* SRC_KERNEL_HEADERS_KHEADERS_H_ */
