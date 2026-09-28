#ifndef PRIME_SDL_H
#define PRIME_SDL_H
#include <stdbool.h>
#include <SDL.h>

typedef struct sprite_t {
    unsigned int texture;
    int width, height;
} sprite_t;

typedef struct {
    float stick_x, stick_y; /* normalized -1..1; stick_y is positive down */
    bool a, a_pressed;
} prime_input_t;

typedef struct {
    Uint8 *data;
    Uint32 size;
} prime_sound_t;

bool primeInit(const char *title, int logical_w, int logical_h, int scale);
void primeShutdown(void);
bool primeRunning(void);
void primeFrameBegin(void);
void primeFrameEnd(void);
float primeDT(void);
prime_input_t primeInput(void);
bool primeGamepadButton(SDL_GameControllerButton button);

sprite_t *primeLoadSprite(const char *bmp_path);
void primeFreeSprite(sprite_t *sprite);
void beginQuads(void);
void setTex(sprite_t *sprite);
void renderQuad(float x, float y, float halfSize, float angle,
                float r, float g, float b, float a);
void endQuads(void);
void primeText(int x, int y, const char *text, float r, float g, float b);

bool primeScreenshot(const char *bmp_path);
#endif
