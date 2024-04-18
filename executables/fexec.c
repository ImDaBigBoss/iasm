#include <executables.h>
#include <executables/fexec.h>

define_exec_write(fexec) {
    uint64_t current_offset = 0;
    executable_data_t exec_data;
    for (int i = 0; i < SEGMENT_TYPE_NUM; i++) {
        int segment_size = segments[i].opcode_num;
        if (segment_size == 0) {
            exec_data.segment_offsets[i] = 0;
            exec_data.segments[i] = 0;
        } else {
            exec_data.segment_offsets[i] = current_offset;
            exec_data.segments[i] = &segments[i];
            current_offset += segment_size;
        }
    }

    if (do_executable_replacements(exec_data, label_definitions, label_definition_num)) {
        return true; //Error
    }

    fexec_header_t header;
    memset((void*) &header, 0, sizeof(fexec_header_t));
    header.magic = FEXEC_MAGIC;
    header.version = 1;
    header.entry = entry_offset;
    header.size = current_offset;

    fwrite(&header, sizeof(fexec_header_t), 1, file);
    for (int i = 0; i < SEGMENT_TYPE_NUM; i++) {
        fwrite(segments[i].opcodes, segments[i].opcode_num, 1, file);
    }

    return false;
}