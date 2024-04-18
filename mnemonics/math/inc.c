#include <mnemonics.h>

define_mnemonic(INC) {
    if (line->operand_num != 1) {
        throw_error("Only 1 operand is allowed");
    }

    operand_t* op = &line->operands[0];

    // inc rax
    if (op->type == REGISTER_OPERAND && op->indirect == false) {
        cpu_register_t* reg = (cpu_register_t*) op->value;

        if (reg->size == 1) {
            append_opcode(0xFE);
        } else if (reg->size == 2) {
            append_opcode(0x66);
            append_opcode(0xFF);
        } else if (reg->size == 4) {
            append_opcode(0xFF);
        } else {
            append_opcode(0x48);
            append_opcode(0xFF);
        }

        append_opcode(0xC0 + reg->code);
    }
    // inc [rax]
    else if (op->type == REGISTER_OPERAND && op->indirect == true) {
        cpu_register_t* reg = (cpu_register_t*) op->value;

        if (reg->size == 1) {
            append_opcode(0xFE);
        } else if (reg->size == 2) {
            append_opcode(0x66);
            append_opcode(0xFF);
        } else if (reg->size == 4) {
            append_opcode(0xFF);
        } else {
            append_opcode(0x48);
            append_opcode(0xFF);
        }

        if (op->displacement != 0) {
            //TODO: Support other sizes of displacement
            //We want to add a int32 displacement, so we set the mod field to 0b10
            append_opcode(reg->code + 0x80);

            op_immediate_value_t imm_val;
            imm_val.value = op->displacement;

            for (int i = 0; i < 4; i++) {
                append_opcode(imm_val.bytes[i]);
            }
        } else {
            append_opcode(reg->code);
        }
    } else {
        throw_error("Invalid operand, INC instruction was not understood");
    }
}
