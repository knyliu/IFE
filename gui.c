#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>


const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 600;

int main(int argc, char* argv[]) {
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

   
    SDL_Window* window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

 
    int imgFlags = IMG_INIT_JPG | IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return -1;
    }

   
    if (TTF_Init() == -1) {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        return -1;
    }

   
    SDL_Surface* bgSurface = IMG_Load("library.jpg");
    if (bgSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", "library.jpg", IMG_GetError());
        return -1;
    }
    SDL_Texture* bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_FreeSurface(bgSurface);


    SDL_Surface* playerSurface = IMG_Load("player_happy.png");
    if (playerSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", "player_happy.png", IMG_GetError());
        return -1;
    }
    SDL_Texture* playerTexture = SDL_CreateTextureFromSurface(renderer, playerSurface);
    SDL_FreeSurface(playerSurface);

    
    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 28);
    if (font == NULL) {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        return -1;
    }

    
    SDL_Color textColor = {255, 255, 255, 255};

    
    SDL_Surface* textSurface = TTF_RenderUTF8_Solid(font, "Hello", textColor);
    if (textSurface == NULL) {
        printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
        return -1;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);

    
    bool quit = false;
    SDL_Event e;

    while (!quit) {
        
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

       
        SDL_RenderClear(renderer);

        
        SDL_RenderCopy(renderer, bgTexture, NULL, NULL);

        // 半透明黑色窗口
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128); // 黑色，50% 透明度
        SDL_Rect blackRect = { 0, SCREEN_HEIGHT - 200, SCREEN_WIDTH, 200 };
        SDL_RenderFillRect(renderer, &blackRect);

        
        SDL_Rect playerRect = { 100, SCREEN_HEIGHT - 200, 100, 150 };  // 調整人物位置
        SDL_RenderCopy(renderer, playerTexture, NULL, &playerRect);

        
        SDL_Rect textRect = { 200, SCREEN_HEIGHT - 150, 100, 50 };  // 調整文字位置和大小
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        
        SDL_RenderPresent(renderer);
    }

    
    SDL_DestroyTexture(bgTexture);
    SDL_DestroyTexture(playerTexture);
    SDL_DestroyTexture(textTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}

