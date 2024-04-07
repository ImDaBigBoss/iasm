#include <mnemonics.h>

define_mnemonic(DQW) {
    if (line->operand_num < 1) {
        throw_error("The DQW instruction requires at least one operand");
    }

    for (int i = 0; i < line->operand_num; i++) {
        operand_t* operand = &line->operands[i];
        
        if (operand->type == IMMEDIATE_OPERAND) {
            op_immediate_value_t imm_val;
            imm_val.value = *((uint64_t*) operand->value);

            if (imm_val.value > UINT64_MAX) {
                throw_error("The DQW instruction only takes a 64-bit immediate operand");
            }

            for (int i = 0; i < 8; i++) {
                append_opcode(imm_val.bytes[i]);
            }
        } else {
            throw_error("The DQW instruction only takes an immediate operand");
        }
    }
}
