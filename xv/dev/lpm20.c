#include <ctype.h>
#include <ncurses.h>
#include "dev/lpm20.h"

#define MAX_COLORS 8

static int width = 0;
static int height = 0;
static unsigned short text_off = 0;
static unsigned short cursor_pos = 0;

static const short colormap[MAX_COLORS] = {
    COLOR_BLACK,
    COLOR_BLUE,
    COLOR_GREEN,
    COLOR_CYAN,
    COLOR_RED,
    COLOR_MAGENTA,
    COLOR_YELLOW,
    COLOR_WHITE
};

static short calc_pair_id(short bg, short fg)
{
    return ((bg & 7) << 4) | (fg & 7);
}

void init_lpm20(void)
{
    short i, j;

    width = getmaxx(stdscr);
    height = getmaxy(stdscr);

    /* Limit the screen height */
    if((width * height) >= LPM20_MAX_MEMORY)
        height = LPM20_MAX_MEMORY / width;

    text_off = 0x8000;

    for(i = 0; i < MAX_COLORS; i++) for(j = 0; j < MAX_COLORS; j++)
        init_pair(calc_pair_id(i, j), j, i);
}

void shutdown_lpm20(void)
{
    width = 0;
    height = 0;
    text_off = 0;
}

void lpm20_draw(const struct vcpu *cpu)
{
    int i, j;
    unsigned short word;
    unsigned char abyte, cbyte;
    int attrib;

    for(i = 0; i < height; i++) {
        for(j = 0; j < width; j++) {
            word = (*cpu->memory)[(text_off + (i * width) + j) & 0xFFFF];
            abyte = (word >> 8) & 0xFF;
            cbyte = word & 0xFF;
            attrib = 0;
            attrib |= COLOR_PAIR(calc_pair_id((abyte >> 4) & 7, abyte & 7));
            if(abyte & (1 << 3))
                attrib |= A_BOLD;
            if(abyte & (1 << 7))
                attrib |= A_REVERSE;
            attron(attrib);
            mvaddch(i, j, isprint(cbyte) ? cbyte : ' ');
            attroff(attrib);
        }
    }

    move(cursor_pos / width, cursor_pos % width);
}

int lpm20_ioread(struct vcpu *cpu, unsigned short port, unsigned short *value)
{
    switch(port) {
        case LPM20_IOPORT_TEXT_OFF:
            *value = text_off;
            return 1;
        case LPM20_IOPORT_CUR_POS:
            *value = cursor_pos;
            return 1;
        case LPM20_IOPORT_SCR_DIMS:
            *value = ((width & 0xFF) << 8) | (height & 0xFF);
            return 1;
    }

    return 0;
}

int lpm20_iowrite(struct vcpu *cpu, unsigned short port, unsigned short value)
{
    switch(port) {
        case LPM20_IOPORT_TEXT_OFF:
            text_off = value;
            return 1;
        case LPM20_IOPORT_CUR_POS:
            cursor_pos = value;
            return 1;
    }

    return 0;
}
