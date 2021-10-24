#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_ 1
#include <SDL2/SDL.h>
#include <vcpu16.h>

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

void init_kb(void);
void kb_update(struct vcpu *cpu, const SDL_Event *event);
int kb_ioread(struct vcpu *cpu, unsigned short port, unsigned short *value);

#endif 
