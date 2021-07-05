// Copyright (c) 2021, Kirill GPRB
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <V16.h>

typedef struct ASM_label {
    uint16_t pc;
    char id[65];
} ASM_label_t;

static inline int ASM_stricmp(const char *a, const char *b)
{
    while(a[0] && tolower(a[0]) == tolower(b[0])) { a++; b++; }
    return a[0] - b[0];
}

static inline uint16_t ASM_atow(const char *a)
{
    if(a[0] == '0' && tolower(a[1]) == 'x')
        return (uint16_t)strtol(a + 2, NULL, 16);
    if(a[0] == '0' && tolower(a[1]) == 'b')
        return (uint16_t)strtol(a + 2, NULL, 2);
    return (uint16_t)atoi(a);
}

static inline uint16_t ASM_opcode(const char *id)
{
    #define ASM_OPCODE_X(x) if(!ASM_stricmp(id, #x)) return V16_OPCODE_##x

    ASM_OPCODE_X(NOP);
    ASM_OPCODE_X(HLT);
    ASM_OPCODE_X(PTS);
    ASM_OPCODE_X(PFS);
    ASM_OPCODE_X(SCL);
    ASM_OPCODE_X(SRT);
    ASM_OPCODE_X(IOR);
    ASM_OPCODE_X(IOW);
    ASM_OPCODE_X(MRD);
    ASM_OPCODE_X(MWR);

    ASM_OPCODE_X(MOV);
    ASM_OPCODE_X(ADD);
    ASM_OPCODE_X(SUB);
    ASM_OPCODE_X(MUL);
    ASM_OPCODE_X(DIV);
    ASM_OPCODE_X(MOD);
    ASM_OPCODE_X(SHL);
    ASM_OPCODE_X(SHR);
    ASM_OPCODE_X(AND);
    ASM_OPCODE_X(BOR);
    ASM_OPCODE_X(XOR);
    ASM_OPCODE_X(NOT);
    ASM_OPCODE_X(INC);
    ASM_OPCODE_X(DEC);

    ASM_OPCODE_X(IEQ);
    ASM_OPCODE_X(INE);
    ASM_OPCODE_X(IGT);
    ASM_OPCODE_X(IGE);
    ASM_OPCODE_X(ILT);
    ASM_OPCODE_X(ILE);

    #undef ASM_OPCODE_X
    return UINT16_MAX;
}

static inline uint16_t ASM_register(const char *id)
{
    #define ASM_REGISTER_X(x) if(!ASM_stricmp(id, #x)) return V16_REGISTER_##x

    ASM_REGISTER_X(R0);
    ASM_REGISTER_X(R1);
    ASM_REGISTER_X(R2);
    ASM_REGISTER_X(R3);
    ASM_REGISTER_X(R4);
    ASM_REGISTER_X(R5);
    ASM_REGISTER_X(R6);
    ASM_REGISTER_X(R7);
    ASM_REGISTER_X(R8);
    ASM_REGISTER_X(R9);
    ASM_REGISTER_X(RI);
    ASM_REGISTER_X(RJ);
    ASM_REGISTER_X(IA);
    ASM_REGISTER_X(OF);
    ASM_REGISTER_X(SP);
    ASM_REGISTER_X(PC);

    #undef ASM_REGISTER_X
    return UINT16_MAX;
}

static inline char *ASM_skipComments(char *line)
{
    if(line) {
        size_t len = strlen(line);
        for(size_t i = 0; i < len; i++) {
            if(line[i] == '#') {
                line[i] = 0;
                break;
            }
        }

        return line;
    }

    return NULL;
}

static inline int ASM_isEmptyOrWhitespace(const char *s)
{
    size_t len = strlen(s);
    if(len) {
        for(size_t i = 0; i < len; i++) {
            if(isspace(s[i]) || s[i] == '\n' || s[i] == '\r')
                continue;
            return 0;
        }
    }

    return 1;
}

static inline const ASM_label_t *ASM_findLabel(const char *name, const ASM_label_t *labels, size_t num_labels)
{
    for(size_t i = 0; i < num_labels; i++) {
        if(strcmp(labels[i].id, name))
            continue;
        return labels + i;
    }
    return NULL;
}

static void lprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

int main(int argc, char **argv)
{
    FILE *infile = NULL;
    FILE *outfile = NULL;

    int r;
    while((r = getopt(argc, argv, "o:h")) != EOF) {
        switch(r) {
            case 'o':
                outfile = ASM_stricmp(optarg, "stdout") ? fopen(optarg, "wb") : stdout;
                break;
            default:
                lprintf("Usage: %s [-o <outfile|stdout>] [-h] <infile>\n", argv[0]);
                return 1;
        }
    }

    if(!infile) {
        if(optind >= argc) {
            lprintf("%s: fatal: infile is null!\n", argv[0]);
            return 1;
        }

        infile = fopen(argv[optind], "rb");
        if(!infile) {
            lprintf("%s: fatal: infile is null!\n", argv[0]);
            return 1;
        }
    }

    if(!outfile) {
        lprintf("%s: fatal: outfile is null!\n", argv[0]);
        return 1;
    }

    size_t num_lines = 0;
    while(!feof(infile)) {
        if(fgetc(infile) == '\n')
            num_lines++;
    }

    ASM_label_t *labels = malloc(sizeof(ASM_label_t) * num_lines);
    memset(labels, 0, sizeof(ASM_label_t) * num_lines);

    size_t line_no = 0;
    char line[128] = { 0 };
    char *line_p;
    int empty = 0, nc = 0;

    char identifier[65] = { 0 };
    char prefix = 0;

    uint16_t virt_pc = 0x0000;
    fseek(infile, 0, SEEK_SET);
    while(line_p = ASM_skipComments(fgets(line, sizeof(line), infile))) {
        if(ASM_isEmptyOrWhitespace(line_p)) {
            line_no++;
            continue;
        }

        char *label_p = strchr(line_p, ':');
        if(label_p) {
            char *tmp = line_p;
            label_p[0] = 0;
            line_p = label_p + 1;
            label_p = tmp;
            if(ASM_opcode(label_p) == UINT16_MAX && ASM_register(label_p) == UINT16_MAX) {
                labels[line_no].pc = virt_pc;
                strncpy(labels[line_no].id, label_p, sizeof(labels[line_no].id));
            }
        }

        if(ASM_isEmptyOrWhitespace(line_p)) {
            line_no++;
            continue;
        }

        if(sscanf(line_p, " %64s%n", identifier, &nc) == 1) {
            line_p += nc;
            virt_pc++;

            for(int j = 0; j < 2; j++) {
                if(sscanf(line_p, " %c%n", &prefix, &nc) != 1)
                    break;
                line_p += nc;

                if(sscanf(line_p, " %64[^, \t\n]%n", identifier, &nc) != 1)
                    break;
                line_p += nc;

                if(prefix == '$')
                    virt_pc++;
                
                if(sscanf(line_p, " %c%n", &prefix, &nc) != 1 && prefix != ',')
                    break;
                line_p += nc;
            }
        }

        line_no++;
    }

    line_no = 0;
    nc = 0;

    fseek(infile, 0, SEEK_SET);
    while(line_p = ASM_skipComments(fgets(line, sizeof(line), infile))) {
        if(ASM_isEmptyOrWhitespace(line_p)) {
            line_no++;
            continue;
        }

        char *line_p_lab = strchr(line_p, ':');
        if(line_p_lab)
            line_p = line_p_lab + 1;

        if(sscanf(line_p, " %65s%n", identifier, &nc) != 1) {
            line_no++;
            continue;
        }

        line_p += nc;

        uint16_t word = 0;

        uint16_t opcode = ASM_opcode(identifier);
        if(opcode == UINT16_MAX) {
            lprintf("%s: error[%zu]: unknown instruction '%s'\n", argv[0], line_no, identifier);
            goto asm_quit;
        }

        word |= (opcode & 0x3F) << 10;
        
        uint16_t imms[2] = { 0 };
        uint16_t preg = UINT16_MAX;
        size_t num_imms = 0;

        do {
            if(sscanf(line_p, " %c%n", &prefix, &nc) != 1)
                break;
            line_p += nc;

            if(sscanf(line_p, " %64[^, \t\n]%n", identifier, &nc) != 1)
                break;
            line_p += nc;

            switch(prefix) {
                case '$':
                    uint16_t imm = 0;
                    const ASM_label_t *label;
                    if(isalpha(identifier[0])) {
                        if(!(label = ASM_findLabel(identifier, labels, num_lines))) {
                            lprintf("%s: error[%zu]: unknown label '%s'\n", argv[0], line_no, identifier);
                            goto asm_quit;
                        }
                        imm = label->pc;
                    }
                    else imm = ASM_atow(identifier);
                    word |= 1 << 9;
                    imms[num_imms++] = imm;
                    break;
                case '%':
                    preg = ASM_register(identifier);
                    if(preg == UINT16_MAX) {
                        lprintf("%s: error[%zu]: unknown register '%s'\n", argv[0], line_no, identifier);
                        goto asm_quit;
                    }
                    word |= (preg & 0x0F) << 5;
                    break;
                default:
                    lprintf("%s: warning[%zu]: unknown prefix '%c'.\n", argv[0], line_no, prefix);
                    break;
            }

            if(sscanf(line_p, " %c%n", &prefix, &nc) != 1 && prefix != ',')
                break;
            line_p += nc;

            if(sscanf(line_p, " %c%n", &prefix, &nc) != 1)
                break;
            line_p += nc;

            if(sscanf(line_p, " %64[^, \t\n]%n", identifier, &nc) != 1)
                break;
            line_p += nc;

            switch(prefix) {
                case '$':
                    uint16_t imm = 0;
                    const ASM_label_t *label;
                    if(isalpha(identifier[0])) {
                        if(!(label = ASM_findLabel(identifier, labels, num_lines))) {
                            lprintf("%s: error[%zu]: unknown label '%s'\n", argv[0], line_no, identifier);
                            goto asm_quit;
                        }
                        imm = label->pc;
                    }
                    else imm = ASM_atow(identifier);
                    word |= 1 << 4;
                    imms[num_imms++] = imm;
                    break;
                case '%':
                    preg = ASM_register(identifier);
                    if(preg == UINT16_MAX) {
                        lprintf("%s: error[%zu]: unknown register '%s'\n", argv[0], line_no, identifier);
                        goto asm_quit;
                    }
                    word |= preg & 0x0F;
                    break;
                default:
                    lprintf("%s: error[%zu]: unknown prefix '%c'.\n", argv[0], line_no, prefix);
                    goto asm_quit;
            }
        } while(0);

        word = V16_hostToBE16(word);
        imms[0] = V16_hostToBE16(imms[0]);
        imms[1] = V16_hostToBE16(imms[1]);

        fwrite(&word, sizeof(uint16_t), 1, outfile);
        fwrite(imms, sizeof(uint16_t), num_imms, outfile);

        line_no++;
    }

asm_quit:
    fclose(infile);
    fclose(outfile);
    return 0;
}
