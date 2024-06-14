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

const char* messages[] = {
    "我用鑰匙打開了地下寶庫的大門，裡面有寶石和金幣。",
    "我收集了一些寶藏，準備帶回家。",
    "我離開了地下寶庫。",
    "要回到圖書館探索更多祕密，須繳回全數寶藏。",
    "為了知曉更多祕密，我願意放棄一切財產。",
    "因此得到成為守護神的資格，留在古堡享盡榮華富貴。",
    "這次探險刺激又有趣，我成功帶回了寶藏。",
    "那裡有一個奇怪的東西，像是機關。",
    "機關啟動，出口的門關了起來。",
    "需要解開謎題才能開門。請問五年後60歲，五年前幾歲？",
    "我選擇觸碰機關，卻因解不開謎題被困在這裡直至死去。",
    "我成功解開謎題，大門打開，這時旁邊一道暗門也打開了。",
    "隱藏的寶庫有更多寶石和金幣，我收集好後準備帶回家。"
};
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

    // Load background textures
    SDL_Surface* bgSurface = IMG_Load("secret.jpg");
    if (bgSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", "secret.jpg", IMG_GetError());
        return -1;
    }
    SDL_Texture* bgTexture1 = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_FreeSurface(bgSurface);

    SDL_Surface* bgSurface2 = IMG_Load("secret2.jpg");
    if (bgSurface2 == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", "secret2.jpg", IMG_GetError());
        return -1;
    }
    SDL_Texture* bgTexture2 = SDL_CreateTextureFromSurface(renderer, bgSurface2);
    SDL_FreeSurface(bgSurface2);

    SDL_Surface* bgSurface3 = IMG_Load("home.jpg");
    if (bgSurface3 == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", "home.jpg", IMG_GetError());
        return -1;
    }
    SDL_Texture* bgTexture3 = SDL_CreateTextureFromSurface(renderer, bgSurface3);
    SDL_FreeSurface(bgSurface3);

    SDL_Surface* bgSurface4 = IMG_Load("library.jpg");
    if (bgSurface4 == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", "library.jpg", IMG_GetError());
        return -1;
    }
    SDL_Texture* bgTexture4 = SDL_CreateTextureFromSurface(renderer, bgSurface4);
    SDL_FreeSurface(bgSurface4);

    // Load player texture
    SDL_Surface* playerSurface = IMG_Load("player_happy.png");
    if (playerSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", "player_happy.png", IMG_GetError());
        return -1;
    }
    SDL_Texture* playerTexture = SDL_CreateTextureFromSurface(renderer, playerSurface);
    SDL_FreeSurface(playerSurface);

    // Load font
    TTF_Font* font = TTF_OpenFont("NotoSansTC-Regular.ttf", 28);
    if (font == NULL) {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        return -1;
    }

    SDL_Color textColor = {255, 255, 255, 255}; // White text color
    SDL_Color buttonTextColor = {0, 0, 0, 255}; // Black text color
    SDL_Color nameColor = {255, 255, 255, 255}; // White text color

    // Render initial text textures
    SDL_Texture* textTexture = renderText(renderer, messages[currentMessageIndex], font, textColor);
    if (textTexture == NULL) {
        return -1;
    }

    // Render initial button textures
    SDL_Texture* button1Texture = renderText(renderer, "收集寶藏", font, buttonTextColor);
    SDL_Texture* button2Texture = renderText(renderer, "離開這裡", font, buttonTextColor);
    SDL_Texture* nameTexture = renderText(renderer, "探險者", font, nameColor);

    // Define button positions
    button1Rect = (SDL_Rect){SCREEN_WIDTH / 2.5, (3 * SCREEN_HEIGHT) / 4 + 40, 120, 50};
    button2Rect = (SDL_Rect){SCREEN_WIDTH / 2.5 + 150, (3 * SCREEN_HEIGHT) / 4 + 40, 120, 50};
    SDL_Rect nameRect = {80, SCREEN_HEIGHT - 40, 60, 30};

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);

                // Check if button 1 is clicked
                if (isMouseOverButton(x, y, button1Rect)) {
                    if (currentMessageIndex == 0) {
                        currentMessageIndex = 1;
                        SDL_DestroyTexture(textTexture);
                        textTexture = renderText(renderer, messages[currentMessageIndex], font, textColor);

                        // Update button text
                        SDL_DestroyTexture(button1Texture);
                        SDL_DestroyTexture(button2Texture);
                        button1Texture = renderText(renderer, "繼續探索", font, buttonTextColor);
                        button2Texture = renderText(renderer, "直接離開", font, buttonTextColor);
                    } else if (currentMessageIndex == 2) {
                        currentMessageIndex = 3;
                        SDL_DestroyTexture(textTexture);
                        textTexture = renderText(renderer, messages[currentMessageIndex], font, textColor);

                        // Update button text
                        SDL_DestroyTexture(button1Texture);
                        SDL_DestroyTexture(button2Texture);
                        button1Texture = renderText(renderer, "回到圖書館", font, buttonTextColor);
                        button2Texture = renderText(renderer, "放棄寶藏", font, buttonTextColor);
                    }
                }

                // Check if button 2 is clicked
                if (isMouseOverButton(x, y, button2Rect)) {
                    if (currentMessageIndex == 0 || currentMessageIndex == 1) {
                        currentMessageIndex = 2;
                        SDL_DestroyTexture(textTexture);
                        textTexture = renderText(renderer, messages[currentMessageIndex], font, textColor);

                        // Update button text
                        SDL_DestroyTexture(button1Texture);
                        SDL_DestroyTexture(button2Texture);
                        button1Texture = renderText(renderer, "繼續探索", font, buttonTextColor);
                        button2Texture = renderText(renderer, "直接離開", font, buttonTextColor);
                    } else if (currentMessageIndex == 2) {
                        currentMessageIndex = 4;
                        SDL_DestroyTexture(textTexture);
                        textTexture = renderText(renderer, messages[currentMessageIndex], font, textColor);

                        // Update button text
                        SDL_DestroyTexture(button1Texture);
                        SDL_DestroyTexture(button2Texture);
                        button1Texture = renderText(renderer, "放棄財產", font, buttonTextColor);
                        button2Texture = renderText(renderer, "成為守護神", font, buttonTextColor);
                    } else if (currentMessageIndex == 3) {
                        currentMessageIndex = 0;
                        SDL_DestroyTexture(textTexture);
                        textTexture = renderText(renderer, messages[currentMessageIndex], font, textColor);

                        // Update button text
                        SDL_DestroyTexture(button1Texture);
                        SDL_DestroyTexture(button2Texture);
                        button1Texture = renderText(renderer, "收集寶藏", font, buttonTextColor);
                        button2Texture = renderText(renderer, "離開這裡", font, buttonTextColor);
                    } else if (currentMessageIndex == 4) {
                        currentMessageIndex = 5;
                        SDL_DestroyTexture(textTexture);
                        textTexture = renderText(renderer, messages[currentMessageIndex], font, textColor);

                        // Update button text
                        SDL_DestroyTexture(button1Texture);
                        SDL_DestroyTexture(button2Texture);
                        button1Texture = renderText(renderer, "留在古堡", font, buttonTextColor);
                        button2Texture = renderText(renderer, "返回圖書館", font, buttonTextColor);
                    }
                }
            }
        }

        SDL_RenderClear(renderer);

        // Render background based on current message index
        if (currentMessageIndex == 0 || currentMessageIndex == 1) {
            SDL_RenderCopy(renderer, bgTexture1, NULL, NULL);
        } else if (currentMessageIndex == 2 || currentMessageIndex == 3) {
            SDL_RenderCopy(renderer, bgTexture2, NULL, NULL);
        } else if (currentMessageIndex == 4 || currentMessageIndex == 5) {
            SDL_RenderCopy(renderer, bgTexture3, NULL, NULL);
        } else if (currentMessageIndex == 6 || currentMessageIndex == 7 || currentMessageIndex == 8) {
            SDL_RenderCopy(renderer, bgTexture4, NULL, NULL);
        }

        // Render player and UI elements
        SDL_Rect playerRect = {60, SCREEN_HEIGHT - 200, 100, 150};
        SDL_RenderCopy(renderer, playerTexture, NULL, &playerRect);

        SDL_Rect nameRect = {80, SCREEN_HEIGHT - 40, 60, 30};
        SDL_RenderCopy(renderer, nameTexture, NULL, &nameRect);

        SDL_Rect textRect = {150, SCREEN_HEIGHT - 150, 500, 60};
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        // Render buttons
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White color
        SDL_RenderFillRect(renderer, &button1Rect);
        SDL_RenderFillRect(renderer, &button2Rect);

        SDL_RenderCopy(renderer, button1Texture, NULL, &button1Rect);
        SDL_RenderCopy(renderer, button2Texture, NULL, &button2Rect);

        SDL_RenderPresent(renderer);
    }

    // Clean up resources
    SDL_DestroyTexture(bgTexture1);
    SDL_DestroyTexture(bgTexture2);
    SDL_DestroyTexture(bgTexture3);
    SDL_DestroyTexture(bgTexture4);
    SDL_DestroyTexture(playerTexture);
    SDL_DestroyTexture(button1Texture);
    SDL_DestroyTexture(button2Texture);
    SDL_DestroyTexture(nameTexture);
    SDL_DestroyTexture(textTexture);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_CloseFont(font);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}

