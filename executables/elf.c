#include <executables.h>
#include <executables/elf.h>

define_exec_write(elf) {
    uint64_t virtual_offset = 0x400000;

    int program_header_count = 0;

    for (int i = 0; i < SEGMENT_TYPE_NUM; i++) {
        int segment_size = segments[i].opcode_num;
        if (segment_size == 0) {
            continue;
        }

        program_header_count++;
    }

    uint64_t total_header_size = sizeof(elf_header_t) + (sizeof(elf_program_header_t) * program_header_count);
    uint64_t code_offset = PAGE_ROUND_UP(total_header_size);

    //--- Define the header and segments ---
    elf_header_t header = {
        .magic = ELF_MAGIC,
        .bitness = 2, //64-bit
        .endian = 1, //Little endian
        .header_version = 1,
        .abi = 0,
        .type = 2, //Dynamic
        .instruction_set = 0x3E, //x86-64
        .elf_version = 1,
        .entry_point = 0, //This is set later
        .program_header_offset = sizeof(elf_header_t), //Right after the header
        .section_header_offset = 0, //We don't have any sections
        .flags = 0, //No flags
        .header_size = sizeof(elf_header_t),
        .program_entry_size = sizeof(elf_program_header_t),
        .program_num_entries = program_header_count, //Text and data segments
        .section_entry_size = 0, //We don't have any sections
        .section_num_entries = 0,
        .section_name_index = 0,
    };

    elf_program_header_t program_headers[SEGMENT_TYPE_NUM];
    memset((void*) program_headers, 0, sizeof(program_headers));
    uint64_t current_offset = 0;

    executable_data_t exec_data;
    memset((void*) &exec_data, 0, sizeof(executable_data_t));

    for (int i = 0; i < SEGMENT_TYPE_NUM; i++) {
        elf_program_header_t* program_header = &program_headers[i];
        segment_data_t* segment = &segments[i];

        if (segment->opcode_num == 0) { //Don't add it if it's not defined
            continue;
        }

        program_header->type = 1; //Program
        if (segment->segment_type == TEXT || segment->segment_type == RODATA) {
            program_header->flags = 5; //Read and execute (absolutely no idea why linux requires rodata to be executable, but it does)
        } else if (segment->segment_type == DATA || segment->segment_type == BSS) {
            program_header->flags = 7; //Read, write, and execute (same here for data and bss, why on earth should they be executable?)
        } else {
            printf("Unknown segment type %d\n", segment->segment_type);
            return true;
        }
        program_header->offset = code_offset + current_offset;
        program_header->virtual_address = virtual_offset + current_offset;
        program_header->physical_address = program_header->virtual_address; //Usually irrelevant
        program_header->file_size = segment->opcode_num;
        program_header->memory_size = segment->opcode_num;
        program_header->alignment = 0x1000; //4KB pages, so align to 4KB

        exec_data.segment_offsets[i] = program_header->virtual_address;
        exec_data.segments[i] = &segments[i];

        current_offset += segment->opcode_num;
    }

    header.entry_point = exec_data.segment_offsets[TEXT];

    if (do_executable_replacements(exec_data, label_definitions, label_definition_num)) {
        return true; //Error
    }

    //--- Write the file ---
    fwrite(&header, sizeof(elf_header_t), 1, file);
    for (int i = 0; i < SEGMENT_TYPE_NUM; i++) {
        if (segments[i].opcode_num == 0) {
            continue;
        }
        fwrite(&program_headers[i], sizeof(elf_program_header_t), 1, file);
    }

    fseek(file, code_offset, SEEK_SET);

    //Write the segments
    for (int i = 0; i < SEGMENT_TYPE_NUM; i++) {
        if (segments[i].opcode_num == 0) {
            continue;
        }
        fwrite(segments[i].opcodes, segments[i].opcode_num, 1, file);
    }

    make_executable(file);

    return false;
}
