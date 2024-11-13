#include <mnemonics.h>

define_mnemonic(RET) {
    if (line->operand_num == 0) {
        append_opcode(0xC3);
    } else if (line->operand_num == 1) {
        operand_t* op = &line->operands[0];

        if (op->type == IMMEDIATE_OPERAND) {
            op_immediate_value_t imm_val;
            imm_val.value = *((uint64_t*) op->value);

            if (imm_val.value > UINT16_MAX) {
                throw_error("The immediate value must be 16 bits (max 0xFFFF)");
            }

            append_opcode(0xC2);
            append_opcode(imm_val.bytes[0]);
            append_opcode(imm_val.bytes[1]);
        } else {
            throw_error("The operand must be an immediate value");
        }
    } else {
        throw_error("The RET instruction takes either one or no operands");
    }
}
