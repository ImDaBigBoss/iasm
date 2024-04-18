.text

; Write to stdout
mov rax, 0x1 ; sys_write
mov rdi, 1 ; stdout
lea rsi, [rel $message]
mov rdx, 14 ; length
syscall

; Clean exit
mov rax, 0x3C ; sys_exit
mov rbx, 0 ; exit code
syscall

.rodata

message:
    db "Hello, World!", 0xa, 0