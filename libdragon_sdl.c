#include "libdragon.h"
#include <SDL_opengl.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int TRIFMT_SHADE_TEX=1;
static surface_t screen_surface;
static sprite_t *current_sprite;
static bool active;
static SDL_AudioDeviceID sound_device;
static SDL_AudioSpec sound_spec;
static int frame_count, frame_limit;
static const char *capture_path;

typedef struct { const int16_t *samples; unsigned count,cursor; } voice_t;
static voice_t voices[16];
static void audio_mix(void *userdata,Uint8 *stream,int len) {
    (void)userdata;
    int16_t *out=(int16_t *)stream;
    int count=len/(int)sizeof(int16_t);
    for(int n=0;n<count;n++) {
        int mix=0;
        for(int ch=0;ch<16;ch++) {
            voice_t *v=&voices[ch];
            if(v->samples&&v->cursor<v->count)mix+=v->samples[v->cursor++];
        }
        if(mix>32767)mix=32767;
        if(mix< -32768)mix=-32768;
        out[n]=(int16_t)mix;
    }
}
void display_init2(resolution_t r,int depth,int buffers,int gamma,int filter, const char* name) {
    (void)depth;(void)buffers;(void)gamma;(void)filter;
    if(!primeInit(name,r.width,r.height,2)) exit(1);
    const char *limit=getenv("PRIME_TEST_FRAMES");
    frame_limit=limit?atoi(limit):0;
    capture_path=getenv("PRIME_TEST_SCREENSHOT");
    active=true;
}

void display_init(resolution_t r,int depth,int buffers,int gamma,int filter) {
    display_init2(r,depth,buffers,gamma,filter, "PrimeSDL -> Unnamed Window");
}

surface_t *display_get(void) {
    if((frame_limit>0&&frame_count>=frame_limit)||!primeRunning()) {active=false;return NULL;}
    primeFrameBegin();
    return &screen_surface;
}
void rdpq_init(void) {}
void rdpq_attach(surface_t *s,void *unused) {(void)s;(void)unused;}
void rdpq_clear(color_t c) {
    glClearColor(c.r/255.f,c.g/255.f,c.b/255.f,1.f);
    glClear(GL_COLOR_BUFFER_BIT);
}
void rdpq_detach_show(void) {
    if(capture_path && frame_limit>0 && frame_count==frame_limit-1)primeScreenshot(capture_path);
    primeFrameEnd();frame_count++;
}
void rdpq_set_mode_standard(void) {glEnable(GL_TEXTURE_2D);glEnable(GL_BLEND);}
void rdpq_mode_filter(int mode) {(void)mode;}
void rdpq_mode_combiner(int mode) {(void)mode;glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);}
void rdpq_mode_blender(int mode) {(void)mode;glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);}
void rdpq_sprite_upload(int tile,sprite_t *s,const void *params) {
    (void)tile;(void)params;current_sprite=s;
    if(s){glEnable(GL_TEXTURE_2D);glBindTexture(GL_TEXTURE_2D,s->texture);}
}
void rdpq_triangle(const int *format,const float *a,const float *b,const float *c) {
    (void)format;
    if(!current_sprite)return;
    const float *v[3]={a,b,c};
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_TRIANGLES);
    for(int i=0;i<3;i++) {
        glColor4f(v[i][2],v[i][3],v[i][4],v[i][5]);
        glTexCoord2f(v[i][6]/current_sprite->width,v[i][7]/current_sprite->height);
        glVertex2f(v[i][0],v[i][1]);
    }
    glEnd();
}
void rdpq_set_mode_fill(color_t c) {glDisable(GL_TEXTURE_2D);glColor4ub(c.r,c.g,c.b,c.a);}
void rdpq_fill_rectangle(int x0,int y0,int x1,int y1) {
    glBegin(GL_QUADS);
    glVertex2i(x0,y0);glVertex2i(x1,y0);glVertex2i(x1,y1);glVertex2i(x0,y1);
    glEnd();
}
void *rdpq_font_load_builtin(int font) {(void)font;return (void *)1;}
void rdpq_text_register_font(int id,void *font) {(void)id;(void)font;}
void rdpq_text_print(void *font,int id,int x,int y,const char *format,...) {
    (void)font;(void)id;
    char text[512];va_list args;va_start(args,format);
    vsnprintf(text,sizeof(text),format,args);va_end(args);
    primeText(x,y,text,1,1,1);
}
void rdpq_sprite_blit(sprite_t *s,int x,int y,const void *params) {
    (void)params;
    if(!s)return;
    rdpq_sprite_upload(0,s,NULL);
    glColor4f(1,1,1,1);
    glBegin(GL_QUADS);
    glTexCoord2f(0,0);glVertex2i(x,y);
    glTexCoord2f(1,0);glVertex2i(x+s->width,y);
    glTexCoord2f(1,1);glVertex2i(x+s->width,y+s->height);
    glTexCoord2f(0,1);glVertex2i(x,y+s->height);
    glEnd();
}
void dfs_init(int location) {(void)location;}
static void asset_path(char *out,size_t cap,const char *path,const char *extension) {
    const char *base=strrchr(path,'/');base=base?base+1:path;
    size_t n=strcspn(base,".");
    snprintf(out,cap,"assets/%.*s.%s",(int)n,base,extension);
}
sprite_t *sprite_load2(const char *filename) {
    png_image image={0};image.version=PNG_IMAGE_VERSION;
    if(!png_image_begin_read_from_file(&image,filename)) {
        fprintf(stderr,"sprite_load2 %s: %s\n",filename,image.message);return NULL;
    }
    image.format=PNG_FORMAT_RGBA;
    void *pixels=malloc(PNG_IMAGE_SIZE(image));
    if(!pixels||!png_image_finish_read(&image,NULL,pixels,0,NULL)) {
        fprintf(stderr,"sprite_load2 %s: %s\n",filename,image.message);
        png_image_free(&image);free(pixels);return NULL;
    }
    sprite_t *s=calloc(1,sizeof(*s));
    if(!s) {free(pixels);return NULL;}
    s->width=image.width;s->height=image.height;
    glGenTextures(1,&s->texture);glBindTexture(GL_TEXTURE_2D,s->texture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,s->width,s->height,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
    free(pixels);png_image_free(&image);
    return s;
}
sprite_t *sprite_load(const char *path) {
    char filename[256];asset_path(filename,sizeof(filename),path,"png");
    return sprite_load2(filename);
}
void joypad_init(void) {}
void joypad_poll(void) {SDL_PumpEvents();}
joypad_inputs_t joypad_get_inputs(int port) {
    (void)port;
    prime_input_t in=primeInput();
    joypad_inputs_t out={0};
    out.stick_x=in.stick_x*85.f;out.stick_y= -in.stick_y*85.f;
    const Uint8 *k=SDL_GetKeyboardState(NULL);
    out.btn.z=in.a;
    out.btn.a=k[SDL_SCANCODE_X]||k[SDL_SCANCODE_K];
    out.btn.start=k[SDL_SCANCODE_RETURN];
    out.btn.d_up=k[SDL_SCANCODE_UP]||k[SDL_SCANCODE_W];
    out.btn.d_down=k[SDL_SCANCODE_DOWN]||k[SDL_SCANCODE_S];
    /* GameController polling uses the same SDL controller as primeInput. */
    if(primeGamepadButton(SDL_CONTROLLER_BUTTON_B))out.btn.a=true;
    if(primeGamepadButton(SDL_CONTROLLER_BUTTON_START))out.btn.start=true;
    out.btn.d_up|=primeGamepadButton(SDL_CONTROLLER_BUTTON_DPAD_UP);
    out.btn.d_down|=primeGamepadButton(SDL_CONTROLLER_BUTTON_DPAD_DOWN);
    if(primeGamepadButton(SDL_CONTROLLER_BUTTON_DPAD_LEFT))out.stick_x=-85;
    if(primeGamepadButton(SDL_CONTROLLER_BUTTON_DPAD_RIGHT))out.stick_x=85;
    if(out.btn.d_up)out.stick_y=85;
    if(out.btn.d_down)out.stick_y=-85;
    return out;
}
void audio_init(int frequency,int buffers) {
    (void)buffers;
    SDL_AudioSpec want={0};
    want.freq=frequency;want.format=AUDIO_S16SYS;want.channels=2;
    want.samples=512;want.callback=audio_mix;
    sound_device=SDL_OpenAudioDevice(NULL,0,&want,&sound_spec,0);
    if(sound_device)SDL_PauseAudioDevice(sound_device,0);
    else fprintf(stderr,"audio_init: %s\n",SDL_GetError());
}
void mixer_init(int voice_count) {(void)voice_count;}
bool audio_can_write(void) {return false;} /* SDL callback mixes continuously. */
short *audio_write_begin(void) {return NULL;}
int audio_get_buffer_length(void) {return 0;}
void mixer_poll(short *buffer,int samples) {(void)buffer;(void)samples;}
void audio_write_end(void) {}
void wav64_load2(wav64_t *sound,const char *filename) {
    if(!sound)return;
    memset(sound,0,sizeof(*sound));
    if(!sound_device)return;
    SDL_AudioSpec src;Uint8 *data;Uint32 size;
    if(!SDL_LoadWAV(filename,&src,&data,&size)) {
        fprintf(stderr,"wav64_load2 %s: %s\n",filename,SDL_GetError());return;
    }
    SDL_AudioCVT cvt;
    if(SDL_BuildAudioCVT(&cvt,src.format,src.channels,src.freq,
                         sound_spec.format,sound_spec.channels,sound_spec.freq)<0) {
        SDL_FreeWAV(data);return;
    }
    cvt.buf=SDL_malloc(size*cvt.len_mult);
    if(!cvt.buf) {SDL_FreeWAV(data);return;}
    memcpy(cvt.buf,data,size);cvt.len=size;SDL_FreeWAV(data);
    if(cvt.needed && SDL_ConvertAudio(&cvt)<0) {SDL_free(cvt.buf);return;}
    sound->data=cvt.buf;sound->size=cvt.needed?(Uint32)cvt.len_cvt:size;
}
void wav64_open(wav64_t *sound,const char *path) {
    char filename[256];asset_path(filename,sizeof(filename),path,"wav");
    wav64_load2(sound,filename);
}
void wav64_play(wav64_t *sound,int channel) {
    if(!sound||!sound->data||!sound_device||channel<0||channel>=16)return;
    SDL_LockAudioDevice(sound_device);
    voices[channel]=(voice_t){(int16_t*)sound->data,sound->size/2,0};
    SDL_UnlockAudioDevice(sound_device);
}
uint64_t get_ticks(void) {return SDL_GetPerformanceCounter();}
