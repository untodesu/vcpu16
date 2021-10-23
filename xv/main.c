#include <SDL2/SDL.h>
#include <stdarg.h>
#include <stdio.h>
#include <vcpu16.h>
#include "keyboard.h"
#include "lpm25.h"

static void die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    abort();
}

static void xv_ioread(struct vcpu *cpu, unsigned short port, unsigned short *value)
{
    kb_ioread(cpu, port, value);
    lpm25_ioread(cpu, port, value);
}

static void xv_iowrite(struct vcpu *cpu, unsigned short port, unsigned short value)
{
    lpm25_iowrite(cpu, port, value);
}

int main(int argc, char **argv)
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    struct vcpu cpu;
    FILE *infile;
    size_t i;
    float cpu_freq, cpu_step, cpu_clock, frequency, frametime;
    Uint64 lasttime, curtime;
    Uint32 fps;
    int running, rendering;
    SDL_Event event;

    if(SDL_Init(SDL_INIT_EVERYTHING))
        die("SDL_Init failed\n");

    window = SDL_CreateWindow(argv[0], SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, LPM25_WIDTH * LPM25_CH_WIDTH * 4, LPM25_HEIGHT * LPM25_CH_HEIGHT * 4, 0);
    if(!window)
        die("SDL_CreateWindow failed\n");
    
    renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED);
    if(!renderer)
        die("SDL_CreateRenderer failed\n");

    init_vcpu(&cpu, NULL);
    cpu.on_ioread = &xv_ioread;
    cpu.on_iowrite = &xv_iowrite;

    if(argc < 2)
        die("Argument required!\n");
    
    infile = fopen(argv[1], "rb");
    if(!infile)
        die("Failed to load %s\n", argv[1]);

    fseek(infile, 0, SEEK_END);
    long size = ftell(infile) / sizeof(uint16_t);
    fseek(infile, 0, SEEK_SET);

    if(size > VCPU_MEM_SIZE)
        size = VCPU_MEM_SIZE;
    
    fread(*cpu.memory, sizeof(uint16_t), size, infile);
    fclose(infile);

    // Patch endianness
    for(i = 0; i < VCPU_MEM_SIZE; i++)
        (*cpu.memory)[i] = vcpu_be16_to_host((*cpu.memory)[i]);

    init_kb();
    init_lpm25(renderer, &cpu);

    cpu_freq = 24000.0f;
    if(argc > 2)
        cpu_freq = atof(argv[2]);

    cpu_step = 1.0f / cpu_freq;
    cpu_clock = 0.0f;
    frequency = (float)SDL_GetPerformanceFrequency();
    lasttime = SDL_GetPerformanceCounter();
    fps = (Uint32)(1000.0f / (float)LPM25_FPS);
    for(running = 1, rendering = 1; running;) {
        curtime = SDL_GetPerformanceCounter();
        float frametime = (float)(curtime - lasttime) / frequency;
        lasttime = curtime;

        cpu_clock += frametime;
        while(cpu_clock >= cpu_step) {
            if(!vcpu_step(&cpu))
                running = 0;
            cpu_clock -= cpu_step;
        }

        if(rendering) {
            lpm25_render(renderer, &cpu);
            SDL_RenderPresent(renderer);
        }

        SDL_Delay(fps);

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

            kb_update(&cpu, &event);
        }
    } 

    shutdown_lpm25();
    shutdown_vcpu(&cpu);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
