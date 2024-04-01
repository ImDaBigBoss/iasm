#include <mnemonics.h>

define_mnemonic(MUL) {
    if (line->operand_num != 1) {
        throw_error("Only 1 operand is allowed");
    }

    operand_t* op1 = &line->operands[0];

    // mul rbx
    if (op1->type == REGISTER_OPERAND && op1->indirect == false) {
        cpu_register_t* reg = (cpu_register_t*) op1->value;

        if (reg->size == 1) {
            append_opcode(0xF6);
        } else if (reg->size == 2) {
            append_opcode(0x66);
            append_opcode(0xF7);
        } else if (reg->size == 4) {
            append_opcode(0xF7);
        } else {
            append_opcode(0x48);
            append_opcode(0xF7);
        }

        append_opcode(0xE0 + reg->code);
    } else {
        throw_error("Invalid operand, MUL instruction was not understood");
    }
}
