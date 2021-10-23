#include <string.h>
#include "lpm25.h"

struct lpm_cursor {
    unsigned short pos;
    unsigned short blink;
    int visible;
    Uint32 last_swap;
};

static const unsigned short charset[128 * 2] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,

    0x0000, 0x0000, 0x0444, 0x4404, 0x00AA, 0x0000, 0x00AE, 0xAEA0,
    0x04E8, 0xE2E4, 0x0A24, 0x448A, 0x04A8, 0x4AA4, 0x0044, 0x0000,
    0x0244, 0x4442, 0x0844, 0x4448, 0x000A, 0x4A00, 0x0004, 0xE400,
    0x0000, 0x0044, 0x0000, 0xE000, 0x0000, 0x0004, 0x0222, 0x4444,

    0x04AA, 0xAAA4, 0x0262, 0x2222, 0x04AA, 0x488E, 0x04A2, 0x42A4,
    0x0AAA, 0xE222, 0x0E88, 0xE22E, 0x04A8, 0xCAA4, 0x0E22, 0x2222,
    0x04AA, 0x4AA4, 0x04AA, 0x62A4,
    
    0x0080, 0x0080, 0x0080, 0x0088, 0x0024, 0x8420, 0x000E, 0x0E00,
    0x0084, 0x2480, 0x04A2, 0x4404, 0x04AA, 0xE8A4,
    
    0x04AA, 0xEAAA, 0x0CAA, 0xCAAC, 0x04A8, 0x88A4, 0x0CAA, 0xAAAC,
    0x0E88, 0xC88E, 0x0E88, 0xC888, 0x04A8, 0xEAA4, 0x0AAA, 0xEAAA,
    0x0E44, 0x444E, 0x0E44, 0x444C, 0x0AAC, 0xAAAA, 0x0888, 0x888E,
    0x0AEE, 0xAAAA, 0x0CAA, 0xAAAA, 0x0EAA, 0xAAAE, 0x0CAA, 0xC888,
    0x0EAA, 0xAAC2, 0x0EAA, 0xCAAA, 0x0688, 0x422C, 0x0E44, 0x4444,
    0x0AAA, 0xAAAE, 0x0AAA, 0xAAA4, 0x0AAA, 0xAAEA, 0x0AAA, 0x4AAA,
    0x0AAA, 0x4444, 0x0E22, 0x488E,

    0x0644, 0x4446, 0x0444, 0x2222, 0x0C44, 0x444C, 0x04A0, 0x0000,
    0x0000, 0x000E, 0x0840, 0x0000,

    0x000C, 0x26A4, 0x0888, 0xCAA4, 0x0004, 0xA8A4, 0x0222, 0x6AA4,
    0x0004, 0xAC86, 0x04A8, 0xC888, 0x0006, 0x8AA4, 0x0888, 0xCAAA,
    0x0404, 0x4444, 0x0404, 0x444C, 0x088A, 0xACAA, 0x0C44, 0x4446,
    0x000A, 0xEAAA, 0x000C, 0xAAAA, 0x0004, 0xAAA4, 0x0004, 0xAC88,
    0x0004, 0xA622, 0x000A, 0xC888, 0x0006, 0x842C, 0x044E, 0x4446,
    0x000A, 0xAAAE, 0x000A, 0xAAA4, 0x000A, 0xAAEA, 0x000A, 0xA4AA,
    0x000A, 0xA444, 0x000E, 0x248E,

    0x0244, 0xC442, 0x0444, 0x4444, 0x0844, 0x6448, 0x0002, 0xE800,

    0x0000, 0x0000
};

static SDL_Texture *texture = NULL;
static unsigned short text_off = 0;
static unsigned short char_off = 0;
static struct lpm_cursor cursor;

static inline void unpack_color(uint8_t value, SDL_Color *c)
{
    Uint8 cv = (value & 1) ? 255 : 180;
    c->r = ((value >> 3) & 1) ? cv : 0;
    c->g = ((value >> 2) & 1) ? cv : 0;
    c->b = ((value >> 1) & 1) ? cv : 0;
}

static inline void invert_color(SDL_Color *c)
{
    c->r = (255 - c->r);
    c->g = (255 - c->g);
    c->b = (255 - c->b);
}

void init_lpm25(SDL_Renderer *renderer, struct vcpu *cpu)
{
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, LPM25_WIDTH * LPM25_CH_WIDTH, LPM25_HEIGHT * LPM25_CH_HEIGHT);
    text_off = 0x8000;
    char_off = 0x8A00;
    cursor.pos = 0;
    cursor.blink = 500;
    cursor.visible = 1;
    cursor.last_swap = SDL_GetTicks();
    memcpy((*cpu->memory) + char_off, charset, sizeof(charset));
}

void shutdown_lpm25()
{
    SDL_DestroyTexture(texture);
}

void lpm25_render(SDL_Renderer *renderer, const struct vcpu *cpu)
{
    Uint32 ticks;
    SDL_Color bg, fg;
    unsigned int chv;
    int i, j;
    unsigned short pos, word;
    const unsigned short *chp;
    unsigned char row;
    int y, x;
    
    ticks = SDL_GetTicks();
    if(cursor.blink && ((ticks - cursor.last_swap) > (Uint32)cursor.blink)) {
        cursor.visible = !cursor.visible;
        cursor.last_swap = ticks;
    }

    SDL_SetRenderTarget(renderer, texture);
    for(i = 0; i < LPM25_HEIGHT; i++) {
        for(j = 0; j < LPM25_WIDTH; j++) {
            pos = i * LPM25_WIDTH + j;
            word = (*cpu->memory)[text_off + pos];
            chp = (*cpu->memory) + char_off + (word & 0xFF) * 2;

            unpack_color((word >> 12) & 0x0F, &bg);
            unpack_color((word >> 8) & 0x0F, &fg);

            if(pos == cursor.pos && cursor.visible) {
                invert_color(&bg);
                invert_color(&fg);
            }

            chv = (chp[0] << 16) | chp[1];
            for(y = 0; y < LPM25_CH_HEIGHT; y++) {
                row = (chv >> (32 - (LPM25_CH_WIDTH * (y + 1)))) & 0x0F;
                for(x = 0; x < LPM25_CH_WIDTH; x++) {
                    if((row >> (LPM25_CH_WIDTH - x - 1)) & 1)
                        SDL_SetRenderDrawColor(renderer, fg.r, fg.g, fg.b, 255);
                    else
                        SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, 255);
                    SDL_RenderDrawPoint(renderer, j * LPM25_CH_WIDTH + x, i * LPM25_CH_HEIGHT + y);
                }
            }
        }
    }
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
}

void lpm25_ioread(struct vcpu *cpu, unsigned short port, unsigned short *value)
{
    switch(port) {
        case LPM25_IOPORT_TEXT_OFF:
            *value = text_off;
            break;
        case LPM25_IOPORT_CHAR_OFF:
            *value = char_off;
            break;
        case LPM25_IOPORT_CUR_POS:
            *value = cursor.pos;
            break;
        case LPM25_IOPORT_CUR_BLINK:
            *value = cursor.blink;
            break;
    }
}

void lpm25_iowrite(struct vcpu *cpu, unsigned short port, unsigned short value)
{
    switch(port) {
        case LPM25_IOPORT_TEXT_OFF:
            text_off = value;
            return;
        case LPM25_IOPORT_CHAR_OFF:
            char_off = value;
            return;
        case LPM25_IOPORT_CUR_POS:
            cursor.pos = value;
            return;
        case LPM25_IOPORT_CUR_BLINK:
            cursor.blink = value;
            cursor.visible = !!value;
            return;
    }
}
