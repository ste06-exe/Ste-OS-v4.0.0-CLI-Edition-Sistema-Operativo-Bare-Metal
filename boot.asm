; boot.asm - Bootloader Multiboot v1 per Ste OS v4.0.0
MBOOT_PAGE_ALIGN    equ 1 << 0
MBOOT_MEM_INFO      equ 1 << 1
MBOOT_HEADER_MAGIC  equ 0x1BADB002
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

section .multiboot
align 4
    dd MBOOT_HEADER_MAGIC
    dd MBOOT_HEADER_FLAGS
    dd MBOOT_CHECKSUM

section .text
global _start
global detect_cpu
global detect_cpu_features
extern kernel_main

_start:
    mov esp, stack_top      ; Imposta il puntatore dello stack
    cli                     ; Disabilita gli interrupt hardware
    call kernel_main        ; Passa il controllo al kernel in C

.hang:
    hlt                     ; Mette la CPU in stato di riposo
    jmp .hang

; Legge il Vendor ID della CPU (es. GenuineIntel / AuthenticAMD)
detect_cpu:
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    push edx
    mov eax, 0
    cpuid
    mov eax, [ebp + 8]
    mov [eax], ebx
    mov [eax + 4], edx
    mov [eax + 8], ecx
    mov byte [eax + 12], 0
    pop edx
    pop ecx
    pop ebx
    pop ebp
    ret

; Legge i flag delle funzionalità della CPU (es. supporto 64-bit)
detect_cpu_features:
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    push edx
    mov eax, 1
    cpuid
    mov eax, [ebp + 8]
    mov [eax], edx
    mov eax, [ebp + 12]
    mov [eax], ecx
    pop edx
    pop ecx
    pop ebx
    pop ebp
    ret

section .bss
align 16
stack_bottom:
    resb 16384              ; Riserva 16 KB per lo stack
stack_top: