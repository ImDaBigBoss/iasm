#include <mnemonics.h>

define_mnemonic(DW) {
    if (line->operand_num < 1) {
        throw_error("The DW instruction requires at least one operand");
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

            if (reg_val.value > UINT16_MAX) {
                throw_error("The DW instruction only takes a 16-bit immediate operand");
            }

            for (int i = 0; i < 2; i++) {
                append_opcode(reg_val.bytes[i]);
            }
        } else {
            throw_error("The DW instruction only takes an immediate operand");
        }
    }
}
