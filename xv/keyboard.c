#include <ctype.h>
#include "keyboard.h"

static unsigned short buffer[KB_BUFFER_SIZE];
static int buffer_size;

void init_kb(void)
{
    SDL_StartTextInput();
    buffer_size = 0;
}

void kb_update(struct vcpu *cpu, const SDL_Event *event)
{
    const char *text;
    unsigned short keyval;

    if(event->type == SDL_TEXTINPUT) {
        text = event->text.text;
        while(buffer_size < KB_BUFFER_SIZE && text[0]) {
            buffer[buffer_size++] = text++[0];
            vcpu_interrupt(cpu, KB_HARDWARE_ID);
        }
        return;
    }

    if(event->type == SDL_KEYDOWN && buffer_size < KB_BUFFER_SIZE) {
        keyval = 0;
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
        }

        if(keyval) {
            buffer[buffer_size++] = keyval;
            vcpu_interrupt(cpu, KB_HARDWARE_ID);
        }

        return;
    }
}

void kb_ioread(struct vcpu *cpu, unsigned short port, unsigned short *value)
{
    switch(port) {
        case KB_IOPORT:
            if(buffer_size > 0)
                *value = buffer[--buffer_size];
            break;
    }
}
