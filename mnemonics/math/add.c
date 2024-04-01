#include <mnemonics.h>

define_mnemonic(ADD) {
    if (line->operand_num != 2) {
        throw_error("Only 2 operands are allowed");
    }

    operand_t* op1 = &line->operands[0];
    operand_t* op2 = &line->operands[1];

    // add rax, rbx
    if (op1->type == REGISTER_OPERAND && op1->indirect == false && op2->type == REGISTER_OPERAND && op2->indirect == false) {
        cpu_register_t* destination = (cpu_register_t*) op1->value;
        cpu_register_t* source = (cpu_register_t*) op2->value;

        if (destination->size != source->size) {
            throw_error("The size of the destination register must be the same as the size of the source register");
        }

        if (destination->size == 1) {
            append_opcode(0x00);
        } else if (destination->size == 2) {
            append_opcode(0x66);
            append_opcode(0x01);
        } else if (destination->size == 4) {
            append_opcode(0x01);
        } else {
            append_opcode(0x48);
            append_opcode(0x01);
        }

        //0xC0 is the mod field for a register to register move (0b11 << 6)
        append_opcode(0xC0 + get_two_register_magic(destination, source));
    }
    // add rax, 1
    else if (op1->type == REGISTER_OPERAND && op1->indirect == false && op2->type == IMMEDIATE_OPERAND) {
        cpu_register_t* destination = (cpu_register_t*) op1->value;

        union register_value_t {
            uint64_t value;
            uint8_t bytes[8];
        };
        union register_value_t reg_val;
        reg_val.value = *((uint64_t*) op2->value);

        int size = destination->size;
        if (destination->size == 8) {
            size = 4;
            if (reg_val.value > UINT32_MAX) {
                throw_error("Value must be 32 bits (max 0xFFFFFFFF), even though the register is 64 bits");
            }
        }

        if (destination->code == R0) { //The are special opcodes for adding to the a register
            if (destination->size == 1) {
                append_opcode(0x04);
            } else if (destination->size == 2) {
                append_opcode(0x66);
                append_opcode(0x05);
            } else if (destination->size == 4) {
                append_opcode(0x05);
            } else {
                append_opcode(0x48);
                append_opcode(0x05);
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

            append_opcode(0xC0 + destination->code);
        }

        for (int i = 0; i < size; i++) {
            append_opcode(reg_val.bytes[i]);
        }
    } else {
        throw_error("Invalid operands, ADD instruction was not understood");
    }
}
