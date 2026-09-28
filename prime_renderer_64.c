//prime_renderer_64.c
#include <libdragon.h>

void beginQuads(){
    // Normal RDP rendering mode
    rdpq_set_mode_standard();
    rdpq_mode_filter(FILTER_POINT);

    // TEXTURE * SHADE
    rdpq_mode_combiner(RDPQ_COMBINER_TEX_SHADE);
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
}

float tex_w = 16;
float tex_h = 16;

void setTex(sprite_t *my_tex){
    rdpq_sprite_upload(TILE0, my_tex, NULL);

    tex_w = my_tex->width;
    tex_h = my_tex->height;
}

void renderQuad(float x, float y, float halfSize, float angle, float r, float g, float b, float a){
    float w = tex_w;
    float h = tex_h;

    float s = sinf(angle);
    float c = cosf(angle);

    float negHalf = -1 * halfSize;

    float alpha = (c * negHalf);
    float bravo = (s * negHalf);
    float charlie = (c * halfSize);
    float delta = (s * halfSize);

    float A[] = {
        x + (alpha - bravo), y + (bravo + alpha),            // X, Y
        r, g, b, a,                                          // R, G, B, A
        0, 0, 1                                              // S, T, INV_W
    };

    float B[] = {x + (charlie - bravo), y + (delta + alpha), r, g, b, a, w, 0, 1};

    float C[] = {x + (charlie - delta), y + (delta + charlie), r, g, b, a, w, h, 1};

    float D[] = {x + (alpha - delta), y + (bravo + charlie), r, g, b, a, 0, h, 1};

    // First half
    rdpq_triangle(&TRIFMT_SHADE_TEX, A, B, C);

    // Second half
    rdpq_triangle(&TRIFMT_SHADE_TEX, A, C, D);
}

void endQuads(){
    //wow, it's nothing!
}