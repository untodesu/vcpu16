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
#ifndef KEYBOARD_H_
#define KEYBOARD_H_ 1
#include <SDL2/SDL.h>
#include <V16.h>

#define KB_BUFFER_SIZE  (16)
#define KB_HARDWARE_ID  (0x000F)
#define KB_IOPORT       (0x000F)
#define KB_CHR_BACKSP   (0xFF01)
#define KB_CHR_RETURN   (0xFF02)
#define KB_CHR_INSERT   (0xFF03)
#define KB_CHR_DELETE   (0xFF04)
#define KB_CHR_UP       (0xFF05)
#define KB_CHR_DOWN     (0xFF06)
#define KB_CHR_LEFT     (0xFF07)
#define KB_CHR_RIGHT    (0xFF08)
#define KB_CHR_SHIFT    (0xFF09)
#define KB_CHR_CTRL     (0xFF0A)

void KB_init();
void KB_update(V16_vm_t *vm, const SDL_Event *event);
bool KB_ioread(V16_vm_t *vm, uint16_t port, uint16_t *value);

#endif 
