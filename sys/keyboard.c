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
#include "keyboard.h"

static uint16_t buffer[KB_BUFFER_SIZE];
static int buffer_size;

void KB_init()
{
    buffer_size = 0;
}

void KB_update(V16_vm_t *vm, const SDL_Event *event)
{
    if(event->type == SDL_KEYUP && buffer_size < KB_BUFFER_SIZE) {
        uint16_t keyval = 0;
        switch(event->key.keysym.sym) {
            case SDLK_BACKSPACE:
                keyval = KB_CHR_BACKSP;
                break;
            case SDLK_RETURN:
            case SDLK_RETURN2:
                keyval = KB_CHR_RETURN;
                break;
            case SDLK_INSERT:
                keyval = KB_CHR_INSERT;
                break;
            case SDLK_DELETE:
                keyval = KB_CHR_DELETE;
                break;
            case SDLK_UP:
                keyval = KB_CHR_UP;
                break;
            case SDLK_DOWN:
                keyval = KB_CHR_DOWN;
                break;
            case SDLK_LEFT:
                keyval = KB_CHR_LEFT;
                break;
            case SDLK_RIGHT:
                keyval = KB_CHR_RIGHT;
                break;
            case SDLK_LSHIFT:
            case SDLK_RSHIFT:
                keyval = KB_CHR_SHIFT;
                break;
            case SDLK_LCTRL:
            case SDLK_RCTRL:
                keyval = KB_CHR_CTRL;
                break;
            default:
                keyval = (uint16_t)(toupper(event->key.keysym.sym & 0x7F));
                break;
        }

        buffer[buffer_size++] = keyval;
        V16_interrupt(vm, KB_HARDWARE_ID);
    }
}

bool KB_ioread(V16_vm_t *vm, uint16_t port, uint16_t *value)
{
    if(port == KB_IOPORT && buffer_size > 0) {
        value[0] = buffer[--buffer_size];
        return true;
    }
}
