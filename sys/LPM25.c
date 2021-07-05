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
#include <string.h>
#include "LPM25.h"

#define LPM25_DEFAULT_TEXT (0x8000)
#define LPM25_DEFAULT_CHAR (0x8A00)

static const uint16_t charset[256 * sizeof(uint16_t)] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,

    0x0000, 0x0000, 0x0888, 0x8808, 0x0AA0, 0x0000, 0x0AEA, 0xAAEA,
    0x04E8, 0xE2E4, 0x0028, 0x4280, 0x04A8, 0x4AA4, 0x0440, 0x0000,
    0x0244, 0x4442, 0x0844, 0x4448, 0x000A, 0x4A00, 0x0004, 0xE400,
    0x0000, 0x0044, 0x0000, 0xE000, 0x0000, 0x0004, 0x0222, 0x4444,

    0x04AA, 0xAAA4, 0x0262, 0x2222, 0x04AA, 0x488E, 0x04A2, 0x42A4,
    0x0AAA, 0xE222, 0x0E88, 0xE22E, 0x04A8, 0xCAA4, 0x0E22, 0x2222,
    0x04AA, 0x4AA4, 0x04AA, 0x62A4,
    
    0x0080, 0x0080, 0x0080, 0x0088, 0x0024, 0x8420, 0x000E, 0x0E00,
    0x0084, 0x2480, 0x04A2, 0x4404, 0x04AA, 0xEAA4,
    
    0x04AA, 0xEAAA, 0x0CAA, 0xCAAE, 0x04A8, 0x88A4, 0x0CAA, 0xAAAE,
    0x0E88, 0xC88E, 0x0E88, 0xC888, 0x04A8, 0x4AAE, 0x0AAA, 0xEAAA,
    0x0E44, 0x444E, 0x0E44, 0x444C, 0x0AAC, 0xAAAA, 0x0888, 0x888E,
    0x0AEE, 0xAAAA, 0x0AEE, 0xEEEA, 0x0EAA, 0xAAAE, 0x0EAA, 0xC888,
    0x0EAA, 0xAAC2, 0x0EAA, 0xCAAA, 0x0688, 0x422C, 0x0E44, 0x4444,
    0x0AAA, 0xAAAE, 0x0AAA, 0xAAA4, 0x0AAA, 0xAAEE, 0x0AAA, 0x4AAA,
    0x0AAA, 0x4444, 0x0E22, 0x488E,

    0x0644, 0x4446, 0x0444, 0x2222, 0x0C44, 0x444C, 0x04A0, 0x0000,
    0x0000, 0x000E, 0x0840, 0x0000,

    0x000C, 0x26A4, 0x0888, 0xCAA4, 0x0004, 0xA8A4, 0x0222, 0x6AA4,
    0x0006, 0xAC86, 0x04A8, 0xC888, 0x0006, 0x8AA4, 0x0888, 0xCAAA,
    0x0404, 0x4444, 0x0404, 0x444C, 0x088A, 0xACAA, 0x0C44, 0x4446,
    0x000A, 0xEAAA, 0x000C, 0xAAAA, 0x0004, 0xAAA4, 0x0004, 0xAC88,
    0x0004, 0xA622, 0x000A, 0xC888, 0x0006, 0x842C, 0x044E, 0x4446,
    0x000A, 0xAAAC, 0x000A, 0xAAA4, 0x000A, 0xAAEE, 0x000A, 0xA4AA,
    0x000A, 0xA444, 0x000E, 0x248E,

    0x0244, 0xC442, 0x0444, 0x4444, 0x0844, 0x6448, 0x0002, 0xE800,

    0x0000, 0x0000,

    // TODO: extended ASCII table with cyrillic characters and
    // stuff
};

static SDL_Texture *texture = NULL;
static uint16_t text_off = 0;
static uint16_t char_off = 0;

static inline void LPM25_unpackColor(uint8_t value, SDL_Color *c)
{
    Uint8 cv = (value & 1) ? 255 : 180;
    c->r = ((value >> 3) & 1) ? cv : 0;
    c->g = ((value >> 2) & 1) ? cv : 0;
    c->b = ((value >> 1) & 1) ? cv : 0;
}

void LPM25_init(SDL_Renderer *renderer, V16_vm_t *vm)
{
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, LPM25_WIDTH * LPM25_CH_WIDTH, LPM25_HEIGHT * LPM25_CH_HEIGHT);
    text_off = LPM25_DEFAULT_TEXT;
    char_off = LPM25_DEFAULT_CHAR;
    memcpy(vm->memory + char_off, charset, sizeof(charset));
}

void LPM25_shutdown()
{
    SDL_DestroyTexture(texture);
}

void LPM25_render(SDL_Renderer *renderer, const V16_vm_t *vm)
{
    SDL_SetRenderTarget(renderer, texture);
    for(int i = 0; i < LPM25_HEIGHT; i++) {
        for(int j = 0; j < LPM25_WIDTH; j++) {
            uint16_t word = vm->memory[text_off + i * LPM25_WIDTH + j];
            const uint16_t *chp = vm->memory + char_off + (word & 0xFF) * 2;

            SDL_Color bg, fg;
            LPM25_unpackColor((word >> 12) & 0x0F, &bg);
            LPM25_unpackColor((word >> 8) & 0x0F, &fg);

            uint32_t chv = (chp[0] << 16) | chp[1];
            for(int y = 0; y < LPM25_CH_HEIGHT; y++) {
                uint8_t row = (chv >> (32 - (LPM25_CH_WIDTH * (y + 1)))) & 0x0F;
                for(int x = 0; x < LPM25_CH_WIDTH; x++) {
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

bool LPM25_ioread(V16_vm_t *vm, uint16_t port, uint16_t *value)
{
    switch(port) {
        case LPM25_IOPORT_TEXT:
            value[0] = text_off;
            return true;
        case LPM25_IOPORT_CHAR:
            value[0] = char_off;
            return true;
    }

    return false;
}

void LPM25_iowrite(V16_vm_t *vm, uint16_t port, uint16_t value)
{
    switch(port) {
        case LPM25_IOPORT_TEXT:
            text_off = value;
            return;
        case LPM25_IOPORT_CHAR:
            char_off = value;
            return;
    }
}
