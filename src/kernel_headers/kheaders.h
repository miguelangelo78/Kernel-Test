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

#define hashmap_create(count) FCASTF(SYF("hashmap_create"), hashmap_t *, int)(count)
#define hashmap_set(map, key, value) FCASTF(SYF("hashmap_set"), void *, hashmap_t *, void *, void *)(map, key, value)
#define hashmap_remove(map, key) FCASTF(SYF("hashmap_remove"), void *, hashmap_t*, void*)(map, key)
#define hashmap_get_i(map, idx) FCASTF(SYF("hashmap_get_i"), void *, hashmap_t *, int)(map, idx)
#define hashmap_has(map, key) FCASTF(SYF("hashmap_has"), int, hashmap_t *, void*)(map, key)

#define strlen(str) FCASTF(SYF("strlen"), unsigned long, char*)(str)
#define strcpy(dst, src) FCASTF(SYF("strcpy"), char*, char*, char*)(dst, src)

#define memset(dest, c, n) FCASTF(SYF("memset"), void *, void *, int, size_t)(dest, c, n)
#define memcpy(dest, src, n) FCASTF(SYF("memcpy"), void *, void *, void *, size_t)(dest, src, n)

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
#define wakeup_queue(queue) FCASTF(SYF("wakeup_queue"), int, list_t *)(queue)
#define sleep_on(queue) FCASTF(SYF("sleep_on"), int, list_t *)(queue)

#endif

using namespace Kernel;

#endif /* SRC_KERNEL_HEADERS_KHEADERS_H_ */
