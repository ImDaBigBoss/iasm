#include <mnemonics.h>

//See Vol 2A chapter 3 page 516 of the Intel manual for more information

define_mnemonic(CMP) {
    if (line->operand_num != 2) {
        throw_error("Only 2 operands are allowed");
    }

    operand_t* op1 = &line->operands[0];
    operand_t* op2 = &line->operands[1];

    //Register -> register
    // cmp rax, rbx
    if (op1->type == REGISTER_OPERAND && op1->indirect == false && op2->type == REGISTER_OPERAND && op2->indirect == false) {
        cpu_register_t* reg1 = (cpu_register_t*) op1->value;
        cpu_register_t* reg2 = (cpu_register_t*) op2->value;

        if (reg1->size != reg2->size) {
            throw_error("The size of the registers to compare must be the same");
        }

        if (reg1->size == 1) {
            append_opcode(0x38);
        } else if (reg1->size == 2) {
            append_opcode(0x66);
            append_opcode(0x39);
        } else if (reg1->size == 4) {
            append_opcode(0x39);
        } else {
            append_opcode(0x48);
            append_opcode(0x39);
        }

        append_opcode(0xC0 + get_two_register_magic(reg1, reg2));
    }
    //Number -> register
    // cmp rax, 0x12345678
    else if (op1->type == REGISTER_OPERAND && op1->indirect == false && op2->type == IMMEDIATE_OPERAND) {
        cpu_register_t* destination = (cpu_register_t*) op1->value;
        get_32bit_capped_immediate_value(op2, destination->size);

        int size = destination->size;
        if (destination->size == 8) {
            size = 4;
        }

        if (destination->code == R0) { //The are special opcodes for adding to the first register
            if (destination->size == 1) {
                append_opcode(0x3C);
            } else if (destination->size == 2) {
                append_opcode(0x66);
                append_opcode(0x3D);
            } else if (destination->size == 4) {
                append_opcode(0x3D);
            } else {
                append_opcode(0x48);
                append_opcode(0x3D);
            }
        } else {
            if (destination->size == 1) {
                append_opcode(0x80);
            } else if (destination->size == 2) {
                append_opcode(0x66);
                append_opcode(0x81);
            } else if (destination->size == 4) {
                append_opcode(0x81);
            } else {
                append_opcode(0x48);
                append_opcode(0x81);
            }

            append_opcode(0xF8 + destination->code);
        }

        for (int i = 0; i < size; i++) {
            append_opcode(imm_val.bytes[i]);
        }
    }
}
