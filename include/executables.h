#pragma once

#include <asmutils.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef enum {
    BIN_FORMAT,
    FEXEC_FORMAT,
    MACHO64_FORMAT,
    ELF_FORMAT
} executable_format_t;

typedef struct {
    uint64_t segment_offsets[SEGMENT_TYPE_NUM];
    segment_data_t* segments[SEGMENT_TYPE_NUM];
} executable_data_t;

#define PAGE_ROUND_UP(address) ((address + 0xfff) & ~0xfff)

executable_format_t get_exec_type(char* name);

bool write_exec(executable_format_t format, FILE* file, uint64_t entry_offset, segment_data_t* segments, label_t* label_definitions, int label_definition_num);

void make_executable(FILE* file);

bool do_executable_replacements(executable_data_t exec_data, label_t* label_definitions, int label_definition_num);

#define define_exec_write(format) \
    bool write_##format(FILE* file, uint64_t entry_offset, segment_data_t* segments, label_t* label_definitions, int label_definition_num)

#define call_exec_write(format) \
    write_##format(file, entry_offset, segments, label_definitions, label_definition_num)

define_exec_write(bin);
define_exec_write(fexec);
define_exec_write(macho64);
define_exec_write(elf);
