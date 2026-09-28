#ifndef PRIME_LIBDRAGON_SDL_H
#define PRIME_LIBDRAGON_SDL_H
/* A focused libdragon-compatible surface for XenTank's current source.
   RDPQ triangle positions/texture coordinates and joypad axes match its calls. */
#include "prime_sdl.h"
#include <stdint.h>
#include <stdarg.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct { int width,height; bool interlaced; } resolution_t;
typedef struct { int unused; } surface_t;
typedef struct { unsigned char r,g,b,a; } color_t;
typedef prime_sound_t wav64_t;
typedef struct {
    float stick_x,stick_y; /* approximately -85..85; up is positive */
    struct { bool z,start,a,d_up,d_down; } btn;
} joypad_inputs_t;

enum { DEPTH_16_BPP=0,GAMMA_NONE=0,FILTERS_RESAMPLE=0,
       DFS_DEFAULT_LOCATION=0,JOYPAD_PORT_1=0,FONT_BUILTIN_DEBUG_VAR=0,
       FILTER_POINT=0,RDPQ_COMBINER_TEX_SHADE=0,RDPQ_BLENDER_MULTIPLY=0,TILE0=0 };
extern const int TRIFMT_SHADE_TEX;

void display_init(resolution_t resolution,int depth,int buffers,int gamma,int filter);
void display_init2(resolution_t r,int depth,int buffers,int gamma,int filter, const char* name);
surface_t *display_get(void);
void rdpq_init(void);
void rdpq_attach(surface_t *surface,void *unused);
void rdpq_clear(color_t color);
void rdpq_detach_show(void);
void rdpq_set_mode_standard(void);
void rdpq_mode_filter(int mode);
void rdpq_mode_combiner(int mode);
void rdpq_mode_blender(int mode);
void rdpq_sprite_upload(int tile,sprite_t *sprite,const void *params);
void rdpq_triangle(const int *format,const float *a,const float *b,const float *c);
void rdpq_set_mode_fill(color_t color);
void rdpq_fill_rectangle(int x0,int y0,int x1,int y1);
void *rdpq_font_load_builtin(int font);
void rdpq_text_register_font(int id,void *font);
void rdpq_text_print(void *font,int font_id,int x,int y,const char *format,...);
void rdpq_sprite_blit(sprite_t *sprite,int x,int y,const void *params);
void dfs_init(int location);
sprite_t *sprite_load(const char *path);
sprite_t *sprite_load2(const char *png_path);
void joypad_init(void);
void joypad_poll(void);
joypad_inputs_t joypad_get_inputs(int port);
void audio_init(int frequency,int buffers);
void mixer_init(int voices);
bool audio_can_write(void);
short *audio_write_begin(void);
int audio_get_buffer_length(void);
void mixer_poll(short *buffer,int samples);
void audio_write_end(void);
void wav64_open(wav64_t *sound,const char *path);
void wav64_load2(wav64_t *sound,const char *wav_path);
void wav64_play(wav64_t *sound,int channel);
uint64_t get_ticks(void);
#define TICKS_PER_SECOND SDL_GetPerformanceFrequency()
#define TICKS_DISTANCE(a,b) ((b)-(a))
#endif
