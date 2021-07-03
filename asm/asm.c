#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <V16.h>

static int ASM_stricmp(const char *a, const char *b)
{
    while(a[0] && tolower(a[0]) == tolower(b[0])) { a++; b++; }
    return a[0] - b[0];
}

static uint16_t ASM_atow(const char *a)
{
    if(a[0] == '0' && tolower(a[1]) == 'x')
        return (uint16_t)strtol(a + 2, NULL, 16);
    if(a[0] == '0' && tolower(a[1]) == 'b')
        return (uint16_t)strtol(a + 2, NULL, 2);
    return (uint16_t)atoi(a);
}

static uint16_t ASM_opcode(const char *id)
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
//  ASM_OPCODE_X(CID);

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

    ASM_OPCODE_X(IEQ);
    ASM_OPCODE_X(INE);
    ASM_OPCODE_X(IGT);
    ASM_OPCODE_X(IGE);
    ASM_OPCODE_X(ILT);
    ASM_OPCODE_X(ILE);

    #undef ASM_OPCODE_X
    return UINT16_MAX;
}

static uint16_t ASM_register(const char *id)
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
    while((r = getopt(argc, argv, "o:sh")) != EOF) {
        switch(r) {
            case 'o':
                outfile = ASM_stricmp(optarg, "stdout") ? fopen(optarg, "wb") : stdout;
                break;
            case 's':
                infile = stdin;
                break;
            default:
                lprintf("Usage: V16ASM [-o <outfile|stdout>] [-h] [-s] <infile>\n");
                return 1;
        }
    }

    if(!infile) {
        if(optind >= argc) {
            lprintf("V16ASM: fatal: infile is null!\n");
            return 1;
        }

        infile = fopen(argv[optind], "rb");
        if(!infile) {
            lprintf("V16ASM: fatal: infile is null!\n");
            return 1;
        }
    }

    if(!outfile) {
        lprintf("V16ASM: fatal: outfile is null!\n");
        return 1;
    }

    int nline = 0;
    char line[128] = { 0 };
    char *line_p = NULL;
    while(line_p = fgets(line, sizeof(line), infile)) {
        V16_instruction_t instr = { .word = 0 };
        int nc = 0;

        size_t len = strlen(line_p);
        for(size_t i = 0; i < len; i++) {
            if(line_p[i] == '#') {
                line_p[i] = 0;
                break;
            }
        }

        int empty = 1;
        len = strlen(line_p);
        for(size_t i = 0; i < len; i++) {
            if(!isspace(line_p[i])) {
                empty = 0;
                break;
            }
        }

        if(empty)
            continue;

        char mnemonic[17] = { 0 };
        sscanf(line_p, " %16s%n", mnemonic, &nc);
        line_p += nc;

        uint16_t opcode = ASM_opcode(mnemonic);
        if(opcode == UINT16_MAX) {
            lprintf("V16ASM: error[%d]: unknown mnemonic %s.", nline, mnemonic);
            goto asm_quit;
        }

        instr.i.opcode = opcode;

        char prefix = '$';
        char identifier[65] = { 0 };
        uint16_t imms[2] = { 0 };
        uint16_t preg = UINT16_MAX;
        int num_imms = 0;

        do {
            if(sscanf(line_p, " %c%n", &prefix, &nc) != 1)
                break;
            line_p += nc;
            
            if(sscanf(line_p, " %64[^, \t]%n", identifier, &nc) != 1)
                break;
            line_p += nc;

            switch(prefix) {
                case '$':
                    instr.i.a_imm = 1;
                    imms[num_imms++] = ASM_atow(identifier);
                    break;
                case '%':
                    preg = ASM_register(identifier);
                    if(preg == UINT16_MAX) {
                        lprintf("V16ASM: error[%d]: unknown register '%s'\n", nline, identifier);
                        goto asm_quit;
                    }
                    instr.i.a_reg = preg;
                    break;
                default:
                    lprintf("V16ASM: warning: unknown prefix '%c'.\n", prefix);
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
                    instr.i.b_imm = 1;
                    imms[num_imms++] = ASM_atow(identifier);
                    break;
                case '%':
                    preg = ASM_register(identifier);
                    if(preg == UINT16_MAX) {
                        lprintf("V16ASM: error[%d]: unknown register '%s'\n", nline, identifier);
                        goto asm_quit;
                    }
                    instr.i.b_reg = preg;
                    break;
                default:
                    lprintf("V16ASM: error[%d]: unknown prefix '%c'.\n", nline, prefix);
                    goto asm_quit;
            }
        } while(0);

        instr.word = V16_hostToBE16(instr.word);
        imms[0] = V16_hostToBE16(imms[0]);
        imms[1] = V16_hostToBE16(imms[1]);

        fwrite(&instr.word, sizeof(uint16_t), 1, outfile);
        fwrite(imms, sizeof(uint16_t), num_imms, outfile);

        nline++;
    }

asm_quit:
    fclose(infile);
    fclose(outfile);
    return 0;
}
