#ifndef GT86_H_
#define GT86_H_
#include <SDL2/SDL.h>
#include <V16.h>

#define GT86_WIDTH      (80)
#define GT86_HEIGHT     (25)
#define GT86_VIDPTR     (0x8000)
#define GT86_CHARPTR    (0x8A00)
#define GT86_CH_WIDTH   (4)
#define GT86_CH_HEIGHT  (8)
#define GT86_FPS        (50) // A standard frequency in Russia

void GT86_init(SDL_Renderer *renderer, V16_vm_t *vm);
void GT86_shutdown();
void GT86_render(SDL_Renderer *renderer, const V16_vm_t *vm);

#endif
