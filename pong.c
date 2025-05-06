#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>
#include <SDL2/SDL_image.h>
#include <stdlib.h>
#include <time.h>


#define WINDOW_TITLE "Pong"
#define WINDOW_HEIGHT 720
#define WINDOW_WIDTH 1280
#define IMAGE_FLAGS IMG_INIT_PNG
#define TEXT_SIZE 50
#define MIXER_FLAGS MIX_INIT_OGG



struct Pong
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *background;
    SDL_Texture *text_image;
    TTF_Font *text_font;
    SDL_Color text_color;
    SDL_Rect text_rect;
    int text_xvel;
    int text_yvel;
    SDL_Texture *sprite_image;
    SDL_Texture *sprite2_image;
    SDL_Texture *ball_image;
    SDL_Rect sprite_rect;
    SDL_Rect sprite2_rect;
    SDL_Rect ball_rect;
    int sprite_vel;
    int ball_xvel;
    int ball_yvel;
    const Uint8 *keystate;
    const Uint8 *keystate2;
    Mix_Chunk *c_sound;
    Mix_Chunk *sdl_sound;
    Mix_Music *music;
};

void cleanup(struct Pong *pong, int exit_status);
bool media(struct Pong *pong);
bool sdl_initialize(struct Pong *pong);
void text_move(struct Pong *pong);
void sprite_move(struct Pong *pong);
void sprite2_move (struct Pong *pong);
void ball_move(struct Pong *pong);



int main( int argc, char* args[]){

    struct Pong pong = {

        .window = NULL,
        .renderer = NULL,
        .background = NULL,
        .text_font = NULL,
        .text_color = {255, 255, 255, 255},
        .text_rect = {0, 0, 0, 0},
        .text_image = NULL,
        .text_xvel = 4,
        .text_yvel = 4,
        .sprite_image = NULL,
        .sprite2_image = NULL,
        .ball_image = NULL,
        .sprite_rect = {0, 285, 0, 0},
        .sprite2_rect = {1150,285,0,0},
        .ball_rect = {77,325,0,0},
        .ball_xvel = 5,
        .ball_yvel = 5,
        .sprite_vel = 15,
        .keystate = SDL_GetKeyboardState(NULL),
        .keystate2 = SDL_GetKeyboardState(NULL),
        .c_sound = NULL,
        .sdl_sound = NULL,
        .music = NULL

    };


    if (sdl_initialize(&pong)){
        cleanup(&pong, EXIT_FAILURE);
    }

    if (media(&pong)){
        cleanup(&pong, EXIT_FAILURE);
    }

    if (Mix_PlayMusic(pong.music, -1)){
        fprintf(stderr, "Error Playing Music: %s\n", Mix_GetError());
        return true;
    }

    while(true){    
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            switch (event.type){
            case SDL_QUIT:
                cleanup(&pong, EXIT_SUCCESS);
                break;
            case SDL_KEYDOWN:
                switch(event.key.keysym.scancode){
                case SDL_SCANCODE_ESCAPE:
                    cleanup(&pong, EXIT_SUCCESS);
                    break;
                case SDL_SCANCODE_M:
                    if(Mix_PausedMusic()){
                        Mix_ResumeMusic();
                    }else {
                        Mix_PausedMusic();
                    }
                    break;
                default:
                    break;
                }
            default:
                break;
            }
        }

        // text_move(&pong);
        ball_move(&pong);
        sprite2_move(&pong);
        sprite_move(&pong);

        SDL_RenderClear(pong.renderer);
        SDL_RenderCopy(pong.renderer, pong.background, NULL, NULL);
        SDL_RenderCopy(pong.renderer, pong.text_image, NULL, &pong.text_rect);
        SDL_RenderCopy(pong.renderer, pong.sprite_image, NULL, &pong.sprite_rect);
        SDL_RenderCopy(pong.renderer, pong.sprite2_image, NULL, &pong.sprite2_rect);
        SDL_RenderCopy(pong.renderer, pong.ball_image, NULL, &pong.ball_rect);
        SDL_RenderPresent(pong.renderer);
        SDL_Delay(16);
    }

    cleanup(&pong, EXIT_SUCCESS);




    return 0;
}

void cleanup(struct Pong *pong, int exit_status) {

    Mix_HaltMusic();
    Mix_HaltChannel(-1);
    Mix_FreeMusic(pong->music);
    Mix_FreeChunk(pong->sdl_sound);
    Mix_FreeChunk(pong->c_sound);
    Mix_CloseAudio();
    SDL_DestroyTexture(pong->ball_image);
    SDL_DestroyTexture(pong->sprite2_image);
    SDL_DestroyTexture(pong->sprite_image);
    SDL_DestroyTexture(pong->text_image);
    TTF_CloseFont(pong->text_font);
    SDL_DestroyTexture(pong->background);
    SDL_DestroyRenderer(pong->renderer);
    SDL_DestroyWindow(pong->window);

    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    exit(exit_status);
}

bool sdl_initialize(struct Pong *pong) {
    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
        return true;
    }

    int img_init = IMG_Init(IMAGE_FLAGS);
    if ((img_init & IMAGE_FLAGS) != IMAGE_FLAGS){
        fprintf(stderr, "Error initializing SDL_image: %s\n", IMG_GetError());
        return true;
    }

    if (TTF_Init()) {
        fprintf(stderr, "Error initializing SDL_ttf: %s\n", IMG_GetError());
        return true;
    }

    int mix_init = Mix_Init(MIXER_FLAGS);
    if ((mix_init & MIXER_FLAGS) != MIXER_FLAGS) {
        fprintf(stderr, "Error initializing SDL_mixer: %s\n", Mix_GetError());
        return true;
    }

    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT,
        MIX_DEFAULT_CHANNELS, 1024)) {
        fprintf(stderr, "Error Opening Audio: %s\n", Mix_GetError());
        return true;
        }


    pong->window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH,
                                    WINDOW_HEIGHT, 0);
    if (!pong->window) {
        fprintf(stderr, "Error creating window: %s\n", SDL_GetError());
        return true;
    }

    pong->renderer = SDL_CreateRenderer(pong->window, -1, 0);
    if (!pong->renderer) {
        fprintf(stderr, "Error creating renderer: %s\n", SDL_GetError());
        return true;
    }

    SDL_Surface *icon_surf = IMG_Load("images/pong.png");
    if (!icon_surf) {
        fprintf(stderr, "Error loading Surface: %s\n", IMG_GetError());
        return true;
    }

    SDL_SetWindowIcon(pong->window, icon_surf);
    SDL_FreeSurface(icon_surf);
    srand((unsigned)time(NULL));

    return false;
}

bool media(struct Pong *pong) 
{
    pong->background = IMG_LoadTexture(pong->renderer, "images/final.png");
    if(!pong->background){
        fprintf(stderr, "Error creating Texture: %s\n", IMG_GetError());
        return true;
    }

    pong->text_font = TTF_OpenFont("fonts/freesansbold.ttf", TEXT_SIZE);
    if(!pong->text_font){
        fprintf(stderr, "Error creating Font: %s\n", TTF_GetError());
        return true;
    }

    SDL_Surface *surface = TTF_RenderText_Blended(pong->text_font, "Score: ", pong->text_color);
    if (!surface) {
        fprintf(stderr, "Error creating Surface: %s\n", SDL_GetError());
        return true;
    }

    pong->text_rect.w = surface->w;
    pong->text_rect.h = surface->h;

    pong->text_image = SDL_CreateTextureFromSurface(pong->renderer, surface);
    SDL_FreeSurface(surface);
    if (!pong->text_image) {
        fprintf(stderr, "Error creating Texture: %s\n", SDL_GetError());
        return true;
    }

    pong->sprite_image = IMG_LoadTexture(pong->renderer, "images/bigbar.png");
    if (!pong->sprite_image) {
        fprintf(stderr, "Error creating Texture: %s\n", IMG_GetError());
        return true;
    }

    if (SDL_QueryTexture(pong->sprite_image, NULL, NULL, &pong->sprite_rect.w, &pong->sprite_rect.h))
    {
        fprintf(stderr, "Error querying Texture: %s\n", SDL_GetError());
        return true;
    }

    pong->sprite2_image = IMG_LoadTexture(pong->renderer, "images/bigbar.png");
    if (!pong->sprite2_image) {
        fprintf(stderr, "Error creating Texture: %s\n", IMG_GetError());
        return true;
    }

    if (SDL_QueryTexture(pong->sprite2_image, NULL, NULL, &pong->sprite2_rect.w, &pong->sprite2_rect.h))
    {
        fprintf(stderr, "Error querying Texture: %s\n", SDL_GetError());
        return true;
    }

    pong->ball_image = IMG_LoadTexture(pong->renderer, "images/game.png");
    if (!pong->ball_image) {
        fprintf(stderr, "Error creating Texture: %s\n", IMG_GetError());
        return true;
    }

    if (SDL_QueryTexture(pong->ball_image, NULL, NULL, &pong->ball_rect.w, &pong->ball_rect.h))
    {
        fprintf(stderr, "Error querying Texture: %s\n", SDL_GetError());
        return true;
    }



    pong->c_sound = Mix_LoadWAV("sounds/C.ogg");
    if (!pong->c_sound) {
        fprintf(stderr, "Error loading Chunk: %s\n", Mix_GetError());
        return true;
    }

    pong->sdl_sound = Mix_LoadWAV("sounds/SDL.ogg");
    if (!pong->sdl_sound) {
        fprintf(stderr, "Error loading Chunk: %s\n", Mix_GetError());
        return true;
    }

    pong->music = Mix_LoadMUS("music/monster.ogg");
    if (!pong->music){
        fprintf(stderr, "Error loading Music: %s\n", Mix_GetError());
        return true;
    }

    return false;
}



void sprite_move (struct Pong *pong){


    if((pong->keystate[SDL_SCANCODE_DOWN] || pong->keystate[SDL_SCANCODE_S]) && (pong->sprite_rect.y + pong->sprite_rect.h < WINDOW_HEIGHT)){
        pong->sprite_rect.y += pong->sprite_vel;
    }


    if((pong->keystate[SDL_SCANCODE_UP] || pong->keystate[SDL_SCANCODE_W]) && (pong->sprite_rect.y > 0) ){
        pong->sprite_rect.y -= pong->sprite_vel;
    }
}

void sprite2_move (struct Pong *pong){


    if((pong->keystate2[SDL_SCANCODE_H] || pong->keystate2[SDL_SCANCODE_K]) && (pong->sprite2_rect.y + pong->sprite2_rect.h < WINDOW_HEIGHT)){
        pong->sprite2_rect.y += pong->sprite_vel;
    }


    if((pong->keystate2[SDL_SCANCODE_Y] || pong->keystate2[SDL_SCANCODE_I]) && (pong->sprite2_rect.y > 0) ){
        pong->sprite2_rect.y -= pong->sprite_vel;
    }
}

// void ball_move(struct Pong *pong) {
    
//     if (pong->ball_rect.y + pong->ball_rect.h > WINDOW_HEIGHT) {
//         pong->ball_yvel = -3;
//     }
//     if (pong->ball_rect.y < 0) {
//         pong->ball_yvel = 3;
//     }
    

//     if (SDL_HasIntersection(&pong->ball_rect, &pong->sprite_rect) == SDL_TRUE) {

//         pong->ball_xvel = 3;
        
       
//         float relative_intersect_y = (pong->sprite_rect.y + (pong->sprite_rect.h / 2)) - (pong->ball_rect.y + (pong->ball_rect.h / 2));
//         float normalized_y = relative_intersect_y / (pong->sprite_rect.h / 2);
//         pong->ball_yvel = normalized_y * 3; 
//     }
    

//     if (SDL_HasIntersection(&pong->ball_rect, &pong->sprite2_rect) == SDL_TRUE) {
 
//         pong->ball_xvel = -3;
        
  
//         float relative_intersect_y = (pong->sprite2_rect.y + (pong->sprite2_rect.h / 2)) - (pong->ball_rect.y + (pong->ball_rect.h / 2));
//         float normalized_y = relative_intersect_y / (pong->sprite2_rect.h / 2);
//         pong->ball_yvel = normalized_y * 3; 
//     }
    

//     pong->ball_rect.x += pong->ball_xvel;
//     pong->ball_rect.y += pong->ball_yvel;
// }


void ball_move(struct Pong *pong) {
    
    pong->ball_rect.x += pong->ball_xvel;
    pong->ball_rect.y += pong->ball_yvel;

    // Bounce off top and bottom walls
    if (pong->ball_rect.y + pong->ball_rect.h > WINDOW_HEIGHT || pong->ball_rect.y < 0) {
        pong->ball_yvel = -pong->ball_yvel;
    }

    // Bounce off left paddle
    if (SDL_HasIntersection(&pong->ball_rect, &pong->sprite_rect)) {
        pong->ball_xvel = abs(pong->ball_xvel);  
        
    }

    // Bounce off right paddle
    if (SDL_HasIntersection(&pong->ball_rect, &pong->sprite2_rect)) {
        pong->ball_xvel = -abs(pong->ball_xvel); 
       
    }

    // Optional: Reset ball if it goes off left or right screen
    if (pong->ball_rect.x < 0 || pong->ball_rect.x > WINDOW_WIDTH) {
        pong->ball_rect.x = WINDOW_WIDTH / 2;
        pong->ball_rect.y = WINDOW_HEIGHT / 2;
        pong->ball_xvel = (rand() % 2 == 0) ? 10 : -10;
        pong->ball_yvel = (rand() % 2 == 0) ? 10 : -10;
    }
}
