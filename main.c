#include <stdlib.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <julia/julia.h>
#include "microui.h"
#include "extra.h"

static SDL_Window *win;
static SDL_Renderer *rend;
static TTF_Font *font;
static mu_Context *ctx;
static int fontsize = 18;

static int
textwidth(mu_Font f, const char *text, int len)
{
    return len;
}

static int
textheight(mu_Font f)
{
    return fontsize;
}

SDL_AppResult
SDL_AppInit(void **state, int argc, char **argv)
{
    SDL_SetAppMetadata("game title","0.1","game");

    if(SDL_Init(SDL_INIT_VIDEO) == 0){
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if(TTF_Init() == 0){
        SDL_Log("TTF_Init failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if(SDL_CreateWindowAndRenderer("game title", 800, 600, 0, &win, &rend) == 0){
        SDL_Log("SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    font = TTF_OpenFont("font.ttf", fontsize);
    if(font == 0){
        SDL_Log("TTF_OpenFont failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    /* microui */
    ctx = (mu_Context*)malloc(sizeof(mu_Context));
    mu_init(ctx);
    ctx->text_width = textwidth;
    ctx->text_height = textheight;

    return SDL_APP_CONTINUE;
}

static const char btnmap[256] = {
    [SDL_BUTTON_LEFT & 0xff] = MU_MOUSE_LEFT,
    [SDL_BUTTON_RIGHT & 0xff] = MU_MOUSE_RIGHT,
    [SDL_BUTTON_MIDDLE & 0xff] = MU_MOUSE_MIDDLE
};

static const char keymap[256] = {
    [SDLK_LSHIFT & 0xff] = MU_KEY_SHIFT,
    [SDLK_RSHIFT & 0xff] = MU_KEY_SHIFT,
    [SDLK_LCTRL & 0xff] = MU_KEY_CTRL,
    [SDLK_RCTRL & 0xff] = MU_KEY_CTRL,
    [SDLK_LALT & 0xff] = MU_KEY_ALT,
    [SDLK_RALT & 0xff] = MU_KEY_ALT,
    [SDLK_RETURN & 0xff] = MU_KEY_RETURN,
    [SDLK_BACKSPACE & 0xff] = MU_KEY_BACKSPACE
};

SDL_AppResult
SDL_AppEvent(void *state, SDL_Event *ev)
{
    switch(ev->type){
    case SDL_EVENT_MOUSE_MOTION:
        mu_input_mousemove(ctx,ev->motion.x,ev->motion.y);
    break;
    case SDL_EVENT_MOUSE_WHEEL:
        mu_input_scroll(ctx,0,ev->wheel.y * -30);
    break;
    case SDL_EVENT_TEXT_INPUT:
        mu_input_text(ctx,ev->text.text);
    break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        int btn1 = btnmap[ev->button.button & 0xff];
        if(btn1 != 0)
            mu_input_mousedown(ctx,ev->button.x,ev->button.y,btn1);
    break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        int btn2 = btnmap[ev->button.button & 0xff];
        if(btn2 != 0)
            mu_input_mouseup(ctx,ev->button.x,ev->button.y,btn2);
    break;
    case SDL_EVENT_KEY_DOWN:
        int key1 = keymap[ev->key.key & 0xff];
        if(key1 != 0)
            mu_input_keydown(ctx,key1);
    break;
    case SDL_EVENT_KEY_UP:
        int key2 = keymap[ev->key.key & 0xff];
        if(key2 != 0)
            mu_input_keyup(ctx,key2);
    break;
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}

void
drawtext(const char *text, mu_Vec2 pos, mu_Color col)
{
    SDL_Color col2 = { col.r, col.g, col.b };
    SDL_Surface *surf = TTF_RenderText_Solid(font,text,strlen(text),col2);
    SDL_Texture *tex = SDL_CreateTextureFromSurface(rend,surf);
    SDL_DestroySurface(surf);
    SDL_FRect rect = { pos.x, pos.y, 50, fontsize };
    SDL_RenderTexture(rend,tex,0,&rect);
    SDL_DestroyTexture(tex);
}

void
drawrect(mu_Rect rect, mu_Color col)
{
    SDL_SetRenderDrawColor(rend,col.r,col.g,col.b,255);
    SDL_FRect rect2 = { rect.x,rect.y, rect.w, rect.h };
    SDL_RenderFillRect(rend,&rect2);
}

void
drawicon(int id, mu_Rect rect, mu_Color col)
{
    // ??
}

void
setclip(mu_Rect rect)
{
    SDL_Rect rect2 = { rect.x,rect.y, rect.w, rect.h };
    SDL_SetRenderClipRect(rend,&rect2);
}

SDL_AppResult
SDL_AppIterate(void *state)
{
    SDL_SetRenderDrawColor(rend,0,0,0,255);
    SDL_RenderClear(rend);

    mu_begin(ctx);
    style_window(ctx);
    log_window(ctx);
    test_window(ctx);
    mu_end(ctx);

    mu_Command *cmd = 0;

    while(mu_next_command(ctx,&cmd)){
        switch(cmd->type){
        case MU_COMMAND_TEXT:
            drawtext(cmd->text.str, cmd->text.pos, cmd->text.color);
        break;
        case MU_COMMAND_RECT:
            drawrect(cmd->rect.rect, cmd->rect.color);
        break;
        case MU_COMMAND_ICON:
            drawicon(cmd->icon.id, cmd->icon.rect, cmd->icon.color);
        break;
        case MU_COMMAND_CLIP:
            setclip(cmd->clip.rect);
        break;
        }
    }

    SDL_RenderPresent(rend);

    return SDL_APP_CONTINUE;
}

void
SDL_AppQuit(void *state, SDL_AppResult res)
{
    TTF_CloseFont(font);
    SDL_Log("bye");
}

