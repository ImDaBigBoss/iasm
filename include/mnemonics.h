#pragma once

#include <asmutils.h>

#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    char* name;
    int local_address;
} label_t;

typedef enum {
    ABSOULTE_LABEL_REFERENCE,
    RELATIVE_LABEL_REFERENCE
} replacement_type_t;

typedef struct {
    void* data;
    replacement_type_t type;
    int address;
} replacement_t;

#define define_mnemonic(name) \
    void mnemonic_##name(lvl2_line_t* line, uint8_t** opcodes, int* opcode_num, replacement_t** replacements, int* replacement_num, bool* errors_found)

#define check_mnemonic(name) \
    else if (strcmp(line->mnemonic, #name) == 0) { \
        mnemonic_##name(line, opcodes, opcode_num, &replacements, &replacement_num, &errors_found); \
    }

#define throw_error(message, ...) \
    printf("Error line %d: "message"\n", line->line_num, ##__VA_ARGS__); \
    *errors_found = true; \
    return;

#define append_opcode(op) \
    *opcodes = realloc(*opcodes, sizeof(uint8_t) * (*opcode_num + 1)); \
    (*opcodes)[*opcode_num] = op; \
    (*opcode_num)++;

#define append_absolute_label_reference(label_name) \
    *replacements = realloc(*replacements, sizeof(replacement_t) * (*replacement_num + 1)); \
    (*replacements)[*replacement_num].data = malloc(strlen(label_name) + 1); \
    strcpy((char*) (*replacements)[*replacement_num].data, label_name); \
    (*replacements)[*replacement_num].address = *opcode_num; \
    (*replacements)[*replacement_num].type = ABSOULTE_LABEL_REFERENCE; \
    (*replacement_num)++;

#define append_relative_label_reference(label_name) \
    *replacements = realloc(*replacements, sizeof(replacement_t) * (*replacement_num + 1)); \
    (*replacements)[*replacement_num].data = malloc(strlen(label_name) + 1); \
    strcpy((char*) (*replacements)[*replacement_num].data, label_name); \
    (*replacements)[*replacement_num].address = *opcode_num; \
    (*replacements)[*replacement_num].type = RELATIVE_LABEL_REFERENCE; \
    (*replacement_num)++;

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
define_mnemonic(INT);
define_mnemonic(SYSCALL);