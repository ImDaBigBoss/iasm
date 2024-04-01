#include <mnemonics.h>

//See Vol 2A chapter 3 page 492 of the Intel manual for more information

define_mnemonic(HLT) {
    if (line->operand_num != 0) {
        throw_error("The HLT instruction does not take any operands");
    }

    append_opcode(0xF4);
}
