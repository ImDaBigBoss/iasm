#include <mnemonics.h>

//See Vol 2B chapter 4 page 35 of the Intel manual for more information

define_mnemonic(MOV) {
    if (line->operand_num != 2) {
        throw_error("Only 2 operands are allowed");
    }

    operand_t* op1 = &line->operands[0];
    operand_t* op2 = &line->operands[1];

    if (op1->type == REGISTER_OPERAND) {
        cpu_register_t* reg = (cpu_register_t*) op1->value;
        if (reg->code >= 8) {
            throw_error("Those registers aren't supported yet");
        }
    }
    if (op2->type == REGISTER_OPERAND) {
        cpu_register_t* reg = (cpu_register_t*) op2->value;
        if (reg->code >= 8) {
            throw_error("Those registers aren't supported yet");
        }
    }

    //Register -> register
    // mov rax, rbx
    if (op1->type == REGISTER_OPERAND && op1->indirect == false && op2->type == REGISTER_OPERAND && op2->indirect == false) {
        cpu_register_t* destination = (cpu_register_t*) op1->value;
        cpu_register_t* source = (cpu_register_t*) op2->value;

        if (destination->size != source->size) {
            throw_error("The size of the destination register must be the same as the size of the source register");
        }

        if (destination->size == 1) {
            append_opcode(0x88);
        } else if (destination->size == 2) {
            append_opcode(0x66);
            append_opcode(0x89);
        } else if (destination->size == 4) {
            append_opcode(0x89);
        } else {
            append_opcode(0x48);
            append_opcode(0x89);
        }

        //0xC0 is the mod field for a register to register move (0b11 << 6)
        append_opcode(0xC0 + get_two_register_magic(destination, source));
    }
    //Number -> register
    // mov rax, 0x12345678
    else if (op1->type == REGISTER_OPERAND && op1->indirect == false && op2->type == IMMEDIATE_OPERAND) {
        cpu_register_t* destination = (cpu_register_t*) op1->value;
        get_immediate_value(op2, destination->size);

        if (destination->size == 1) {
            append_opcode(0xB0 + destination->code);
        } else if (destination->size == 2) {
            append_opcode(0x66);
            append_opcode(0xB8 + destination->code);
        } else if (destination->size == 4) {
            append_opcode(0xB8 + destination->code);
        } else {
            append_opcode(0x48);
            append_opcode(0xB8 + destination->code);
        }

        for (int i = 0; i < destination->size; i++) {
            append_opcode(imm_val.bytes[i]);
        }
    }
    //Number -> address in register
    // mov [rax], 0x12345678
    else if (op1->type == REGISTER_OPERAND && op1->indirect && op2->type == IMMEDIATE_OPERAND) {
        op_immediate_value_t imm_val;
        imm_val.value = *((uint64_t*) op2->value);
        if (imm_val.value > UINT32_MAX) {
            throw_error("The immediate must be 32 bits (max 0xFFFFFFFF)");
        }

        cpu_register_t* destination = (cpu_register_t*) op1->value;
        if (destination->size == 1 || destination->size == 2) {
            throw_error("The destination register must be 32 or 64 bit");
        }

        if (imm_val.value <= UINT8_MAX) {
            if (destination->size == 4) {
                append_opcode(0x67);
            }
            append_opcode(0xC6);
            append_opcode(destination->code);
            append_opcode(imm_val.bytes[0]);
        } else if (imm_val.value <= UINT16_MAX) {
            append_opcode(0x66);
            if (destination->size == 4) {
                append_opcode(0x67);
            }
            append_opcode(0xC7);
            append_opcode(destination->code);
            append_opcode(imm_val.bytes[0]);
            append_opcode(imm_val.bytes[1]);
        } else {
            if (destination->size == 4) {
                append_opcode(0x67);
            }
            append_opcode(0xC7);
            append_opcode(destination->code);
            for (int i = 0; i < 4; i++) {
                append_opcode(imm_val.bytes[i]);
            }
        }
    }
    //Register -> address in register / Address in register -> register
    // mov [rax], rbx / mov rax, [rbx]
    else if (op1->type == REGISTER_OPERAND && op2->type == REGISTER_OPERAND && (op1->indirect != op2->indirect)) {
        cpu_register_t* destination = (cpu_register_t*) op1->value;
        cpu_register_t* source = (cpu_register_t*) op2->value;

        if (destination->size == 1 || destination->size == 2) {
            throw_error("The destination register must be 32 or 64 bit");
        }
        if (op1->indirect && op2->indirect) {
            throw_error("Both operands can't be indirect");
        }

        if (source->size == 2) {
            append_opcode(0x66);
        } else if (source->size == 4) {
            append_opcode(0x67);
        } else if (source->size == 8) {
            append_opcode(0x48);
        }

        if (op1->indirect) {
            if (source->size == 1) {
                append_opcode(0x88);
            } else {
                append_opcode(0x89);
            }
        } else {
            if (source->size == 1) {
                append_opcode(0x8A);
            } else {
                append_opcode(0x8B);
            }
        }

        uint8_t modrm = get_two_register_magic(source, destination);
        if (op1->displacement != 0) {
            //TODO: Support other sizes of displacement
            //We want to add a int32 displacement, so we set the mod field to 0b10
            append_opcode(modrm + 0x80);

            op_immediate_value_t imm_val;
            imm_val.value = op1->displacement;

            for (int i = 0; i < 4; i++) {
                append_opcode(imm_val.bytes[i]);
            }
        } else {
            append_opcode(modrm);
        }
    }
    //Label address -> register
    // mov rax, $label
    else if (op1->type == REGISTER_OPERAND && op1->indirect == false && op2->type == LABEL_OPERAND && op2->indirect == false) {
        cpu_register_t* destination = (cpu_register_t*) op1->value;

        if (destination->size != 8) {
            throw_error("The destination register for a label must be 64 bit");
        }

        append_opcode(0x48);
        append_opcode(0x8B);
        append_opcode((8 * ((uint8_t) destination->code)) + 5);

        append_label_reference((char*) op2->value, ABSOULTE_LABEL_REFERENCE);
        for (int i = 0; i < 8; i++) {
            append_opcode(0x00);
        }
    } else {
        throw_error("Invalid operands, MOV instruction was not understood");
    }
}
