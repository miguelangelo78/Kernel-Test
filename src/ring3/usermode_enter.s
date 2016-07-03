/* Enter userspace (ring3) */
.global usermode_enter_asm
.type usermode_enter_asm, @function

.set MAGIC, 0xDECADE21

usermode_enter_asm:
    pushl %ebp
    mov %esp, %ebp
    mov 0xC(%ebp), %edx
    mov %edx, %esp
    pushl $MAGIC

    /* Segment selector */
    mov $0x23,%ax

    /* Save segment registers */
    mov %eax, %ds
    mov %eax, %es
    mov %eax, %fs
    mov %eax, %gs
    /* %ss is handled by iret */

    /* Store stack address in %eax */
    mov %esp, %eax

    /* Data segmenet with bottom 2 bits set for ring3 */
    pushl $0x23

    /* Push the stack address */
    pushl %eax

    /* Push flags and fix interrupt flag */
    pushf
    popl %eax

    /* Request ring3 */
    orl $0x200, %eax /* Enable interrupt flag */
    pushl %eax
    pushl $0x1B

    /* Push entry point */
    pushl 0x8(%ebp)

    iret
    popl %ebp
    ret
