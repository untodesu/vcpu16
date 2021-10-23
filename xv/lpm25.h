#ifndef _LPM25_H_
#define _LPM25_H_
#include <SDL2/SDL.h>
#include <vcpu16.h>

#define LPM26_HARDWARE_ID       (0x1F25)
#define LPM25_WIDTH             (80)
#define LPM25_HEIGHT            (25)
#define LPM25_CH_WIDTH          (4)
#define LPM25_CH_HEIGHT         (8)
#define LPM25_FPS               (50)
#define LPM25_IOPORT_TEXT_OFF   (0x1F01)
#define LPM25_IOPORT_CHAR_OFF   (0x1F02)
#define LPM25_IOPORT_CUR_POS    (0x1F03)
#define LPM25_IOPORT_CUR_BLINK  (0x1F04)

void init_lpm25(SDL_Renderer *renderer, struct vcpu *cpu);
void shutdown_lpm25(void);
void lpm25_render(SDL_Renderer *renderer, const struct vcpu *cpu);
void lpm25_ioread(struct vcpu *cpu, unsigned short port, unsigned short *value);
void lpm25_iowrite(struct vcpu *cpu, unsigned short port, unsigned short value);

#endif
