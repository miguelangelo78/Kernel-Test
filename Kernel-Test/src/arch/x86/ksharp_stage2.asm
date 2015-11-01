;*************************************************************
;*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;*
;*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;*
;*;;;;;;;;;;;;;;;;; SECOND STAGE BOOTLOADER ;;;;;;;;;;;;;;;;;*
;*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;*
;*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;*
;*************************************************************

BITS 32

GLOBAL _Kernel_Start:function
EXTERN _Z5kmainv

KERNEL_VIRTUAL_BASE equ 0xC0000000					; Constant declaring base of Higher-half kernel
KERNEL_PAGE_TABLE equ (KERNEL_VIRTUAL_BASE >> 22)	; Constant declaring Page Table index in virtual memory

SECTION .text

; BEGIN - Multiboot Header
MultibootSignature dd 464367618
MultibootFlags dd 3
MultibootChecksum dd -464367621
MultibootGraphicsRuntime_VbeModeInfoAddr dd 2147483647
MultibootGraphicsRuntime_VbeControlInfoAddr dd 2147483647
MultibootGraphicsRuntime_VbeMode dd 2147483647
MultibootInfo_Memory_High dd 0
MultibootInfo_Memory_Low dd 0
; END - Multiboot Header

MultibootInfo_Structure dd 0

; BEGIN - Kernel stack space allocation
Kernel_Stack_End:
	TIMES 65535 db 0
Kernel_Stack_Start:
; END - Kernel stack space allocation

; BEGIN - GDT allocations
; This is the GDT table pre-filled with the entries we want
GDT_Contents:
;	Code and data selectors first then TSS
db 0, 0, 0, 0, 0, 0, 0, 0	; Offset: 0  - Null selector - required
db 255, 255, 0, 0, 0, 0x9A, 0xCF, 0	; Offset: 8  - KM Code selector - covers the entire 4GiB address range
db 255, 255, 0, 0, 0, 0x92, 0xCF, 0	; Offset: 16 - KM Data selector - covers the entire 4GiB address range
db 255, 255, 0, 0, 0, 0xFA, 0xCF, 0	; Offset: 24 - UM Code selector - covers the entire 4GiB address range
db 255, 255, 0, 0, 0, 0xF2, 0xCF, 0	; Offset: 32 - UM Data selector - covers the entire 4GiB address range

; Size - Change iff adding/removing rows from GDT contents
; Size = Total bytes in GDT - 1
GDT_Pointer db 39, 0, 0, 0, 0, 0
; END - GDT allocations

; BEGIN - IDT Allocations
IDT_Contents: TIMES 2048 db 0
IDT_Pointer db 0xFF, 0x7, 0, 0, 0, 0
; END - IDT Allocations

;;;;; FUNCTIONS ;;;;;

MultibootCheck:
	mov dword ECX, 0x2BADB002
	cmp ECX, EAX
	jne (HndNoMultiboot - KERNEL_VIRTUAL_BASE)
	ret

SwitchProtected:
	mov dword EAX, CR0
	or EAX, 1
	mov dword CR0, EAX
	ret

GDTInstall:
	mov dword [MultibootInfo_Structure - KERNEL_VIRTUAL_BASE], EBX
	add dword EBX, 0x4
	mov dword EAX, [EBX]
	mov dword [MultibootInfo_Memory_Low - KERNEL_VIRTUAL_BASE], EAX
	add dword EBX, 0x4
	mov dword EAX, [EBX]
	mov dword [MultibootInfo_Memory_High - KERNEL_VIRTUAL_BASE], EAX

	mov dword [GDT_Pointer - KERNEL_VIRTUAL_BASE + 2], (GDT_Contents - KERNEL_VIRTUAL_BASE)
	mov dword EAX, (GDT_Pointer - KERNEL_VIRTUAL_BASE)
	lgdt [EAX]
	ret

GDTFlush:
	jmp SegmentSetDone

SegmentSet:
	mov dword EAX, 0x10
	mov word DS, EAX
	mov word ES, EAX
	mov word FS, EAX
	mov word GS, EAX
	mov word SS, EAX
	jmp 8:(GDTFlush - KERNEL_VIRTUAL_BASE)
	ret

SetVirtualMemory:
	; Step 1: Map virtual memory for physical address execution
	lea EAX, [Page_Table1 - KERNEL_VIRTUAL_BASE]
	mov EBX, 0b111
	mov ECX, (4 * 1024) ; (# of Page Tables) * (# of entries per Page Table)
	.Loop1:
	mov [EAX], EBX
	add EAX, 4 ; Move to the next entry in the Page Table (4 bytes down)
	add EBX, 0x1000 ; Update physical address to which to set the next Page Table entry to (4 KiB down)
	loop .Loop1

	lea EAX, [Page_Table1 - KERNEL_VIRTUAL_BASE]
	add EAX, (KERNEL_PAGE_TABLE * 1024 * 4)
	mov EBX, 0b111 ; 0b111 - Setting Page Table flags (Present: ON, Read/Write: ON, User/Supervisor: ON)
	mov ECX, (4 * 1024) ; (# of Page Tables) * (# of entries per Page Table)
	.Loop2:
	mov [EAX], EBX
	add EAX, 4 ; Move to the next entry in the Page Table (4 bytes down)
	add EBX, 0x1000 ; Update physical address to which to set the next Page Table entry to (4 KiB down)
	loop .Loop2

	lea EBX, [Page_Table1 - KERNEL_VIRTUAL_BASE]
	lea EDX, [Page_Directory - KERNEL_VIRTUAL_BASE]
	or EBX, 0b111 ; 0b111 - Setting Page Table flags (Present: ON, Read/Write: ON, User/Supervisor: ON)
	mov ECX, 1024 ; (# of Page Tables) * (# of entries per Page Table)
	.Loop3:
	mov [EDX], EBX
	add EDX, 4 ; Move to the next entry in the Page Table (4 bytes down)
	add EBX, 0x1000 ; Update physical address to which to set the next Page Table entry to (4 KiB down)
	loop .Loop3

	; Step 2: Set page directory
	; This requires us to load the physical address of the page directory
	; then move it into CR3

	lea ECX, [Page_Directory - KERNEL_VIRTUAL_BASE]
	mov CR3, ECX

	; Step 3: Enable Paging
	; This requires us to enable paging by setting CR0
	mov ECX, CR0
	or ECX, 0x80000000 ; Set PG bit in CR0 to enable paging.
	mov CR0, ECX ; Enable it!

	; At this point, paging should be enabled

	lea ECX, [Kernel_Start_HigherHalf]
	jmp ECX

; ****************************
; ****** KERNEL START ********
; ****************************

_Kernel_Start:
	; Initialize the important stuff:
	cli
	call MultibootCheck
	call GDTInstall
	call SwitchProtected
	mov dword ESP, (Kernel_Stack_Start - KERNEL_VIRTUAL_BASE) ; Set stack pointer
	call SegmentSet
	SegmentSetDone:
	call SetVirtualMemory

	Kernel_Start_HigherHalf: ; We're done initializing. We will initialize the rest with C code.
	nop
	lea EAX, [_Z5kmainv - KERNEL_VIRTUAL_BASE]
	call EAX
	jmp KernelExit

HndNoMultiboot:
	; SET SCREEN COLOR TO RED:
	mov dword EAX, 0x4F
	mov dword EBX, 0xB8000
	mov dword ECX, 2000
	.ColorOut:
	mov byte [EBX], 0
	mov byte [EBX+1], AL
	add EBX, 2
	loop .ColorOut
	; PRINT ERROR MESSAGE:
	mov dword ECX, 26
	mov dword EAX, 0xB8000
	mov dword EBX, 0
	.PrintNoMulti:
	mov byte DL, [NoMultibootMsg+EBX]
	mov byte [EAX], DL
	add EAX, 2
	inc EBX
	loop .PrintNoMulti
	jmp Halt

KernelExit:
	; SET SCREEN COLOR TO RED:
	mov dword EAX, 0x1E
	mov dword EBX, 0xB8000
	mov dword ECX, 2000
	.ColorOut:
	mov byte [EBX], 0
	mov byte [EBX+1], AL
	add EBX, 2
	loop .ColorOut
	mov dword ECX, 38
	mov dword EAX, 0xB8000
	mov dword EBX, 0
	.PrintNoMulti:
	mov byte DL, [KernelExitMsg+EBX]
	mov byte [EAX], DL
	add EAX, 2
	inc EBX
	loop .PrintNoMulti

Halt:
	cli
	hlt
	jmp Halt

NoMultibootMsg: db "PANIC: No multiboot found!", 0
KernelExitMsg: db "INFO: The Kernel has exited. Halting...", 0

SECTION .paging_sect

align 0x1000
Page_Table1: resb(1024 * 1024 * 4) ; Reserve uninitialized space for paging table
Page_Directory: resb(1024 * 4) ; Reserve uninitialized space for paging directory
