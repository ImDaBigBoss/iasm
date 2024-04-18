#pragma once

#include <asmutils.h>

#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define define_mnemonic(name) \
    void mnemonic_##name(lvl2_line_t* line, segment_data_t* current_segment, bool* errors_found)

#define check_mnemonic(name) \
    else if (strcmp(line->mnemonic, #name) == 0) { \
        mnemonic_##name(line, current_segment, &errors_found); \
    }

#define throw_error(message, ...) \
    printf("Error line %d: "message"\n", line->line_num, ##__VA_ARGS__); \
    *errors_found = true; \
    return;

#define append_opcode(op) \
    current_segment->opcodes = realloc(current_segment->opcodes, sizeof(uint8_t) * (current_segment->opcode_num + 1)); \
    current_segment->opcodes[current_segment->opcode_num] = op; \
    current_segment->opcode_num++;

#define append_label_reference(label_name, replacement_type) \
    current_segment->replacements = realloc(current_segment->replacements, sizeof(replacement_t) * (current_segment->replacement_num + 1)); \
    replacement_t* replacement = &current_segment->replacements[current_segment->replacement_num]; \
    replacement->data = malloc(strlen(label_name) + 1); \
    strcpy((char*) replacement->data, label_name); \
    replacement->address = current_segment->opcode_num; \
    replacement->type = replacement_type; \
    current_segment->replacement_num++;


typedef union {
    uint64_t value;
    uint8_t bytes[8];
} op_immediate_value_t;

#define get_32bit_capped_immediate_value(op, size) \
    op_immediate_value_t imm_val; \
    imm_val.value = *((uint64_t*) op->value); \
    \
    if (size == 8) { \
        if (imm_val.value > UINT32_MAX) { \
            throw_error("Value must be 32 bits (max 0xFFFFFFFF), even though the register is 64 bits"); \
        } \
    } else if (size == 4) { \
        if (imm_val.value > UINT32_MAX) { \
            throw_error("Value must be 32 bits (max 0xFFFFFFFF), since the register is 32 bits"); \
        } \
    } else if (size == 2) { \
        if (imm_val.value > UINT16_MAX) { \
            throw_error("Value must be 16 bits (max 0xFFFF), since the register is 16 bits"); \
        } \
    } else if (size == 1) { \
        if (imm_val.value > UINT8_MAX) { \
            throw_error("Value must be 8 bits (max 0xFF), since the register is 8 bits"); \
        } \
    }

#define get_immediate_value(op, size) \
    op_immediate_value_t imm_val; \
    imm_val.value = *((uint64_t*) op->value); \
    \
    if (size == 8) { \
        if (imm_val.value > UINT64_MAX) { \
            throw_error("Value must be 64 bits (max 0xFFFFFFFFFFFFFFFF), since the register is 64 bits"); \
        } \
    } else if (size == 4) { \
        if (imm_val.value > UINT32_MAX) { \
            throw_error("Value must be 32 bits (max 0xFFFFFFFF), since the register is 32 bits"); \
        } \
    } else if (size == 2) { \
        if (imm_val.value > UINT16_MAX) { \
            throw_error("Value must be 16 bits (max 0xFFFF), since the register is 16 bits"); \
        } \
    } else if (size == 1) { \
        if (imm_val.value > UINT8_MAX) { \
            throw_error("Value must be 8 bits (max 0xFF), since the register is 8 bits"); \
        } \
    }


define_mnemonic(HLT);
//Data
define_mnemonic(DB);
define_mnemonic(DDW);
define_mnemonic(DQW);
define_mnemonic(DW);
//Math
define_mnemonic(ADD);
define_mnemonic(DEC);
define_mnemonic(DIV);
define_mnemonic(INC);
define_mnemonic(MUL);
define_mnemonic(SUB);
//Memory
define_mnemonic(LEA);
define_mnemonic(MOV);
//Misc
define_mnemonic(CMP);
define_mnemonic(INT);
define_mnemonic(SYSCALL);