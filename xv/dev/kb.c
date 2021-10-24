#include <ncurses.h>
#include "dev/kb.h"

#define KB_BUFFER_SIZE 16

#define EXT_BACKSPACE_1 127
#define EXT_BACKSPACE_2 '\b'
#define EXT_BACKSPACE_3 KEY_BACKSPACE

#define EXT_RETURN_1 '\n'
#define EXT_RETURN_2 KEY_ENTER

#define EXT_TAB_1 '\t'
#define EXT_TAB_2 KEY_STAB

static unsigned short buffer[KB_BUFFER_SIZE];
static int buffer_size;

void init_kb(void)
{
    cbreak();
    nodelay(stdscr, TRUE);
    noecho();
    keypad(stdscr, TRUE);
    buffer_size = 0;
}

void kb_update(struct vcpu *cpu)
{
    unsigned short fx;
    unsigned short key;
    int ch = getch();

    if(ch != ERR) {
        switch(ch) {
            case EXT_BACKSPACE_1:
            case EXT_BACKSPACE_2:
            case EXT_BACKSPACE_3:
                key = KB_CHR_BACKSP;
                goto done;
            case EXT_RETURN_1:
            case EXT_RETURN_2:
                key = KB_CHR_RETURN;
                goto done;
            case KEY_IC:
                key = KB_CHR_INSERT;
                goto done;
            case KEY_DC:
                key = KB_CHR_DELETE;
                goto done;
            case KEY_UP:
                key = KB_CHR_UP;
                goto done;
            case KEY_DOWN:
                key = KB_CHR_DOWN;
                goto done;
            case KEY_LEFT:
                key = KB_CHR_LEFT;
                goto done;
            case KEY_RIGHT:
                key = KB_CHR_RIGHT;
                goto done;
            case KEY_SLEFT:
            case KEY_SRIGHT:
                key = KB_CHR_SHIFT;
                goto done;
            case EXT_TAB_1:
            case EXT_TAB_2:
                key = KB_CHR_TAB;
                goto done;
        }

        for(fx = 0; fx < 16; fx++) {
            if(ch != KEY_F(fx))
                continue;
            key = KB_CHR_FX + fx;
            goto done;
        }

        key = ch & 0xFF;
        goto done;
    }

    return;

done:
    if(buffer_size < KB_BUFFER_SIZE) {
        buffer[buffer_size++] = key;
        vcpu_interrupt(cpu, KB_HARDWARE_ID);
    }
}

int kb_ioread(struct vcpu *cpu, unsigned short port, unsigned short *value)
{
    switch(port) {
        case KB_IOPORT:
            if(buffer_size > 0)
                *value = buffer[--buffer_size];
            return 1;
    }

    return 0;
}
