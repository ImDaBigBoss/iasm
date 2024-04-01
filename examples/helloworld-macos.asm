; Write to stdout
mov rax, 0x2000004 ; sys_write
mov rdi, 1 ; stdout
lea rsi, [rel $message]
mov rdx, 14 ; length
syscall

; Clean exit
mov rax, 0x2000001 ; sys_exit
mov rbx, 0 ; exit code
syscall

message:
    db "Hello, World!", 0xa, 0