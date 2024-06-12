#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;

SDL_Rect button1Rect;
SDL_Rect button2Rect;

const char* messages[] = {"我用鑰匙打開了地下寶庫的大門。",
                          "裡面光芒四射，有各種寶石和錢幣。"};
int currentMessageIndex = 0;

bool isMouseOverButton(int x, int y, SDL_Rect buttonRect) {
    return x >= buttonRect.x && x <= buttonRect.x + buttonRect.w &&
           y >= buttonRect.y && y <= buttonRect.y + buttonRect.h;
}

SDL_Texture* renderText(SDL_Renderer* renderer, const char* message, TTF_Font* font, SDL_Color color) {
    SDL_Surface* textSurface = TTF_RenderUTF8_Solid(font, message, color);
    if (textSurface == NULL) {
        printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
        return NULL;
    }
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);
    return textTexture;
}

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

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    
    int imgFlags = IMG_INIT_JPG | IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return -1;
    }

    if (TTF_Init() == -1) {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        return -1;
    }

    SDL_Surface* bgSurface = IMG_Load("secret.jpg");
    if (bgSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", "secret.jpg", IMG_GetError());
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

    // 使用相对路径加载字体文件
    TTF_Font* font = TTF_OpenFont("NotoSansTC-Regular.ttf", 28);
    if (font == NULL) {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        return -1;
    }

    SDL_Color textColor = {255, 255, 255, 255};

    SDL_Texture* textTexture = renderText(renderer, messages[currentMessageIndex], font, textColor);
    if (textTexture == NULL) {
        return -1;
    }

    SDL_Texture* button1Texture = renderText(renderer, "GO!", font, textColor);
    SDL_Texture* button2Texture = renderText(renderer, "Leave.", font, textColor);

    SDL_Rect button1Rect = {SCREEN_WIDTH - 270, SCREEN_HEIGHT - 70, 60, 30};
    SDL_Rect button2Rect = {SCREEN_WIDTH - 200, SCREEN_HEIGHT - 70, 60, 30};

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                if (isMouseOverButton(x, y, button1Rect) || isMouseOverButton(x, y, button2Rect)) {
                    currentMessageIndex = (currentMessageIndex + 1) % 2;

                    SDL_DestroyTexture(textTexture);
                    textTexture = renderText(renderer, messages[currentMessageIndex], font, textColor);
                    if (textTexture == NULL) {
                        return -1;
                    }
                }
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, bgTexture, NULL, NULL);

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
        SDL_Rect blackRect = {0, SCREEN_HEIGHT - 200, SCREEN_WIDTH, 200};
        SDL_RenderFillRect(renderer, &blackRect);

        SDL_Rect playerRect = {60, SCREEN_HEIGHT - 200, 100, 150};
        SDL_RenderCopy(renderer, playerTexture, NULL, &playerRect);

        SDL_Rect textRect = {200, SCREEN_HEIGHT - 150, 200, 60};
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        SDL_RenderCopy(renderer, button1Texture, NULL, &button1Rect);
        SDL_RenderCopy(renderer, button2Texture, NULL, &button2Rect);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(bgTexture);
    SDL_DestroyTexture(playerTexture);
    SDL_DestroyTexture(button1Texture);
    SDL_DestroyTexture(button2Texture);
    SDL_DestroyTexture(textTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}

