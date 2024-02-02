# webgl2-wasm-sdl-sokol-sprite

Minimal sprite rendering example written in C with [`SDL2`](https://github.com/libsdl-org/SDL) for windowing, [`sokol_gfx`](https://github.com/floooh/sokol) for graphics API using WebGL2/GLES3, [`stb_image`](https://github.com/nothings/stb) for loading image, compiled in WebAssembly with [Emscripten](https://emscripten.org/).

## Step by step

1. Clone the repositories `sokol` and `stb` in the `deps` folder
```bash
git clone https://github.com/floooh/sokol deps/sokol
git clone https://github.com/nothings/stb deps/stb

# else init/update submodules
git submodule update --init
```
2. Add an image (_sky.png_) in `assets` directory
3. Compile and build with the following command (need Emscripten installed):
```bash
emcc sprite.c -o sprite.html -Ideps/sokol -Ideps/stb --preload-file assets -s EXIT_RUNTIME=1 -s ALLOW_MEMORY_GROWTH=1 -s USE_WEBGL2=1 -s USE_SDL=2
```
4. Launch a web server from the output folder
5. Open `sprite.hml` page from web server 

## Dependencies

* [sdl2](https://github.com/libsdl-org/SDL) (builtin in Emscripten)
* [sokol](https://github.com/floooh/sokol) (#02b1dae)
* [stb](https://github.com/nothings/stb) (#f4a71b1)