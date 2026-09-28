//fps debug

// Globals
static uint64_t fps_last_time = 0;
static int fps_frames = 0;
static int fps_display = 0;

static uint64_t dt_last_time = 0;
static float dt = 0.0f;

// Draw one 3x5 bitmap character at 3x scale
void drawFPSChar(char c, int x, int y) {
    static const uint8_t digits[10][5] = {
        {7,5,5,5,7}, // 0
        {2,6,2,2,7}, // 1
        {7,1,7,4,7}, // 2
        {7,1,7,1,7}, // 3
        {5,5,7,1,1}, // 4
        {7,4,7,1,7}, // 5
        {7,4,7,5,7}, // 6
        {7,1,1,1,1}, // 7
        {7,5,7,5,7}, // 8
        {7,5,7,1,7}  // 9
    };

    static const uint8_t F[5] = {7,4,6,4,4};
    static const uint8_t P[5] = {7,5,7,4,4};
    static const uint8_t S[5] = {7,4,7,1,7};
    static const uint8_t COLON[5] = {0,2,0,2,0};

    const uint8_t *rows;

    if(c >= '0' && c <= '9')
        rows = digits[c - '0'];
    else if(c == 'F')
        rows = F;
    else if(c == 'P')
        rows = P;
    else if(c == 'S')
        rows = S;
    else if(c == ':')
        rows = COLON;
    else
        return;

    const int SCALE = 3;

    for(int row = 0; row < 5; row++) {
        for(int col = 0; col < 3; col++) {
            if(rows[row] & (1 << (2-col))) {
                rdpq_fill_rectangle(
                    x + col * SCALE,
                    y + row * SCALE,
                    x + (col+1) * SCALE,
                    y + (row+1) * SCALE
                );
            }
        }
    }
}

void drawFPS() {
    char text[16];
    snprintf(text, sizeof(text), "FPS:%d", fps_display);

    rdpq_set_mode_fill((color_t){255,255,255,255});

    int x = 8;

    for(int i = 0; text[i]; i++) {
        drawFPSChar(text[i], x, 8);
        x += 12;
    }
}

void initFPS(){
    uint64_t now = get_ticks();

    fps_last_time = now;
    dt_last_time = now;
}

void updateFPS(){
    fps_frames++;

    uint64_t now = get_ticks();

    // Delta time in seconds
    dt = TICKS_DISTANCE(dt_last_time, now) / (float)TICKS_PER_SECOND;
    dt_last_time = now;

    // FPS
    if (TICKS_DISTANCE(fps_last_time, now) >= TICKS_PER_SECOND / 2) {
        float elapsed =
            TICKS_DISTANCE(fps_last_time, now) / (float)TICKS_PER_SECOND;

        fps_display = (int)(fps_frames / elapsed);

        fps_frames = 0;
        fps_last_time = now;
    }
}

float getDT(){
    return dt;
}