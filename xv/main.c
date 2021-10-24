#include <errno.h>
#include <glad/gl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <vcpu16.h>
#include "keyboard.h"
#include "lpm25.h"

static void xv_ioread(struct vcpu *cpu, unsigned short port, unsigned short *value)
{
    if(kb_ioread(cpu, port, value))
        return;
    if(lpm25_ioread(cpu, port, value))
        return;
}

static void xv_iowrite(struct vcpu *cpu, unsigned short port, unsigned short value)
{
    if(lpm25_iowrite(cpu, port, value))
        return;
}

int main(int argc, char **argv)
{
    SDL_Window *window;
    SDL_GLContext context;
    struct vcpu cpu;
    FILE *infile;
    size_t i;
    float cpu_freq, cpu_step, cpu_clock, frequency, frametime;
    Uint64 lasttime, curtime;
    Uint32 fps;
    int running, rendering;
    SDL_Event event;

    if(SDL_Init(SDL_INIT_EVERYTHING)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init failed");
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    window = SDL_CreateWindow(argv[0], SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, LPM25_WIDTH * LPM25_CH_WIDTH * 4, LPM25_HEIGHT * LPM25_CH_HEIGHT * 4, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if(!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateWindow failed");
        return 1;
    }
    
    context = SDL_GL_CreateContext(window);
    if(!context) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_GL_CreateContext failed");
        return 1;
    }

    SDL_GL_MakeCurrent(window, context);
    if(!gladLoadGL((GLADloadfunc)(&SDL_GL_GetProcAddress))) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "gladLoadGL failed");
        return 1;
    }

    if(!GLAD_GL_EXT_texture_filter_anisotropic)
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "No anisotropic filtering extension found, expect bad quality!");

    init_vcpu(&cpu, NULL);
    cpu.on_ioread = &xv_ioread;
    cpu.on_iowrite = &xv_iowrite;

    if(argc < 2) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Argument required!\n");
        return 1;
    }
    
    infile = fopen(argv[1], "rb");
    if(!infile) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: %s", argv[1], strerror(errno));
        return 1;
    }

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
    init_lpm25(&cpu);

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

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        lpm25_render(&cpu);
        SDL_GL_SwapWindow(window);

        SDL_Delay(fps);

        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                running = 0;
                break;
            }

            if(event.type == SDL_WINDOWEVENT) {
                switch(event.window.event) {
                    case SDL_WINDOWEVENT_RESTORED:
                        rendering = 1;
                        break;
                    case SDL_WINDOWEVENT_MINIMIZED:
                        rendering = 0;
                        break;
                    case SDL_WINDOWEVENT_RESIZED:
                        glViewport(0, 0, event.window.data1, event.window.data2);
                        break;
                }

                break;
            }

            kb_update(&cpu, &event);
        }
    } 

    shutdown_lpm25();
    shutdown_vcpu(&cpu);
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
