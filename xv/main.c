#include <errno.h>
#include <ncurses.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vcpu16.h>
#include "dev/kb.h"
#include "dev/lpm20.h"
#include "cross_clock.h"

static void xv_ioread(struct vcpu *cpu, unsigned short port, unsigned short *value)
{
    if(kb_ioread(cpu, port, value))
        return;
    if(lpm20_ioread(cpu, port, value))
        return;
}

static void xv_iowrite(struct vcpu *cpu, unsigned short port, unsigned short value)
{
    if(lpm20_iowrite(cpu, port, value))
        return;
}

int main(int argc, char **argv)
{
    struct vcpu cpu;
    FILE *infile;
    long size, i;
    WINDOW *window;
    int running;
    float vcpu_dt, vcpu_clock;
    float curtime, lasttime, dt;

    init_vcpu(&cpu, NULL);
    cpu.on_ioread = &xv_ioread;
    cpu.on_iowrite = &xv_iowrite;

    if(argc < 2) {
        fprintf(stderr, "%s: argument required!\n", argv[0]);
        return 1;
    }

    if(argc >= 3) {
        cpu.cpi.speed = strtoul(argv[2], NULL, 10);
        if(!cpu.cpi.speed)
            cpu.cpi.speed = VCPU_CPI_DEF_SPEED;
    }

    infile = fopen(argv[1], "rb");
    if(!infile) {
        fprintf(stderr, "%s: %s!\n", argv[1], strerror(errno));
        return 1;
    }

    fseek(infile, 0, SEEK_END);
    size = ftell(infile) / sizeof(unsigned short);
    fseek(infile, 0, SEEK_SET);

    if(size > VCPU_MEM_SIZE)
        size = VCPU_MEM_SIZE;
    
    fread(*cpu.memory, sizeof(unsigned short), size, infile);
    fclose(infile);

    // Patch endianness
    for(i = 0; i < VCPU_MEM_SIZE; i++)
        (*cpu.memory)[i] = vcpu_be16_to_host((*cpu.memory)[i]);

    initscr();
    if(has_colors())
        start_color();
    noecho();

    init_kb();
    init_lpm20();

    vcpu_dt = 1.0 / (float)cpu.cpi.speed;;
    vcpu_clock = 0.0;

    init_cross_clock();
    lasttime = cross_clock_value_seconds();

    for(running = 1; running;) {
        curtime = cross_clock_value_seconds();
        dt = curtime - lasttime;
        lasttime = curtime;

        vcpu_clock += dt;
        while(vcpu_clock >= vcpu_dt) {
            if(!vcpu_step(&cpu))
                running = 0;
            vcpu_clock -= vcpu_dt;
        }

        lpm20_draw(&cpu);
        kb_update(&cpu);
        refresh();

        napms(20);
    }

    endwin();
    shutdown_vcpu(&cpu);
    return 0;
}
