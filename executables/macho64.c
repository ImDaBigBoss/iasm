#include <executables.h>
#include <executables/macho64.h>

//TEXT -> __TEXT __text
//DATA -> __DATA __data
//RODATA -> __TEXT __const
//BSS -> __DATA __bss

define_exec_write(macho64) {
    int text_segments = 0;
    int data_segments = 0;
    bool text_section_defined = false;
    bool rodata_section_defined = false;
    bool data_section_defined = false;
    bool bss_section_defined = false;

    executable_data_t exec_data;
    memset((void*) &exec_data, 0, sizeof(executable_data_t));

    for (int i = 0; i < SEGMENT_TYPE_NUM; i++) {
        int segment_size = segments[i].opcode_num;
        if (segment_size == 0) {
            continue;
        }

        if (segments[i].segment_type == TEXT) {
            text_segments++;
            text_section_defined = true;
        } else if (segments[i].segment_type == DATA) {
            data_segments++;
            data_section_defined = true;
        } else if (segments[i].segment_type == RODATA) {
            text_segments++;
            rodata_section_defined = true;
        } else if (segments[i].segment_type == BSS) {
            data_segments++;
            bss_section_defined = true;
        } else {
            printf("Segment type unknown to macho64: %d\n", segments[i].segment_type);
            return true;
        }

        //The rest of the exec data is set much later
        exec_data.segments[i] = &segments[i];
    }

    if (!text_section_defined) {
        printf("There is no code to execute!\n");
        return true;
    }

    //--- Define the header ---
    macho64_header_t header;
    memset((void*) &header, 0, sizeof(macho64_header_t));
    header.magic = 0xfeedfacf; // 64-bit
    header.cputype = 0x01000007; // x86_64
    header.cpusubtype = 0x80000003;
    header.filetype = 0x2; // EXECUTABLE
    header.ncmds = 2 + (text_segments == 0 ? 0 : 1) + (data_segments == 0 ? 0 : 1);
    header.flags = 0x1;
    // Size of commands will be set later

    macho64_load_command_t pagezero_command;
    pagezero_command.cmd = 0x19; // LC_SEGMENT_64
    pagezero_command.cmdsize = sizeof(macho64_load_command_t) + sizeof(macho64_segment_command_t);

    macho64_segment_command_t pagezero_command_data;
    memset((void*) &pagezero_command_data, 0, sizeof(macho64_segment_command_t));
    strcpy(pagezero_command_data.name, "__PAGEZERO");
    pagezero_command_data.vmsize = 0x100000000; //Make sure that this is a multiple of 0x1000

    
    //--- Build the sections ---
    macho64_load_command_t text_segment_command;
    memset(&text_segment_command, 0, sizeof(macho64_load_command_t));
    macho64_segment_command_t text_segment_command_data;
    memset(&text_segment_command_data, 0, sizeof(macho64_segment_command_t));
    if (text_segments != 0) {
        text_segment_command.cmd = 0x19; // LC_SEGMENT_64
        text_segment_command.cmdsize = sizeof(macho64_load_command_t) + sizeof(macho64_segment_command_t) + (sizeof(macho64_section64_t) * text_segments);

        strcpy(text_segment_command_data.name, "__TEXT");
        // File size and vmsize will be set later
        text_segment_command_data.maxprot = 0x7; // RWX
        text_segment_command_data.initprot = 0x5; // RX
        text_segment_command_data.nsects = text_segments;
    }

    macho64_load_command_t data_segment_command;
    memset(&data_segment_command, 0, sizeof(macho64_load_command_t));
    macho64_segment_command_t data_segment_command_data;
    memset(&data_segment_command_data, 0, sizeof(macho64_segment_command_t));
    if (data_segments != 0) {
        data_segment_command.cmd = 0x19; // LC_SEGMENT_64
        data_segment_command.cmdsize = sizeof(macho64_load_command_t) + sizeof(macho64_segment_command_t) + (sizeof(macho64_section64_t) * data_segments);

        strcpy(data_segment_command_data.name, "__DATA");
        // File size and vmsize will be set later
        data_segment_command_data.maxprot = 0x7; // RWX
        data_segment_command_data.initprot = 0x7; // RWX
        data_segment_command_data.nsects = data_segments;
    }


    macho64_section64_t text_text_section;
    memset(&text_text_section, 0, sizeof(macho64_section64_t));
    if (text_section_defined) {
        strcpy(text_text_section.sectname, "__text");
        strcpy(text_text_section.segname, "__TEXT");
        text_text_section.size = segments[TEXT].opcode_num;
    }
    macho64_section64_t text_rodata_section;
    memset(&text_rodata_section, 0, sizeof(macho64_section64_t));
    if (rodata_section_defined) {
        strcpy(text_rodata_section.sectname, "__const");
        strcpy(text_rodata_section.segname, "__TEXT");
        text_rodata_section.size = segments[RODATA].opcode_num;
    }
    macho64_section64_t data_data_section;
    memset(&data_data_section, 0, sizeof(macho64_section64_t));
    if (data_section_defined) {
        strcpy(data_data_section.sectname, "__data");
        strcpy(data_data_section.segname, "__DATA");
        data_data_section.size = segments[DATA].opcode_num;
    }
    macho64_section64_t data_bss_section;
    memset(&data_bss_section, 0, sizeof(macho64_section64_t));
    if (bss_section_defined) {
        strcpy(data_bss_section.sectname, "__bss");
        strcpy(data_bss_section.segname, "__DATA");
        data_bss_section.size = segments[BSS].opcode_num;
    }


    //--- Define an entry point ---
    macho64_load_command_t unix_thread_command;
    unix_thread_command.cmd = 0x5; // LC_UNIXTHREAD
    unix_thread_command.cmdsize = sizeof(macho64_load_command_t) + sizeof(thread_state64_command_t);

    thread_state64_command_t unix_thread_command_data;
    memset((void*) &unix_thread_command_data, 0, sizeof(thread_state64_command_t));
    unix_thread_command_data.flavor = 0x4; // x86_64_THREAD_STATE
    unix_thread_command_data.count = 42;
    //__text address will be set later


    //--- Set the dynamic values ---
    header.sizeofcmds = pagezero_command.cmdsize + text_segment_command.cmdsize + data_segment_command.cmdsize + unix_thread_command.cmdsize;
    if (text_segments != 0) {
        text_segment_command_data.fileoff = 0;
        text_segment_command_data.filesize = PAGE_ROUND_UP(sizeof(macho64_header_t) + header.sizeofcmds + text_text_section.size + text_rodata_section.size);
        text_segment_command_data.vmsize = text_segment_command_data.filesize;
        text_segment_command_data.vmaddr = pagezero_command_data.vmsize;

        if (text_section_defined) {
            text_text_section.offset = (text_segment_command_data.fileoff + text_segment_command_data.filesize) - (text_rodata_section.size + text_text_section.size);
            text_text_section.addr = (text_segment_command_data.vmaddr + text_segment_command_data.vmsize) - (text_rodata_section.size + text_text_section.size);
            exec_data.segment_offsets[TEXT] = text_text_section.addr;
        }
        if (rodata_section_defined) {
            text_rodata_section.offset = (text_segment_command_data.fileoff + text_segment_command_data.filesize) - text_rodata_section.size;
            text_rodata_section.addr = (text_segment_command_data.vmaddr + text_segment_command_data.vmsize) - text_rodata_section.size;
            exec_data.segment_offsets[RODATA] = text_rodata_section.addr;
        }
    }
    if (data_segments != 0) {
        if (text_segments == 0) {
            data_segment_command_data.fileoff = 0;
            data_segment_command_data.filesize = sizeof(macho64_header_t) + header.sizeofcmds;
        } else {
            data_segment_command_data.fileoff = text_segment_command_data.vmsize;
            data_segment_command_data.filesize = 0;
        }
        data_segment_command_data.filesize += data_data_section.size + data_bss_section.size;
        data_segment_command_data.filesize = PAGE_ROUND_UP(data_segment_command_data.filesize);
        data_segment_command_data.vmsize = data_segment_command_data.filesize;
        data_segment_command_data.vmaddr = pagezero_command_data.vmsize + text_segment_command_data.vmsize;

        if (data_section_defined) {
            data_data_section.offset = (data_segment_command_data.fileoff + data_segment_command_data.filesize) - (data_bss_section.size + data_data_section.size);
            data_data_section.addr = (data_segment_command_data.vmaddr + data_segment_command_data.vmsize) - (data_bss_section.size + data_data_section.size);
            exec_data.segment_offsets[DATA] = data_data_section.addr;
        }
        if (bss_section_defined) {
            data_bss_section.offset = (data_segment_command_data.fileoff + data_segment_command_data.filesize) - data_bss_section.size;
            data_bss_section.addr = (data_segment_command_data.vmaddr + data_segment_command_data.vmsize) - data_bss_section.size;
            exec_data.segment_offsets[BSS] = data_bss_section.addr;
        }
    }
    unix_thread_command_data.rip = text_text_section.addr + entry_offset;

    if (do_executable_replacements(exec_data, label_definitions, label_definition_num)) {
        return true; //Error
    }

    //--- Write the file ---
    fwrite(&header, sizeof(macho64_header_t), 1, file);
    fwrite(&pagezero_command, sizeof(macho64_load_command_t), 1, file);
    fwrite(&pagezero_command_data, sizeof(macho64_segment_command_t), 1, file);
    if (text_segments != 0) {
        fwrite(&text_segment_command, sizeof(macho64_load_command_t), 1, file);
        fwrite(&text_segment_command_data, sizeof(macho64_segment_command_t), 1, file);
        if (text_section_defined) {
            fwrite(&text_text_section, sizeof(macho64_section64_t), 1, file);
        }
        if (rodata_section_defined) {
            fwrite(&text_rodata_section, sizeof(macho64_section64_t), 1, file);
        }
    }
    if (data_segments != 0) {
        fwrite(&data_segment_command, sizeof(macho64_load_command_t), 1, file);
        fwrite(&data_segment_command_data, sizeof(macho64_segment_command_t), 1, file);
        if (data_section_defined) {
            fwrite(&data_data_section, sizeof(macho64_section64_t), 1, file);
        }
        if (bss_section_defined) {
            fwrite(&data_bss_section, sizeof(macho64_section64_t), 1, file);
        }
    }
    fwrite(&unix_thread_command, sizeof(macho64_load_command_t), 1, file);
    fwrite(&unix_thread_command_data, sizeof(thread_state64_command_t), 1, file);

    if (text_section_defined) {
        fseek(file, text_text_section.offset, SEEK_SET);
        fwrite(segments[TEXT].opcodes, segments[TEXT].opcode_num, 1, file);
    }
    if (rodata_section_defined) {
        fseek(file, text_rodata_section.offset, SEEK_SET);
        fwrite(segments[RODATA].opcodes, segments[RODATA].opcode_num, 1, file);
    }
    if (data_section_defined) {
        fseek(file, data_data_section.offset, SEEK_SET);
        fwrite(segments[DATA].opcodes, segments[DATA].opcode_num, 1, file);
    }
    if (bss_section_defined) {
        fseek(file, data_bss_section.offset, SEEK_SET);
        fwrite(segments[BSS].opcodes, segments[BSS].opcode_num, 1, file);
    }

    // Make sure the file is at least 0x1000 bytes long
    long current_file_size = ftell(file);
    if (current_file_size < 0x1000) {
        uint8_t padding[0x1000];
        memset((void*) padding, 0, 0x1000);
        fwrite(padding, 0x1000 - current_file_size, 1, file);
    }

    make_executable(file);

    return false;
}
