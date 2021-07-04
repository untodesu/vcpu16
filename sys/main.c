#include <SDL2/SDL.h>
#include <stdarg.h>
#include <stdio.h>
#include <V16.h>
#include "GT86.h"

static void die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    abort();
}

static int sysioread(V16_vm_t *vm, uint16_t port, uint16_t *value)
{
    if(port == 0x00FF) {
        value[0] = (uint16_t)getchar();
        return 1;
    }

    return 0;
}

int main(int argc, char **argv)
{
    if(SDL_Init(SDL_INIT_EVERYTHING))
        die("SDL_Init failed\n");

    SDL_Window *window = SDL_CreateWindow(argv[0], SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, GT86_WIDTH * GT86_CH_WIDTH * 4, GT86_HEIGHT * GT86_CH_HEIGHT * 4, 0);
    if(!window)
        die("SDL_CreateWindow failed\n");
    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED);
    if(!renderer)
        die("SDL_CreateRenderer failed\n");

    V16_vm_t vm;
    if(!V16_open(&vm))
        die("V16_open failed\n");

    vm.ioread = sysioread;

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

    GT86_init(renderer, &vm);

    float cpu_freq = (float)V16_FREQUENCY;
    if(argc > 2)
        cpu_freq = atof(argv[2]);

    float cpu_step = 1.0f / cpu_freq;
    float cpu_clock = 0.0f;
    float frequency = (float)SDL_GetPerformanceFrequency();
    Uint64 lasttime = SDL_GetPerformanceCounter();
    Uint32 fps = (Uint32)(1000.0f / (float)GT86_FPS);

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
            GT86_render(renderer, &vm);
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
        }
    } 

    GT86_shutdown();
    V16_close(&vm);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
