#include <mnemonics.h>

define_mnemonic(DEC) {
    if (line->operand_num != 1) {
        throw_error("Only 1 operand is allowed");
    }

    operand_t* op1 = &line->operands[0];

    // dec rax
    if (op1->type == REGISTER_OPERAND && op1->indirect == false) {
        cpu_register_t* reg = (cpu_register_t*) op1->value;

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

        append_opcode(0xC8 + reg->code);
    } else {
        throw_error("Invalid operand, DEC instruction was not understood");
    }
}
