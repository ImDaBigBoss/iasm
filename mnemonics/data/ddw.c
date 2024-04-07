#include <mnemonics.h>

define_mnemonic(DDW) {
    if (line->operand_num < 1) {
        throw_error("The DDW instruction requires at least one operand");
    }

    for (int i = 0; i < line->operand_num; i++) {
        operand_t* operand = &line->operands[i];
        
        if (operand->type == IMMEDIATE_OPERAND) {
            op_immediate_value_t imm_val;
            imm_val.value = *((uint64_t*) operand->value);

            if (imm_val.value > UINT32_MAX) {
                throw_error("The DDW instruction only takes a 32-bit immediate operand");
            }

            for (int i = 0; i < 4; i++) {
                append_opcode(imm_val.bytes[i]);
            }
        } else {
            throw_error("The DDW instruction only takes an immediate operand");
        }
    }
}
