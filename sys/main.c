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
#include <SDL2/SDL.h>
#include <stdarg.h>
#include <stdio.h>
#include <V16.h>
#include "keyboard.h"
#include "LPM25.h"

static void die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    abort();
}

static bool SYS_ioread(V16_vm_t *vm, uint16_t port, uint16_t *value)
{
    if(KB_ioread(vm, port, value))
        return true;
    if(LPM25_ioread(vm, port, value))
        return true;
    return 0;
}

static void SYS_iowrite(V16_vm_t *vm, uint16_t port, uint16_t value)
{
    LPM25_iowrite(vm, port, value);
}

int main(int argc, char **argv)
{
    if(SDL_Init(SDL_INIT_EVERYTHING))
        die("SDL_Init failed\n");

    SDL_Window *window = SDL_CreateWindow(argv[0], SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, LPM25_WIDTH * LPM25_CH_WIDTH * 4, LPM25_HEIGHT * LPM25_CH_HEIGHT * 4, 0);
    if(!window)
        die("SDL_CreateWindow failed\n");
    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED);
    if(!renderer)
        die("SDL_CreateRenderer failed\n");

    V16_vm_t vm;
    if(!V16_open(&vm))
        die("V16_open failed\n");

    vm.ioread = SYS_ioread;
    vm.iowrite = SYS_iowrite;

    if(argc < 2)
        die("Argument required!\n");
    
    FILE *infile = fopen(argv[1], "rb");
    if(!infile)
        die("Failed to load %s\n", argv[1]);

    fseek(infile, 0, SEEK_END);
    long size = ftell(infile) / sizeof(uint16_t);
    fseek(infile, 0, SEEK_SET);

    if(size > V16_MEM_SIZE)
        size = V16_MEM_SIZE;
    
    fread(vm.memory, sizeof(uint16_t), size, infile);
    fclose(infile);

    // Patch endianness
    for(size_t i = 0; i < V16_MEM_SIZE; i++)
        vm.memory[i] = V16_BE16ToHost(vm.memory[i]);

    KB_init();
    LPM25_init(renderer, &vm);

    float cpu_freq = (float)V16_FREQUENCY;
    if(argc > 2)
        cpu_freq = atof(argv[2]);

    float cpu_step = 1.0f / cpu_freq;
    float cpu_clock = 0.0f;
    float frequency = (float)SDL_GetPerformanceFrequency();
    Uint64 lasttime = SDL_GetPerformanceCounter();
    Uint32 fps = (Uint32)(1000.0f / (float)LPM25_FPS);

    for(int running = 1, rendering = 1; running;) {
        Uint64 curtime = SDL_GetPerformanceCounter();
        float frametime = (float)(curtime - lasttime) / frequency;
        lasttime = curtime;

        cpu_clock += frametime;
        while(cpu_clock >= cpu_step) {
            if(!V16_step(&vm))
                running = 0;
            cpu_clock -= cpu_step;
        }

        if(rendering) {
            LPM25_render(renderer, &vm);
            SDL_RenderPresent(renderer);
        }

        SDL_Delay(fps);

        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                running = 0;
                break;
            }

            if(event.type == SDL_WINDOWEVENT) {
                switch(event.window.type) {
                    case SDL_WINDOWEVENT_RESTORED:
                        rendering = 1;
                        break;
                    case SDL_WINDOW_MINIMIZED:
                        rendering = 0;
                        break;
                }
                break;
            }

            KB_update(&vm, &event);
        }
    } 

    LPM25_shutdown();
    V16_close(&vm);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
