#include <mnemonics.h>

define_mnemonic(JGE) {
    if (line->operand_num != 1) {
        throw_error("Only 1 operand is allowed");
    }

    operand_t* op = &line->operands[0];

    //Signed number
    // jge 0x12345678
    if (op->type == IMMEDIATE_OPERAND) {
        op_immediate_value_t imm_val64;
        imm_val64.value = *((uint64_t*) op->value);
        if (imm_val64.value > UINT32_MAX) {
            throw_error("Value must be 32 bits (max 0xFFFFFFFF), the value is signed");
        }

        typedef union {
            int32_t value;
            uint8_t bytes[4];
        } signed_imm32_t;

        signed_imm32_t imm_val;
        imm_val.value = *((int32_t*) op->value);

        append_opcode(0x0F);
        append_opcode(0x8D);

        for (int i = 0; i < 4; i++) {
            append_opcode(imm_val.bytes[i]);
        }
    }
    //Label
    // jge $label
    else if (op->type == LABEL_OPERAND) {
        append_opcode(0x0F);
        append_opcode(0x8D);

        append_label_reference((char*) op->value, RELATIVE_LABEL_REFERENCE);
        for (int i = 0; i < 4; i++) {
            append_opcode(0x00);
        }
    } else {
        throw_error("Invalid operands, JGE instruction was not understood");
    }
}
