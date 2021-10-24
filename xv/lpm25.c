#include <glad/gl.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>
#include "lpm25.h"

#define TEXTURE_WIDTH   (LPM25_WIDTH * LPM25_CH_WIDTH)
#define TEXTURE_HEIGHT  (LPM25_HEIGHT * LPM25_CH_HEIGHT)

struct lpm_cursor {
    unsigned short pos;
    unsigned short blink;
    int visible;
    Uint32 last_swap;
};

static const unsigned short charset[128 * 2] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,

    0x0000, 0x0000, 0x0444, 0x4404, 0x00AA, 0x0000, 0x00AE, 0xAEA0,
    0x04E8, 0xE2E4, 0x0A24, 0x448A, 0x04A8, 0x4AA4, 0x0044, 0x0000,
    0x0244, 0x4442, 0x0844, 0x4448, 0x000A, 0x4A00, 0x0004, 0xE400,
    0x0000, 0x0044, 0x0000, 0xE000, 0x0000, 0x0004, 0x0222, 0x4444,

    0x04AA, 0xAAA4, 0x0262, 0x2222, 0x04AA, 0x488E, 0x04A2, 0x42A4,
    0x0AAA, 0xE222, 0x0E88, 0xE22E, 0x04A8, 0xCAA4, 0x0E22, 0x2222,
    0x04AA, 0x4AA4, 0x04AA, 0x62A4,
    
    0x0080, 0x0080, 0x0080, 0x0088, 0x0024, 0x8420, 0x000E, 0x0E00,
    0x0084, 0x2480, 0x04A2, 0x4404, 0x04AA, 0xE8A4,
    
    0x04AA, 0xEAAA, 0x0CAA, 0xCAAC, 0x04A8, 0x88A4, 0x0CAA, 0xAAAC,
    0x0E88, 0xC88E, 0x0E88, 0xC888, 0x04A8, 0xEAA4, 0x0AAA, 0xEAAA,
    0x0E44, 0x444E, 0x0E44, 0x444C, 0x0AAC, 0xAAAA, 0x0888, 0x888E,
    0x0AEE, 0xAAAA, 0x0CAA, 0xAAAA, 0x0EAA, 0xAAAE, 0x0CAA, 0xC888,
    0x0EAA, 0xAAC2, 0x0EAA, 0xCAAA, 0x0688, 0x422C, 0x0E44, 0x4444,
    0x0AAA, 0xAAAE, 0x0AAA, 0xAAA4, 0x0AAA, 0xAAEA, 0x0AAA, 0x4AAA,
    0x0AAA, 0x4444, 0x0E22, 0x488E,

    0x0644, 0x4446, 0x0444, 0x2222, 0x0C44, 0x444C, 0x04A0, 0x0000,
    0x0000, 0x000E, 0x0840, 0x0000,

    0x000C, 0x26A4, 0x0888, 0xCAA4, 0x0004, 0xA8A4, 0x0222, 0x6AA4,
    0x0004, 0xAC86, 0x04A8, 0xC888, 0x0006, 0x8AA4, 0x0888, 0xCAAA,
    0x0404, 0x4444, 0x0404, 0x444C, 0x088A, 0xACAA, 0x0C44, 0x4446,
    0x000A, 0xEAAA, 0x000C, 0xAAAA, 0x0004, 0xAAA4, 0x0004, 0xAC88,
    0x0004, 0xA622, 0x000A, 0xC888, 0x0006, 0x842C, 0x044E, 0x4446,
    0x000A, 0xAAAE, 0x000A, 0xAAA4, 0x000A, 0xAAEA, 0x000A, 0xA4AA,
    0x000A, 0xA444, 0x000E, 0x248E,

    0x0244, 0xC442, 0x0444, 0x4444, 0x0844, 0x6448, 0x0002, 0xE800,

    0x0000, 0x0000
};

#define _string_1(x) #x
#define _string_2(x) _string_1(x)

static const char *vert_src =
    "#version 420 core                                                              \n"
    "const vec4 verts[6] = {                                                        \n"
    "   vec4(-1.0, -1.0, 0.0, 1.0),                                                 \n"
    "   vec4(-1.0,  1.0, 0.0, 0.0),                                                 \n"
    "   vec4( 1.0,  1.0, 1.0, 0.0),                                                 \n"
    "   vec4( 1.0,  1.0, 1.0, 0.0),                                                 \n"
    "   vec4( 1.0, -1.0, 1.0, 1.0),                                                 \n"
    "   vec4(-1.0, -1.0, 0.0, 1.0),                                                 \n"
    "};                                                                             \n"
    "layout(location = 0) out vec2 texcoord;                                        \n"
    "void main(void)                                                                \n"
    "{                                                                              \n"
    "   gl_Position = vec4(verts[gl_VertexID].xy, 0.0, 1.0);                        \n"
    "   texcoord = verts[gl_VertexID].zw;                                           \n"
    "}                                                                              \n";

static const char *frag_src =
    "#version 420 core                                                              \n"
    "#define TEXTURE_WIDTH "_string_2(TEXTURE_WIDTH)"                               \n"
    "#define TEXTURE_HEIGHT "_string_2(TEXTURE_HEIGHT)"                             \n"
    "layout(location = 0) out vec4 target;                                          \n"
    "layout(location = 0) in vec2 uv;                                               \n"
    "layout(binding = 0) uniform sampler2D screen;                                  \n"
    "uniform float curtime;                                                         \n"
    "float rand(vec2 uv)                                                            \n"
    "{                                                                              \n"
    "   float t = fract(curtime);                                                   \n"
    "   return fract(sin(dot(uv + 0.07 * t, vec2(12.9898, 78.233))) * 43758.5453);  \n"
    "}                                                                              \n"
    "void main(void)                                                                \n"
    "{                                                                              \n"
    "   const int num_step = 16;                                                    \n"
    "   const vec2 texel_size = 1.0 / vec2(TEXTURE_WIDTH, TEXTURE_HEIGHT);          \n"
    "   vec4 abb = vec4(0.0, 0.0, 0.0, 0.0);                                        \n"
    "   vec4 src = texture(screen, uv);                                             \n"
    "   for(int i = 1; i <= num_step; i++) {                                        \n"
    "       float shift = (float(i) / float(num_step)) * texel_size.x * 0.4;        \n"
    "       abb.r += texture(screen, vec2(uv.x + shift, uv.y)).r;                   \n"
    "       abb.b += texture(screen, vec2(uv.x - shift, uv.y)).b;                   \n"
    "   }                                                                           \n"
    "   abb /= float(num_step);                                                     \n"
    "   target = vec4(abb.r, src.g, abb.b, src.a);                                  \n"
    "   target *= 0.95;                                                             \n"
    "   target += 0.05 * rand((uv - mod(uv, texel_size * 0.4)));                    \n"
    "}                                                                              \n";

typedef unsigned char pixel_t[3];
static pixel_t pixels[TEXTURE_HEIGHT][TEXTURE_WIDTH] = { 0 };
static GLuint vao = 0, texture = 0, program = 0;
static GLint curtime_uniform = 0;
static unsigned short text_off = 0;
static unsigned short char_off = 0;
static struct lpm_cursor cursor;

static void unpack_color(uint8_t value, pixel_t c)
{
    Uint8 cv = (value & 1) ? 255 : 180;
    c[0] = ((value >> 3) & 1) ? cv : 0;
    c[1] = ((value >> 2) & 1) ? cv : 0;
    c[2] = ((value >> 1) & 1) ? cv : 0;
}

static void invert_color(pixel_t c)
{
    c[0] = (255 - c[0]);
    c[1] = (255 - c[1]);
    c[2] = (255 - c[2]);
}

static GLuint compile_shader(GLenum stage, const char *source)
{
    int i;
    char *info_log;
    GLuint shader;
    
    glGenVertexArrays(1, &vao);

    shader = glCreateShader(stage);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &i);
    if(i > 1) {
        info_log = malloc(i);
        SDL_assert(("Out of memory!", info_log));
        glGetShaderInfoLog(shader, i, NULL, info_log);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", info_log);
        free(info_log);
    }

    glGetShaderiv(shader, GL_COMPILE_STATUS, &i);
    if(!i) {
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

void init_lpm25(struct vcpu *cpu)
{
    int status;
    float max_aniso;
    GLuint vert, frag;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    vert = compile_shader(GL_VERTEX_SHADER, vert_src);
    frag = compile_shader(GL_FRAGMENT_SHADER, frag_src);
    program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    SDL_assert(("glLinkProgram failed!", status));

    curtime_uniform = glGetUniformLocation(program, "curtime");

    text_off = 0x8000;
    char_off = 0x8A00;
    cursor.pos = 0;
    cursor.blink = 500;
    cursor.visible = 1;
    cursor.last_swap = SDL_GetTicks();

    memcpy((*cpu->memory) + char_off, charset, sizeof(charset));
}

void shutdown_lpm25()
{
    glDeleteProgram(program);
    glDeleteTextures(1, &texture);
    glDeleteVertexArrays(1, &vao);
}

void lpm25_render(const struct vcpu *cpu)
{
    Uint32 ticks;
    pixel_t bg, fg;
    unsigned int chv;
    int i, j;
    unsigned short pos, word;
    const unsigned short *chp;
    unsigned char row;
    int x, y, tx, ty;
    
    ticks = SDL_GetTicks();
    if(cursor.blink && ((ticks - cursor.last_swap) > (Uint32)cursor.blink)) {
        cursor.visible = !cursor.visible;
        cursor.last_swap = ticks;
    }

    for(i = 0; i < LPM25_HEIGHT; i++) {
        for(j = 0; j < LPM25_WIDTH; j++) {
            pos = i * LPM25_WIDTH + j;
            word = (*cpu->memory)[text_off + pos];
            chp = (*cpu->memory) + char_off + (word & 0xFF) * 2;

            unpack_color((word >> 12) & 0x0F, bg);
            unpack_color((word >> 8) & 0x0F, fg);

            if(pos == cursor.pos && cursor.visible) {
                invert_color(bg);
                invert_color(fg);
            }

            chv = (chp[0] << 16) | chp[1];
            for(y = 0; y < LPM25_CH_HEIGHT; y++) {
                row = (chv >> (32 - (LPM25_CH_WIDTH * (y + 1)))) & 0x0F;
                for(x = 0; x < LPM25_CH_WIDTH; x++) {
                    tx = x + (j * LPM25_CH_WIDTH);
                    ty = y + (i * LPM25_CH_HEIGHT);
                    memcpy(pixels[ty][tx], ((row >> (LPM25_CH_WIDTH - x - 1)) & 1) ? fg : bg, sizeof(pixel_t));
                }
            }
        }
    }

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    glActiveTexture(GL_TEXTURE0);
    glUseProgram(program);
    glBindVertexArray(vao);
    glProgramUniform1f(program, curtime_uniform, SDL_GetPerformanceCounter() / (float)SDL_GetPerformanceFrequency());
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

int lpm25_ioread(struct vcpu *cpu, unsigned short port, unsigned short *value)
{
    switch(port) {
        case LPM25_IOPORT_TEXT_OFF:
            *value = text_off;
            return 1;
        case LPM25_IOPORT_CHAR_OFF:
            *value = char_off;
            return 1;
        case LPM25_IOPORT_CUR_POS:
            *value = cursor.pos;
            return 1;
        case LPM25_IOPORT_CUR_BLINK:
            *value = cursor.blink;
            return 1;
    }

    return 0;
}

int lpm25_iowrite(struct vcpu *cpu, unsigned short port, unsigned short value)
{
    switch(port) {
        case LPM25_IOPORT_TEXT_OFF:
            text_off = value;
            return 1;
        case LPM25_IOPORT_CHAR_OFF:
            char_off = value;
            return 1;
        case LPM25_IOPORT_CUR_POS:
            cursor.pos = value;
            return 1;
        case LPM25_IOPORT_CUR_BLINK:
            cursor.blink = value;
            cursor.visible = !!value;
            return 1;
    }

    return 0;
}
