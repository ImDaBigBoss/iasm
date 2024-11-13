#include <mnemonics.h>

define_mnemonic(CALL) {
    if (line->operand_num != 1) {
        throw_error("Only 1 operand is allowed");
    }

    operand_t* op = &line->operands[0];

    //Register
    // call rax
    if (op->type == REGISTER_OPERAND && op->indirect == false) {
        cpu_register_t* reg = (cpu_register_t*) op->value;

        if (reg->size == 1) {
            throw_error("The register must be at least 2 bytes");
        } else if (reg->size == 2) {
            append_opcode(0x66);
            append_opcode(0xFF);
        } else if (reg->size == 4) {
            append_opcode(0xFF);
        } else {
            append_opcode(0x48);
            append_opcode(0xFF);
        }

        append_opcode(0xD0 + reg->code);
    }
    //Signed number
    // call 0x12345678
    else if (op->type == IMMEDIATE_OPERAND) {
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

        append_opcode(0xE8);

        for (int i = 0; i < 4; i++) {
            append_opcode(imm_val.bytes[i]);
        }
    }
    //Label
    // call $label
    else if (op->type == LABEL_OPERAND) {
        append_opcode(0xE8);

        append_label_reference((char*) op->value, RELATIVE_LABEL_REFERENCE);
        for (int i = 0; i < 4; i++) {
            append_opcode(0x00);
        }
    } else {
        throw_error("Invalid operands, JMP instruction was not understood");
    }
}
