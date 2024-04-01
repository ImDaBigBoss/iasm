#include <mnemonics.h>

//See Vol 2A chapter 3 page 598 of the Intel manual for more information

define_mnemonic(LEA) {
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
        if (reg->size <= 2) {
            throw_error("The destination register must be 32 or 64 bit");
        }
    }
    if (op2->type == REGISTER_OPERAND) {
        cpu_register_t* reg = (cpu_register_t*) op2->value;
        if (reg->code >= 8) {
            throw_error("Those registers aren't supported yet");
        }
        if (reg->size <= 2) {
            throw_error("The source register must be 32 or 64 bit");
        }
    }

    // lea rax, [rbx]
    if (op1->type == REGISTER_OPERAND && op1->indirect == false && op2->type == REGISTER_OPERAND && op2->indirect) {
        cpu_register_t* destination = (cpu_register_t*) op1->value;
        cpu_register_t* source = (cpu_register_t*) op2->value;

        if (destination->size != source->size) {
            throw_error("The size of the destination register must be the same as the size of the source register");
        }

        if (destination->size == 4) {
            append_opcode(0x67);
        } else if (destination->size == 8) {
            append_opcode(0x48);
        }

        append_opcode(0x8D);

        uint8_t modrm = get_two_register_magic(source, destination);
        if (op2->displacement != 0) {
            //We want to add a int32 displacement, so we set the mod field to 0b10
            append_opcode(modrm + 0x80);

            union register_value_t {
                uint32_t value;
                uint8_t bytes[4];
            };
            union register_value_t reg_val;
            reg_val.value = op2->displacement;

            for (int i = 0; i < 4; i++) {
                append_opcode(reg_val.bytes[i]);
            }
        } else {
            append_opcode(modrm);
        }
    }
    // lea rax, [rel $label]
    else if (op1->type == REGISTER_OPERAND && op1->indirect == false && op2->type == LABEL_OPERAND && op2->indirect) {
        cpu_register_t* destination = (cpu_register_t*) op1->value;

        if (destination->size == 4) {
            append_opcode(0x67);
        } else if (destination->size == 8) {
            append_opcode(0x48);
        }

        append_opcode(0x8D);
        append_opcode(0x35); //ModRM byte for RIP relative addressing

        append_relative_label_reference((char*) op2->value);
        for (int i = 0; i < 4; i++) {
            append_opcode(0x00);
        }
    } else {
        throw_error("Invalid operands, LEA instruction was not understood");
    }
}
