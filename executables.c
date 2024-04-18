#include <executables.h>
#include <executables/fexec.h>
#include <executables/macho64.h>
#include <executables/elf.h>

#include <stdlib.h>
#include <sys/stat.h>

executable_format_t get_exec_type(char* name) {
    if (strcmp(name, "bin") == 0) {
        return BIN_FORMAT;
    } else if (strcmp(name, "fexec") == 0) {
        return FEXEC_FORMAT;
    } else if (strcmp(name, "macho64") == 0) {
        return MACHO64_FORMAT;
    } else if (strcmp(name, "elf") == 0) {
        return ELF_FORMAT;
    } else {
        return -1;
    }
}

bool write_exec(executable_format_t format, FILE* file, uint64_t entry_offset, segment_data_t* segments, label_t* label_definitions, int label_definition_num) {
    switch (format) {
        case BIN_FORMAT:
            return call_exec_write(bin);
        case FEXEC_FORMAT:
            return call_exec_write(fexec);
        case MACHO64_FORMAT:
            return call_exec_write(macho64);
        case ELF_FORMAT:
            return call_exec_write(elf);
        default:
            printf("Unknown executable type!\n");
            return true;
    }
}

void make_executable(FILE* file) {
    //Set the executable bit wherever the read bit is set

    struct stat st;
    fstat(fileno(file), &st);

    if (st.st_mode & S_IRUSR) {
        st.st_mode |= S_IXUSR;
    }
    if (st.st_mode & S_IRGRP) {
        st.st_mode |= S_IXGRP;
    }
    if (st.st_mode & S_IROTH) {
        st.st_mode |= S_IXOTH;
    }

    fchmod(fileno(file), st.st_mode);
}

bool do_executable_replacements(executable_data_t exec_data, label_t* label_definitions, int label_definition_num) {
    for (int i = 0; i < SEGMENT_TYPE_NUM; i++) {
        segment_data_t* segment = exec_data.segments[i];
        uint64_t segment_offset = exec_data.segment_offsets[i];

        if (segment == 0) {
            continue;
        }

        for (int i = 0; i < segment->replacement_num; i++) {
            replacement_t* replacement = &segment->replacements[i];
            uint8_t* code_address = &segment->opcodes[replacement->address];

            if (replacement->type == ABSOULTE_LABEL_REFERENCE || replacement->type == RELATIVE_LABEL_REFERENCE) {
                char* label_name = (char*) replacement->data;

                uint64_t address = 0;
                bool found = false;
                for (int j = 0; j < label_definition_num; j++) {
                    label_t* label = &label_definitions[j];

                    if (strcmp(label->name, label_name) == 0) {
                        address = label->local_address + exec_data.segment_offsets[label->segment];
                        found = true;
                        break;
                    }
                }

                if (found) {
                    if (replacement->type == RELATIVE_LABEL_REFERENCE) {
                        uint64_t replacement_absolute_address = segment_offset + replacement->address;
                        int32_t relative_address = (int32_t) (address - (replacement_absolute_address + sizeof(int32_t)));
                        memcpy(code_address, &relative_address, sizeof(int32_t));
                    } else if (replacement->type == ABSOULTE_LABEL_REFERENCE) {
                        //Replace the opcode with the address relative to the start of the code
                        memcpy(code_address, &address, sizeof(uint64_t));
                    } else {
                        printf("Unknown replacement type %d\n", replacement->type);
                        return true;
                    }
                } else {
                    printf("Reference to undefined label %s!\n", label_name);
                    return true;
                }

                free(label_name);
            } else {
                printf("Unknown replacement type %d\n", replacement->type);
                return true;
            }
        }
    }

    return false;
}

define_exec_write(bin) {
    if (entry_offset != 0) {
        printf("Entry offset not supported for bin format!\n");
        return true;
    }

    uint64_t current_offset = 0;
    executable_data_t exec_data;
    for (int i = 0; i < SEGMENT_TYPE_NUM; i++) {
        exec_data.segment_offsets[i] = current_offset;
        exec_data.segments[i] = &segments[i];
        current_offset += segments[i].opcode_num;
    }
    
    if (do_executable_replacements(exec_data, label_definitions, label_definition_num)) {
        return true; //Error
    }

    for (int i = 0; i < SEGMENT_TYPE_NUM; i++) {
        fwrite(segments[i].opcodes, segments[i].opcode_num, 1, file);
    }

    return false;
}
