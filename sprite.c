#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

// emscripten
#include "emscripten.h"

// sdl
#include "SDL2/SDL.h"

// sokol
#define SOKOL_IMPL
//#define SOKOL_DEBUG
#define SOKOL_GLES3
#include "sokol_gfx.h"

// stb_image
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// macros
#define CODE(...) #__VA_ARGS__

// globals
SDL_Window* sdl_window;
SDL_GLContext gl_context;

static struct {
    sg_pipeline pip;
    sg_bindings bind;
    sg_pass_action pass_action;
} state = {
    .pass_action.colors[0] = { .action = SG_ACTION_CLEAR, .value = { 0.0f, 0.0f, 0.0f, 1.0f } }
};



//-----------------------------------------
// frame callback
//-----------------------------------------
void frame() {
    /* process events */
    
    // input();


    /* render and draw */
    
    int32_t w, h;
    SDL_GL_GetDrawableSize(sdl_window, &w, &h);

    sg_begin_default_pass(&state.pass_action, w, h);
    sg_apply_pipeline(state.pip);
    sg_apply_bindings(&state.bind);
    sg_draw(0, 6, 1);
    sg_end_pass();
    sg_commit();

    SDL_GL_SwapWindow(sdl_window);
}



//-----------------------------------------
// main entry point
//-----------------------------------------
int main(int argc, const char* argv[]) {
    
    /* setup sdl */
    
    SDL_Init(SDL_INIT_EVERYTHING);


    sdl_window = SDL_CreateWindow(
        "webgl2-wasm-sdl-sokol-sprite", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );
    assert(sdl_window);


    /* create opengles context on sdl window */

#if defined(SOKOL_GLES2)

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    
#elif defined(SOKOL_GLES3)

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    
#endif

    gl_context = SDL_GL_CreateContext(sdl_window);
    SDL_GL_MakeCurrent(sdl_window, gl_context);
    SDL_GL_SetSwapInterval(0);
    assert(gl_context);


    /* get sdl & gl infos */
    
    SDL_version sdl_compiled, sdl_linked;
    SDL_VERSION(&sdl_compiled);
    SDL_GetVersion(&sdl_linked);
    printf("SDL version (compiled): %d.%d.%d\n", sdl_compiled.major, sdl_compiled.minor, sdl_compiled.patch);
    printf("SDL version (linked): %d.%d.%d\n", sdl_linked.major, sdl_linked.minor, sdl_linked.patch);

    const GLubyte* gl_vendor = glGetString(GL_VENDOR);
    printf("GL Vendor: %s\n", gl_vendor);

    const GLubyte* gl_renderer = glGetString(GL_RENDERER);
    printf("GL Renderer: %s\n", gl_renderer);

    const GLubyte* gl_version = glGetString(GL_VERSION);
    printf("GL Version: %s\n", gl_version);

    const GLubyte* glsl_version = glGetString(GL_SHADING_LANGUAGE_VERSION);
    printf("GLSL Version: %s\n", glsl_version);


    /* setup sokol_gfx */
    
    sg_setup(&(sg_desc){0});
    assert(sg_isvalid());

    const float vertices[] = {
        // pos          // uv
        -0.5f,  0.5f,   0.0, 1.0,
         0.5f,  0.5f,   1.0, 1.0,
         0.5f, -0.5f,   1.0, 0.0,
        -0.5f, -0.5f,   0.0, 0.0,
    };
    state.bind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(vertices)
    });

    const uint16_t indices[] = {
        0, 1, 2,
        0, 2, 3
    };
    state.bind.index_buffer = sg_make_buffer(&(sg_buffer_desc){
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .data = SG_RANGE(indices),
    });

    /* load sprite image from file */
    
    int w, h, n;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *pixels = stbi_load("assets/sky.png", &w, &h, &n, STBI_rgb_alpha);
    assert(pixels);
    
    state.bind.fs_images[0] = sg_make_image(&(sg_image_desc){
        .width = w,
        .height = h,
        .data.subimage[0][0] = {
            .ptr = pixels,
            .size = (size_t)(w * h * 4),
        }
    });
    stbi_image_free(pixels);

    sg_shader shd = sg_make_shader(&(sg_shader_desc){
        .attrs = {
            [0].name = "position",
            [1].name = "texcoord0"
        },
        .fs.images[0] = {
            .name = "tex", 
            .image_type = SG_IMAGETYPE_2D
        },
        .vs.source = "#version 300 es\n" CODE(
            precision mediump float;
            in vec2 position;
            in vec2 texcoord0;
            out vec2 uv;
            void main(){
                gl_Position = vec4(position, 0.0, 1.0);
                uv = texcoord0;
            }
        ),
        .fs.source = "#version 300 es\n" CODE(
            precision mediump float;
            uniform sampler2D tex;
            in vec2 uv;
            out vec4 frag_color;
            void main(){
                frag_color = texture(tex, uv);
            }
        )
    });

    state.pip = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = shd,
        .primitive_type = SG_PRIMITIVETYPE_TRIANGLES,
        .index_type = SG_INDEXTYPE_UINT16,
        .layout = {
            .attrs = {
                [0].format = SG_VERTEXFORMAT_FLOAT2,
                [1].format = SG_VERTEXFORMAT_FLOAT2,
            }
        },        
        .depth = {
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
            .write_enabled = true
        },
        .cull_mode = SG_CULLMODE_BACK,
    });


    /* main frame loop */
    
#ifdef __EMSCRIPTEN__

    emscripten_set_main_loop(frame, 0, 1);
    
#else

    while(true)
        frame();
        
#endif


    sg_shutdown();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();

    return 0;
}