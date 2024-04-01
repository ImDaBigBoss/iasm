# Assembler

There is nothing special about this assembler, it supports a few basic mnemonics and can output a few different formats accessible using the `-f` flag.
 * Binary (`bin`) - A raw binary file
 * Fexec (`fexec`) - A custom format for an operating system I've worked on
 * Mach-O 64 (`macho64`) - A format for macOS 64-bit

## Mnemonics

Below is a table for extra mnemonics that are not opcodes, but that the assembler will understand. It is followed by an example snippet of code.

| Mnemonic | Description |
|----------|-------------|
| DB       | Define some raw data (can be any size or type) |
| DW       | Define a word (2 bytes) |
| DDW       | Define a double word (4 bytes) |
| DQW       | Define a quad word (8 bytes) |

```asm
db "Hello world", 0x20, 0x0
db 0xcafebabe, 0x123

dw 0x1234, 0x5678
ddw 0x12345678
dqw 0x123456789abcdef0
```

## Labels

Labels are defined by a name followed by a colon (`:`). They are referenced by a dollar sign (`$`) followed by the label name.
```asm
label_1:
    mov r1, 0x0
    jmp $label_2

label_2:
    mov r2, 0x1
    jmp $label_1
```

## Comments

Comments are defined by a semicolon (`;`).
```asm
; This is a comment
```

## Addresses in registers

Addresses in registers are defined by square brackets (`[]`) followed by a register name. They can also be offset by a constant (positive or negative) by adding or subtracting the constant after the register name.

```asm
mov rax, [rbx]
mov rax, [rbx + 1]
mov rax, [rbx - 1]
```

## RIP relative addressing

You can reference labels using RIP relative addressing, the assembler will calculate the offset from the current instruction to the label and insert it into the instruction.

```asm
lea rax, [rel $label]

label:
    db "Hello world", 0x0
```

The above example could be used if you wanted to load addresses of labels in position independent code.

## Directives

There are currently no directives.

## Macros

Macros are not supported.
