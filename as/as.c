#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <vcpu16.h>

struct label {
    unsigned short pc;
    char identifier[64];
};

static int ext_stricmp(const char *a, const char *b)
{
    while(a[0] && tolower(a[0]) == tolower(b[0])) { a++; b++; }
    return a[0] - b[0];
}

static long ext_strtol(const char *a)
{
    if(a[0] == '0' && tolower(a[1]) == 'x')
        return strtol(a + 2, NULL, 16);
    if(a[0] == '0' && tolower(a[1]) == 'b')
        return strtol(a + 2, NULL, 2);
    return strtol(a, NULL, 10);
}

static unsigned short get_opcode(const char *id)
{
    #define _opcode_x(x) if(!ext_stricmp(id, #x)) return VCPU_OPCODE_##x

    _opcode_x(NOP);
    _opcode_x(HLT);
    _opcode_x(PTS);
    _opcode_x(PFS);
    _opcode_x(CAL);
    _opcode_x(RET);
    _opcode_x(IOR);
    _opcode_x(IOW);
    _opcode_x(MRD);
    _opcode_x(MWR);
    _opcode_x(CLI);
    _opcode_x(STI);
    _opcode_x(INT);
    _opcode_x(RFI);
    _opcode_x(CPI);
    _opcode_x(IEQ);
    _opcode_x(INE);
    _opcode_x(IGT);
    _opcode_x(IGE);
    _opcode_x(ILT);
    _opcode_x(ILE);
    _opcode_x(MOV);
    _opcode_x(ADD);
    _opcode_x(SUB);
    _opcode_x(MUL);
    _opcode_x(DIV);
    _opcode_x(MOD);
    _opcode_x(SHL);
    _opcode_x(SHR);
    _opcode_x(AND);
    _opcode_x(BOR);
    _opcode_x(XOR);
    _opcode_x(NOT);
    _opcode_x(INC);
    _opcode_x(DEC);
    return USHRT_MAX;

    #undef _opcode_x
}

static unsigned short get_register(const char *id)
{
    #define _register_x(x) if(!ext_stricmp(id, #x)) return VCPU_REGISTER_##x

    _register_x(R0);
    _register_x(R1);
    _register_x(R2);
    _register_x(R3);
    _register_x(R4);
    _register_x(R5);
    _register_x(R6);
    _register_x(R7);
    _register_x(R8);
    _register_x(R9);
    _register_x(RI);
    _register_x(RJ);
    _register_x(IA);
    _register_x(OF);
    _register_x(SP);
    _register_x(PC);
    return USHRT_MAX;

    #undef _register_x
}

static char *skip_comments(char *line)
{
    size_t n, i;

    if(line) {
        n = strlen(line);
        for(i = 0; i < n; i++) {
            if(line[i] == '#') {
                line[i] = 0;
                break;
            }
        }

        return line;
    }

    return NULL;
}

static int is_empty_or_whitespace(const char *s)
{
    size_t i;
    size_t n = strlen(s);
    if(n) {
        for(i = 0; i < n; i++) {
            if(isspace(s[i]) || s[i] == '\n' || s[i] == '\r')
                continue;
            return 0;
        }
    }

    return 1;
}

static const struct label *find_label(const char *name, const struct label *labels, size_t num_labels)
{
    size_t i;
    for(i = 0; i < num_labels; i++) {
        if(strcmp(labels[i].identifier, name))
            continue;
        return labels + i;
    }
    return NULL;
}

static char print_buffer[4096] = { 0 };
static const char *argv_0 = NULL;
static const char *infile_name = NULL;
static size_t line_no = 0;

#define _ansi_reset     "\033[0m"
#define _ansi_warning   "\033[1;35m"
#define _ansi_error     "\033[1;31m"

static void lprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(print_buffer, sizeof(print_buffer), fmt, ap);
    fprintf(stderr, "%s\n", print_buffer);
    va_end(ap);
}

static void warning(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    vsnprintf(print_buffer, sizeof(print_buffer), fmt, va);
    if(infile_name)
        fprintf(stderr, "%s:%zu: " _ansi_warning "warning: " _ansi_reset "%s\n", infile_name, line_no, print_buffer);
    else
        fprintf(stderr, "%s: " _ansi_warning "warning: " _ansi_reset "%s\n", argv_0, print_buffer);
    va_end(va);
}

static void error(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    vsnprintf(print_buffer, sizeof(print_buffer), fmt, va);
    if(infile_name)
        fprintf(stderr, "%s:%zu: " _ansi_error "error: " _ansi_reset "%s\n", infile_name, line_no, print_buffer);
    else
        fprintf(stderr, "%s: " _ansi_error "fatal: " _ansi_reset "%s\n", argv_0, print_buffer);
    va_end(va);
    exit(1);
}

int main(int argc, char **argv)
{
    FILE *infile = NULL;
    FILE *outfile = NULL;
    int r, j;
    int aout = 0;
    size_t num_lines;
    struct label *labels;
    int nc;
    char prefix;
    unsigned short virt_pc;
    char line[128];
    char *line_p;
    char *label_p, *scratch;
    char identifier[64];
    char *line_p_lab;
    unsigned short word, opcode, preg;
    unsigned short imms[2];
    size_t num_imms;

    argv_0 = argv[0];

    while((r = getopt(argc, argv, "o:vh")) != -1) {
        switch(r) {
            case 'o':
                outfile = fopen(optarg, "wb");
                aout = 0;
                break;
            case 'v':
                lprintf("%s (VCPU-16 AS) version 0.0.x", argv_0);
                return 0;
            default:
                lprintf("Usage: %s [-o <outfile>] [-h] <infile>", argv_0);
                lprintf("Options:");
                lprintf("   -o <outfile>    : Set the output file");
                lprintf("   -v              : Print version and exit");
                lprintf("   -h              : Print this message and exit");
                lprintf("   <infile>        : Input source file");
                return (r == 'h') ? 0 : 1;
        }
    }

    if(optind >= argc)
        error("no input files");
    
    infile_name = argv[optind];
    line_no = 0;

    infile = fopen(infile_name, "rb");
    if(!infile)
        error("%s", strerror(errno));


    if(aout && !outfile) {
        if(!(outfile = fopen("a.out", "wb")))
            error("unable to open a.out for writing");
    }

    num_lines = 0;
    while(!feof(infile)) {
        if(fgetc(infile) == '\n')
            num_lines++;
    }

    labels = malloc(sizeof(struct label) * num_lines);
    assert(("Out of memory!", labels));
    memset(labels, 0, sizeof(struct label) * num_lines);

    line_no = 0;
    nc = 0;
    prefix = 0;
    virt_pc = 0x0000;

    fseek(infile, 0, SEEK_SET);
    while((line_p = skip_comments(fgets(line, sizeof(line), infile)))) {
        if(is_empty_or_whitespace(line_p)) {
            line_no++;
            continue;
        }

        label_p = strchr(line_p, ':');
        if(label_p) {
            scratch = line_p;
            label_p[0] = 0;
            line_p = label_p + 1;
            label_p = scratch;
            if(get_opcode(label_p) == USHRT_MAX && get_register(label_p) == USHRT_MAX) {
                labels[line_no].pc = virt_pc;
                strncpy(labels[line_no].identifier, label_p, sizeof(labels[line_no].identifier));
            }
        }

        if(is_empty_or_whitespace(line_p)) {
            line_no++;
            continue;
        }

        if(sscanf(line_p, " %63s%n", identifier, &nc) == 1) {
            line_p += nc;
            virt_pc++;

            for(j = 0; j < 2; j++) {
                if(sscanf(line_p, " %c%n", &prefix, &nc) != 1)
                    break;
                line_p += nc;

                if(sscanf(line_p, " %63[^, \t\n]%n", identifier, &nc) != 1)
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
    while((line_p = skip_comments(fgets(line, sizeof(line), infile)))) {
        if(is_empty_or_whitespace(line_p)) {
            line_no++;
            continue;
        }

        line_p_lab = strchr(line_p, ':');
        if(line_p_lab)
            line_p = line_p_lab + 1;

        if(sscanf(line_p, " %65s%n", identifier, &nc) != 1) {
            line_no++;
            continue;
        }

        line_p += nc;

        word = 0;
        opcode = get_opcode(identifier);
        if(opcode == USHRT_MAX)
            error("unknown mnemonic: %s", identifier);

        word |= (opcode & 0x3F) << 10;
        preg = USHRT_MAX;
        num_imms = 0;

        do {
            unsigned short imm = 0;
            const struct label *label;

            if(sscanf(line_p, " %c%n", &prefix, &nc) != 1)
                break;
            line_p += nc;

            if(sscanf(line_p, " %63[^, \t\n]%n", identifier, &nc) != 1)
                break;
            line_p += nc;

            switch(prefix) {
                case '$':
                    if(isalpha(identifier[0])) {
                        if(!(label = find_label(identifier, labels, num_lines)))
                            error("unknown label: %s", identifier);
                        imm = label->pc;
                    }
                    else if(identifier[0] == '\'' && identifier[2] == '\'')
                        imm = identifier[1];
                    else
                        imm = ext_strtol(identifier);
                    word |= 1 << 9;
                    imms[num_imms++] = imm;
                    break;
                case '%':
                    preg = get_register(identifier);
                    if(preg == USHRT_MAX)
                        error("unknown register: %s", identifier);
                    word |= (preg & 0x0F) << 5;
                    break;
                default:
                    error("unknown operand prefix: %c", prefix);
                    break;
            }

            if(sscanf(line_p, " %c%n", &prefix, &nc) != 1 && prefix != ',')
                break;
            line_p += nc;

            if(sscanf(line_p, " %c%n", &prefix, &nc) != 1)
                break;
            line_p += nc;

            if(sscanf(line_p, " %63[^, \t\n]%n", identifier, &nc) != 1)
                break;
            line_p += nc;

            switch(prefix) {
                case '$':
                    if(isalpha(identifier[0])) {
                        if(!(label = find_label(identifier, labels, num_lines)))
                            error("unknown label: %s", identifier);
                        imm = label->pc;
                    }
                    else if(identifier[0] == '\'' && identifier[2] == '\'')
                        imm = identifier[1];
                    else
                        imm = ext_strtol(identifier);
                    word |= 1 << 4;
                    imms[num_imms++] = imm;
                    break;
                case '%':
                    preg = get_register(identifier);
                    if(preg == USHRT_MAX)
                        error("unknown register: %s", identifier);
                    word |= preg & 0x0F;
                    break;
                default:
                    error("unknown operand prefix: %c", prefix);
                    break;
            }
        } while(0);

        word = vcpu_host_to_be16(word);
        imms[0] = vcpu_host_to_be16(imms[0]);
        imms[1] = vcpu_host_to_be16(imms[1]);

        fwrite(&word, sizeof(unsigned short), 1, outfile);
        fwrite(imms, sizeof(unsigned short), num_imms, outfile);

        line_no++;
    }

    fclose(infile);
    fclose(outfile);
    return 0;
}
