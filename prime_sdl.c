#include "prime_sdl.h"
#include <SDL_opengl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static SDL_Window *window;
static SDL_GLContext gl_context;
static SDL_GameController *gamepad;
static SDL_Joystick *rawpad;
static int logical_w, logical_h;
static bool running;
static Uint64 previous_counter;
static float dt;
static bool was_a;
static Uint32 frame_start;

static void open_gamepad(void) {
    if(gamepad || rawpad) return;
    for(int i=0;i<SDL_NumJoysticks();i++) {
        if(SDL_IsGameController(i)) {
            gamepad=SDL_GameControllerOpen(i);
            if(gamepad) return;
        }
    }
    if(SDL_NumJoysticks()>0) rawpad=SDL_JoystickOpen(0);
}
static void close_gamepad(void) {
    if(gamepad) SDL_GameControllerClose(gamepad);
    if(rawpad) SDL_JoystickClose(rawpad);
    gamepad=NULL; rawpad=NULL;
}
static void viewport(void) {
    int w,h;
    SDL_GL_GetDrawableSize(window,&w,&h);
    float target=(float)logical_w/logical_h;
    int vw=w,vh=(int)(w/target);
    if(vh>h) {vh=h;vw=(int)(h*target);}
    glViewport((w-vw)/2,(h-vh)/2,vw,vh);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,logical_w,logical_h,0,-1,1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
bool primeInit(const char *title,int width,int height,int scale) {
    if(width<1||height<1||scale<1) return false;
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_GAMECONTROLLER|SDL_INIT_JOYSTICK)<0) {
        fprintf(stderr,"SDL_Init: %s\n",SDL_GetError()); return false;
    }
    logical_w=width;logical_h=height;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
    window=SDL_CreateWindow(title,SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
                            width*scale,height*scale,SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    if(!window) {fprintf(stderr,"SDL_CreateWindow: %s\n",SDL_GetError());primeShutdown();return false;}
    gl_context=SDL_GL_CreateContext(window);
    if(!gl_context) {fprintf(stderr,"SDL_GL_CreateContext: %s\n",SDL_GetError());primeShutdown();return false;}
    SDL_GL_SetSwapInterval(1);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    viewport();
    open_gamepad();
    previous_counter=SDL_GetPerformanceCounter();
    dt=1.0f/60.0f;
    running=true;
    return true;
}
void primeShutdown(void) {
    close_gamepad();
    if(gl_context) SDL_GL_DeleteContext(gl_context);
    gl_context=NULL;
    if(window) SDL_DestroyWindow(window);
    window=NULL;
    SDL_Quit();
    running=false;
}
bool primeRunning(void) {
    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        if(e.type==SDL_QUIT) running=false;
        if(e.type==SDL_KEYDOWN && e.key.keysym.sym==SDLK_ESCAPE) running=false;
        if(e.type==SDL_WINDOWEVENT && (e.window.event==SDL_WINDOWEVENT_RESIZED ||
                                        e.window.event==SDL_WINDOWEVENT_SIZE_CHANGED)) viewport();
        if(e.type==SDL_CONTROLLERDEVICEADDED && !gamepad) {
            close_gamepad();open_gamepad();
        }
        if(e.type==SDL_CONTROLLERDEVICEREMOVED && gamepad &&
           SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gamepad))==e.cdevice.which) {
            close_gamepad();open_gamepad();
        }
        if(e.type==SDL_JOYDEVICEREMOVED && rawpad &&
           SDL_JoystickInstanceID(rawpad)==e.jdevice.which) {
            close_gamepad();open_gamepad();
        }
    }
    return running;
}
void primeFrameBegin(void) {
    Uint64 now=SDL_GetPerformanceCounter();
    dt=(float)((double)(now-previous_counter)/SDL_GetPerformanceFrequency());
    previous_counter=now;
    if(dt>0.05f) dt=0.05f;
    frame_start=SDL_GetTicks();
    glClearColor(0.035f,0.055f,0.12f,1);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
}
void primeFrameEnd(void) {
    SDL_GL_SwapWindow(window);
    Uint32 elapsed=SDL_GetTicks()-frame_start;
    if(elapsed<16) SDL_Delay(16-elapsed);
}
float primeDT(void) {return dt;}
prime_input_t primeInput(void) {
    prime_input_t out={0};
    const Uint8 *k=SDL_GetKeyboardState(NULL);
    out.stick_x=(float)((k[SDL_SCANCODE_D]||k[SDL_SCANCODE_RIGHT])-
                        (k[SDL_SCANCODE_A]||k[SDL_SCANCODE_LEFT]));
    out.stick_y=(float)((k[SDL_SCANCODE_S]||k[SDL_SCANCODE_DOWN])-
                        (k[SDL_SCANCODE_W]||k[SDL_SCANCODE_UP]));
    out.a=k[SDL_SCANCODE_SPACE]||k[SDL_SCANCODE_J];
    if(gamepad) {
        float x=SDL_GameControllerGetAxis(gamepad,SDL_CONTROLLER_AXIS_LEFTX)/32768.0f;
        float y=SDL_GameControllerGetAxis(gamepad,SDL_CONTROLLER_AXIS_LEFTY)/32768.0f;
        if(fabsf(x)>0.18f || fabsf(y)>0.18f) {out.stick_x=x;out.stick_y=y;}
        out.a|=SDL_GameControllerGetButton(gamepad,SDL_CONTROLLER_BUTTON_A);
    } else if(rawpad) {
        if(SDL_JoystickNumAxes(rawpad)>=2) {
            float x=SDL_JoystickGetAxis(rawpad,0)/32768.0f;
            float y=SDL_JoystickGetAxis(rawpad,1)/32768.0f;
            if(fabsf(x)>0.18f || fabsf(y)>0.18f) {out.stick_x=x;out.stick_y=y;}
        }
        out.a|=SDL_JoystickNumButtons(rawpad)>0 && SDL_JoystickGetButton(rawpad,0);
    }
    if(fabsf(out.stick_x)<0.18f) out.stick_x=0;
    if(fabsf(out.stick_y)<0.18f) out.stick_y=0;
    out.a_pressed=out.a && !was_a;
    was_a=out.a;
    return out;
}
sprite_t *primeLoadSprite(const char *path) {
    SDL_Surface *bmp=SDL_LoadBMP(path);
    if(!bmp) {fprintf(stderr,"Sprite %s: %s\n",path,SDL_GetError());return NULL;}
    /* Magenta is transparent; the demo BMP stays editable without SDL_image. */
    SDL_SetColorKey(bmp,SDL_TRUE,SDL_MapRGB(bmp->format,255,0,255));
    SDL_Surface *rgba=SDL_ConvertSurfaceFormat(bmp,SDL_PIXELFORMAT_RGBA32,0);
    SDL_FreeSurface(bmp);
    if(!rgba) return NULL;
    sprite_t *s=calloc(1,sizeof(*s));
    if(!s) {SDL_FreeSurface(rgba);return NULL;}
    s->width=rgba->w;s->height=rgba->h;
    glGenTextures(1,&s->texture);
    glBindTexture(GL_TEXTURE_2D,s->texture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,s->width,s->height,0,GL_RGBA,GL_UNSIGNED_BYTE,rgba->pixels);
    SDL_FreeSurface(rgba);
    return s;
}
void primeFreeSprite(sprite_t *s) {
    if(!s)return;
    glDeleteTextures(1,&s->texture);free(s);
}
/* Each 3x5 bitmap is encoded row by row. Unsupported glyphs use blank. */
static const unsigned char glyph[][5]={
    /* A-Z */
    {2,5,7,5,5},{6,5,6,5,6},{3,4,4,4,3},{6,5,5,5,6},
    {7,4,6,4,7},{7,4,6,4,4},{3,4,5,5,3},{5,5,7,5,5},
    {7,2,2,2,7},{1,1,1,5,2},{5,5,6,5,5},{4,4,4,4,7},
    {5,7,7,5,5},{5,7,7,7,5},{2,5,5,5,2},{6,5,6,4,4},
    {2,5,5,7,3},{6,5,6,5,5},{3,4,2,1,6},{7,2,2,2,2},
    {5,5,5,5,7},{5,5,5,5,2},{5,5,7,7,5},{5,5,2,5,5},
    {5,5,2,2,2},{7,1,2,4,7},
    /* 0-9 */
    {7,5,5,5,7},{2,6,2,2,7},{7,1,7,4,7},{7,1,7,1,7},
    {5,5,7,1,1},{7,4,7,1,7},{7,4,7,5,7},{7,1,1,1,1},
    {7,5,7,5,7},{7,5,7,1,7}
};
void primeText(int x,int y,const char *text,float r,float g,float b) {
    glDisable(GL_TEXTURE_2D);
    glColor4f(r,g,b,1);
    for(const unsigned char *p=(const unsigned char*)text;*p;p++,x+=5) {
        unsigned char ch=*p;
        if(ch>='a'&&ch<='z')ch-=32;
        int n=ch>='A'&&ch<='Z'?ch-'A':ch>='0'&&ch<='9'?26+ch-'0':-1;
        if(n<0) {
            if(ch==':'||ch=='.'||ch=='!') {
                glBegin(GL_QUADS);
                float px=(float)x+1,py=(float)y+(ch==':'?1:4);
                glVertex2f(px,py);glVertex2f(px+1,py);
                glVertex2f(px+1,py+1);glVertex2f(px,py+1);
                if(ch==':') {py+=3;glVertex2f(px,py);glVertex2f(px+1,py);glVertex2f(px+1,py+1);glVertex2f(px,py+1);}
                glEnd();
            }
            continue;
        }
        glBegin(GL_QUADS);
        for(int row=0;row<5;row++)for(int col=0;col<3;col++) {
            if(!(glyph[n][row]&(4>>col)))continue;
            float px=(float)x+col,py=(float)y+row;
            glVertex2f(px,py);glVertex2f(px+1,py);
            glVertex2f(px+1,py+1);glVertex2f(px,py+1);
        }
        glEnd();
    }
    glEnable(GL_TEXTURE_2D);
}
bool primeScreenshot(const char *path) {
    GLint vp[4];glGetIntegerv(GL_VIEWPORT,vp);
    int w=vp[2],h=vp[3];
    if(w<1||h<1)return false;
    SDL_Surface *surface=SDL_CreateRGBSurfaceWithFormat(0,w,h,32,SDL_PIXELFORMAT_RGBA32);
    if(!surface)return false;
    glPixelStorei(GL_PACK_ALIGNMENT,1);
    glReadPixels(vp[0],vp[1],w,h,GL_RGBA,GL_UNSIGNED_BYTE,surface->pixels);
    Uint8 *top=surface->pixels;
    Uint8 *row=malloc(surface->pitch);
    if(!row) {SDL_FreeSurface(surface);return false;}
    for(int y=0;y<h/2;y++) {
        Uint8 *bottom=top+(h-y-1)*surface->pitch;
        memcpy(row,top+y*surface->pitch,surface->pitch);
        memcpy(top+y*surface->pitch,bottom,surface->pitch);
        memcpy(bottom,row,surface->pitch);
    }
    free(row);
    bool ok=SDL_SaveBMP(surface,path)==0;
    SDL_FreeSurface(surface);
    return ok;
}

bool primeGamepadButton(SDL_GameControllerButton button) {
    return gamepad && SDL_GameControllerGetButton(gamepad,button);
}
