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
#ifndef LPM25_H_
#define LPM25_H_
#include <SDL2/SDL.h>
#include <V16.h>

#define LPM25_WIDTH         (80)
#define LPM25_HEIGHT        (25)
#define LPM25_CH_WIDTH      (4)
#define LPM25_CH_HEIGHT     (8)
#define LPM25_FPS           (50)
#define LPM25_IOPORT_TEXT   (0x1F01)
#define LPM25_IOPORT_CHAR   (0x1F02)

void LPM25_init(SDL_Renderer *renderer, V16_vm_t *vm);
void LPM25_shutdown();
void LPM25_render(SDL_Renderer *renderer, const V16_vm_t *vm);
bool LPM25_ioread(V16_vm_t *vm, uint16_t port, uint16_t *value);
void LPM25_iowrite(V16_vm_t *vm, uint16_t port, uint16_t value);

#endif
