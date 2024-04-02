#pragma once

#include <stdint.h>

#define ELF_MAGIC 0x464C457F

typedef struct {
    uint32_t magic;
    uint8_t bitness; // 1 = 32-bit, 2 = 64-bit
    uint8_t endian; // 1 = little, 2 = big
    uint8_t header_version;
    uint8_t abi;
    char padding[8];
    uint16_t type; // 1 = relocatable, 2 = executable, 3 = shared object, 4 = core
    uint16_t instruction_set;
    uint32_t elf_version;
    uint64_t entry_point;
    uint64_t program_header_offset;
    uint64_t section_header_offset;
    uint32_t flags;
    uint16_t header_size;
    uint16_t program_entry_size;
    uint16_t program_num_entries;
    uint16_t section_entry_size;
    uint16_t section_num_entries;
    uint16_t section_name_index;
} elf_header_t;

typedef struct {
    uint32_t type; // 0 = null, 1 = program, 2 = dynamic, 3 = interpreter, 4 = note, 5 = shared, 6 = header, 7 = thread local, 8 = GNU_EH_FRAME, 0x60000000 = OS specific, 0x70000000 = processor specific
    uint32_t flags; // 1 = execute, 2 = write, 4 = read
    uint64_t offset;
    uint64_t virtual_address;
    uint64_t physical_address;
    uint64_t file_size;
    uint64_t memory_size;
    uint64_t alignment;
} elf_program_header_t;
