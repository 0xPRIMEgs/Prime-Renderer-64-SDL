//msys2

//pacman -Syu
//pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-SDL2 mingw-w64-ucrt-x86_64-libpng mingw-w64-ucrt-x86_64-pkgconf

//cd /c/Users/<win11 account folder>/Downloads/PrimeSDL_Hello/PrimeSDL_Hello

/* 
  gcc -O2 -std=c11 -I. $(pkg-config --cflags sdl2 libpng) \
  -o hello3.exe hello3.c prime_sdl.c libdragon_sdl.c \
  $(pkg-config --libs sdl2 libpng) -lopengl32 -lm
  ./hello3.exe
*/

//cp /ucrt64/bin/SDL2.dll /ucrt64/bin/libpng16-16.dll /ucrt64/bin/zlib1.dll .

#include <libdragon.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "prime_renderer_64.c"
#include "prime_fps_64.c"

#define W 320
#define H 240

sprite_t *ship;
wav64_t bang;

int scr_border = 8;

typedef struct{
    float x;
    float y;
    float angle;
    float speed;
} actor;

actor player = { .x = 128, .y = 72, .angle = 0, .speed = 1 };


joypad_inputs_t input={0}, previous={0};

void clampActor(actor *ac){
    if(ac->x < scr_border) ac->x = scr_border;
    if(ac->x > W - scr_border) ac->x = W - scr_border;
    if(ac->y < scr_border) ac->y = scr_border;
    if(ac->y > H - scr_border) ac->y = H - scr_border;
}

bool running = true;
int main(int argc,char **argv) {
    display_init2((resolution_t){W,H,false},DEPTH_16_BPP,3,GAMMA_NONE,FILTERS_RESAMPLE, "Prime SDL -> Hello 3");
    rdpq_init();dfs_init(DFS_DEFAULT_LOCATION);joypad_init();
    audio_init(44100,4);mixer_init(16);
    rdpq_text_register_font(1,rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_VAR));

    ship = sprite_load2("assets/ship.png");
    bang; wav64_load2(&bang,"assets/bang.wav");

    if(!ship||!bang.data) { fprintf(stderr,"Missing demo assets\n"); primeShutdown(); return 1; }

    initFPS();

    while(running) {
        updateFPS();
        float dt = getDT(); if(dt > 0.05f) dt = 0.05f;

        surface_t *screen = display_get(); if(!screen) break;

        rdpq_attach(screen,NULL);
        rdpq_clear((color_t){7,15,34,255});

        beginQuads();
            setTex(ship);
            renderQuad(player.x, player.y, 8, player.angle, 0.5, 0.5, 0.5, 1);
        endQuads();

        rdpq_text_print(NULL,1,8,8,"HELLO 3 | PNG AND WAV | B | JOHN 3:16");
        rdpq_text_print(NULL,1,8,18,"LEFT STICK MOVE  A BANG");
        rdpq_text_print(NULL,1,8,132,"ESC QUIT");

        rdpq_detach_show();


        joypad_poll();
        input = joypad_get_inputs(JOYPAD_PORT_1);

        player.x += input.stick_x * player.speed * dt;
        player.y -= input.stick_y * player.speed * dt;
        player.angle += 1.8f * dt;
        clampActor(&player);

        if(input.btn.z&&!previous.btn.z) wav64_play(&bang,4);

        previous = input;
    }

    primeFreeSprite(ship);
    primeShutdown();

    return 0;
}
