# PrimeSDL hello world | desktop compatibility layer

Build on Linux with SDL2, OpenGL 2.1, and libpng development packages:

```sh
make
./hello
```

The demo uses the left stick to move a spinning ship, A to play `assets/bang.wav`, and Escape to quit. Keyboard: WASD or arrow keys to move, Space/J to play the sound.

This is a focused compatibility layer for this source, not an implementation of every libdragon feature. It uses OpenGL 2.1's fixed pipeline; text is rendered with a built-in bitmap font. SDL handles the audio mixing callback.
