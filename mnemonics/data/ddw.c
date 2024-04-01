#include <mnemonics.h>

define_mnemonic(DDW) {
    if (line->operand_num < 1) {
        throw_error("The DDW instruction requires at least one operand");
    }

    for (int i = 0; i < line->operand_num; i++) {
        operand_t* operand = &line->operands[i];
        
        if (operand->type == IMMEDIATE_OPERAND) {
            union register_value_t {
                uint64_t value;
                uint8_t bytes[8];
            };
            union register_value_t reg_val;
            reg_val.value = *((uint64_t*) operand->value);

            if (reg_val.value > UINT32_MAX) {
                throw_error("The DQW instruction only takes a 32-bit immediate operand");
            }

            for (int i = 0; i < 4; i++) {
                append_opcode(reg_val.bytes[i]);
            }
        } else {
            throw_error("The DDW instruction only takes an immediate operand");
        }
    }
}
