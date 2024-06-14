#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <toml.h>

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 600;

typedef struct {
    char scene_name[256];
    char background[256];
} Scene;

typedef struct {
    char label[256];       // 對話標籤
    char character[256];   // 說話的角色
    char text[256];        // 對話文字
    char options[2][256];  // 兩個選項文字
    char next[2][256];     // 兩個選項對應的下一個對話標籤
    char scene[256];       // 當前場景
} Dialogue;

typedef struct {
    char name[256];        // 人物名稱
    char avatar[256];      // 人物圖片
} Character;

Scene scenes[10];
Dialogue dialogues[30];
Character characters[10];
int currentDialogueIndex = 0;

void loadScenesAndDialogues(const char* filename);
int findDialogueIndex(const char* dialogueLabel);
int findCharacterIndex(const char* characterName);
SDL_Texture* renderText(SDL_Renderer* renderer, const char* message, TTF_Font* font, SDL_Color color);
void updateScene(SDL_Renderer* renderer, SDL_Texture** bgTexture, SDL_Texture** characterTexture, TTF_Font* font, SDL_Color textColor, SDL_Texture** textTexture, SDL_Texture** button1Texture, SDL_Texture** button2Texture);
bool isMouseOverButton(int x, int y, SDL_Rect buttonRect);

void loadScenesAndDialogues(const char* filename) {
    FILE* fp;
    char errbuf[200];
    toml_table_t* conf;

    fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return;
    }

    conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);

    if (!conf) {
        fprintf(stderr, "Error parsing file: %s\n", errbuf);
        return;
    }

    toml_table_t* scene_table = toml_table_in(conf, "scene");
    toml_table_t* dialogue_table = toml_table_in(conf, "dialogue");
    toml_table_t* character_table = toml_table_in(conf, "character");

    // Load scenes
    int i = 0;
    const char* key;
    toml_table_t* tab;
    while ((key = toml_key_in(scene_table, i++))) {
        tab = toml_table_in(scene_table, key);
        if (tab) {
            char* scene_name = NULL;
            char* background = NULL;
            if (toml_rtos(toml_raw_in(tab, "name"), &scene_name) == 0 && scene_name) {
                snprintf(scenes[i-1].scene_name, sizeof(scenes[i-1].scene_name), "%s", scene_name);
                free(scene_name);
            }
            if (toml_rtos(toml_raw_in(tab, "background"), &background) == 0 && background) {
                snprintf(scenes[i-1].background, sizeof(scenes[i-1].background), "%s", background);
                free(background);
            }
        }
    }

    // Load dialogues
    i = 0;
    while ((key = toml_key_in(dialogue_table, i++))) {
        tab = toml_table_in(dialogue_table, key);
        if (tab) {
            char* text = NULL;
            char* scene = NULL;
            snprintf(dialogues[i-1].label, sizeof(dialogues[i-1].label), "%s", key);
            if (toml_rtos(toml_raw_in(tab, "text"), &text) == 0 && text) {
                snprintf(dialogues[i-1].text, sizeof(dialogues[i-1].text), "%s", text);
                free(text);
            }

            char* character = NULL;
            if (toml_rtos(toml_raw_in(tab, "character"), &character) == 0 && character) {
                snprintf(dialogues[i-1].character, sizeof(dialogues[i-1].character), "%s", character);
                free(character);
            }

            if (toml_rtos(toml_raw_in(tab, "scene"), &scene) == 0 && scene) {
                snprintf(dialogues[i-1].scene, sizeof(dialogues[i-1].scene), "%s", scene);
                free(scene);
            }

            toml_array_t* opt_array = toml_array_in(tab, "options");
            for (int j = 0; j < toml_array_nelem(opt_array); j++) {
                toml_table_t* opt_tab = toml_table_at(opt_array, j);
                char* opt_text = NULL;
                char* opt_next = NULL;
                if (toml_rtos(toml_raw_in(opt_tab, "text"), &opt_text) == 0 && opt_text) {
                    snprintf(dialogues[i-1].options[j], sizeof(dialogues[i-1].options[j]), "%s", opt_text);
                    free(opt_text);
                }
                if (toml_rtos(toml_raw_in(opt_tab, "next"), &opt_next) == 0 && opt_next) {
                    snprintf(dialogues[i-1].next[j], sizeof(dialogues[i-1].next[j]), "%s", opt_next);
                    free(opt_next);
                }
            }
        }
    }

    // Load characters
    i = 0;
    while ((key = toml_key_in(character_table, i++))) {
        tab = toml_table_in(character_table, key);
        if (tab) {
            char* name = NULL;
            char* avatar = NULL;
            if (toml_rtos(toml_raw_in(tab, "name"), &name) == 0 && name) {
                snprintf(characters[i-1].name, sizeof(characters[i-1].name), "%s", name);
                free(name);
            }
            if (toml_rtos(toml_raw_in(tab, "avatar"), &avatar) == 0 && avatar) {
                snprintf(characters[i-1].avatar, sizeof(characters[i-1].avatar), "%s", avatar);
                free(avatar);
            }
        }
    }

    toml_free(conf);
}

int findDialogueIndex(const char* dialogueLabel) {
    for (int i = 0; i < 20; i++) {
        if (strcmp(dialogueLabel, dialogues[i].label) == 0) {
            return i;
        }
    }
    return -1; // 找不到對應的標籤
}

int findCharacterIndex(const char* characterName) {
    for (int i = 0; i < 10; i++) {
        if (strcmp(characterName, characters[i].name) == 0) {
            return i;
        }
    }
    return -1; // 找不到對應的角色
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

void updateScene(SDL_Renderer* renderer, SDL_Texture** bgTexture, SDL_Texture** characterTexture, TTF_Font* font, SDL_Color textColor, SDL_Texture** textTexture, SDL_Texture** button1Texture, SDL_Texture** button2Texture) {
    printf("Updating scene to dialogue: %s\n", dialogues[currentDialogueIndex].label);

    // 獲取當前場景背景
    int sceneIndex = -1;
    for (int i = 0; i < 10; i++) {
        if (strcmp(scenes[i].scene_name, dialogues[currentDialogueIndex].scene) == 0) {
            sceneIndex = i;
            break;
        }
    }

    if (sceneIndex != -1) {
        printf("Loading background image: %s\n", scenes[sceneIndex].background);
        SDL_Surface* newBgSurface = IMG_Load(scenes[sceneIndex].background);
        if (newBgSurface) {
            SDL_DestroyTexture(*bgTexture);
            *bgTexture = SDL_CreateTextureFromSurface(renderer, newBgSurface);
            SDL_FreeSurface(newBgSurface);
        } else {
            printf("Unable to load image %s! SDL_image Error: %s\n", scenes[sceneIndex].background, IMG_GetError());
        }
    } else {
        printf("Scene not found: %s\n", dialogues[currentDialogueIndex].scene);
    }

    // 獲取說話者的圖片
    int characterIndex = findCharacterIndex(dialogues[currentDialogueIndex].character);
    if (characterIndex != -1) {
        printf("Loading character image: %s\n", characters[characterIndex].avatar);
        SDL_Surface* newCharSurface = IMG_Load(characters[characterIndex].avatar);
        if (newCharSurface) {
            SDL_DestroyTexture(*characterTexture);
            *characterTexture = SDL_CreateTextureFromSurface(renderer, newCharSurface);
            SDL_FreeSurface(newCharSurface);
        } else {
            printf("Unable to load image %s! SDL_image Error: %s\n", characters[characterIndex].avatar, IMG_GetError());
        }
    } else {
        printf("Character not found: %s\n", dialogues[currentDialogueIndex].character);
    }

    // 清空舊的文字和按鈕
    SDL_DestroyTexture(*textTexture);
    SDL_DestroyTexture(*button1Texture);
    SDL_DestroyTexture(*button2Texture);

    // 更新對話文字
    *textTexture = renderText(renderer, dialogues[currentDialogueIndex].text, font, textColor);

    // 更新按鈕文字
    if (strlen(dialogues[currentDialogueIndex].options[0]) > 0) {
        *button1Texture = renderText(renderer, dialogues[currentDialogueIndex].options[0], font, textColor);
    } else {
        *button1Texture = NULL;
    }

    if (strlen(dialogues[currentDialogueIndex].options[1]) > 0) {
        *button2Texture = renderText(renderer, dialogues[currentDialogueIndex].options[1], font, textColor);
    } else {
        *button2Texture = NULL;
    }

    if (*textTexture == NULL || (strlen(dialogues[currentDialogueIndex].options[0]) > 0 && *button1Texture == NULL) || (strlen(dialogues[currentDialogueIndex].options[1]) > 0 && *button2Texture == NULL)) {
        printf("Failed to render text textures!\n");
    }
}

bool isMouseOverButton(int x, int y, SDL_Rect buttonRect) {
    return x >= buttonRect.x && x <= buttonRect.x + buttonRect.w &&
           y >= buttonRect.y && y <= buttonRect.y + buttonRect.h;
}

int main(int argc, char* argv[]) {
    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    // 窗口
    SDL_Window* window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    // 渲染
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    // 初始化SDL_image
    int imgFlags = IMG_INIT_JPG | IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return -1;
    }

    // 初始化SDL_ttf
    if (TTF_Init() == -1) {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        return -1;
    }

    // 讀取劇本
    loadScenesAndDialogues("story_CN.toml");

    // 加載初始背景圖片
    printf("Loading initial background image: %s\n", scenes[0].background);
    SDL_Surface* bgSurface = IMG_Load(scenes[0].background);
    if (bgSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", scenes[0].background, IMG_GetError());
        return -1;
    }
    SDL_Texture* bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_FreeSurface(bgSurface);

    // 加載初始角色圖片
    SDL_Surface* playerSurface = IMG_Load("player_happy.png"); // 預設角色圖片
    if (playerSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", "player_happy.png", IMG_GetError());
        return -1;
    }
    SDL_Texture* characterTexture = SDL_CreateTextureFromSurface(renderer, playerSurface);
    SDL_FreeSurface(playerSurface);

    // 字型
    TTF_Font* font = TTF_OpenFont("NotoSansTC-Regular.ttf", 28);
    if (font == NULL) {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        return -1;
    }

    // 設定顏色
    SDL_Color textColor = {255, 255, 255, 255};

    SDL_Texture* textTexture = renderText(renderer, dialogues[currentDialogueIndex].text, font, textColor);
    if (textTexture == NULL) {
        return -1;
    }

    // 按鈕文字
    SDL_Texture* button1Texture = renderText(renderer, dialogues[currentDialogueIndex].options[0], font, textColor);
    SDL_Texture* button2Texture = renderText(renderer, dialogues[currentDialogueIndex].options[1], font, textColor);

    // 按鈕位置
    SDL_Rect button1Rect = {SCREEN_WIDTH / 4 - 200, SCREEN_HEIGHT - 80, 400, 50};
    SDL_Rect button2Rect = {3 * SCREEN_WIDTH / 4 - 200, SCREEN_HEIGHT - 80, 400, 50};

    
    bool quit = false;
    SDL_Event e;

    while (!quit) {        
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                printf("Mouse Clicked at (%d, %d)\n", x, y); // 打印鼠標點擊位置
                if (isMouseOverButton(x, y, button1Rect)) {
                    printf("Button 1 Clicked\n");
                    int nextDialogueIndex = findDialogueIndex(dialogues[currentDialogueIndex].next[0]);
                    if (nextDialogueIndex != -1) {
                        currentDialogueIndex = nextDialogueIndex;
                        printf("Switching to dialogue: %s\n", dialogues[currentDialogueIndex].label);
                        updateScene(renderer, &bgTexture, &characterTexture, font, textColor, &textTexture, &button1Texture, &button2Texture);
                    } 
                    else {
                        printf("Next dialogue not found: %s\n", dialogues[currentDialogueIndex].next[0]);
                    }
                } 
                
                else if (isMouseOverButton(x, y, button2Rect)) {
                    printf("Button 2 Clicked\n");
                    int nextDialogueIndex = findDialogueIndex(dialogues[currentDialogueIndex].next[1]);
                    if (nextDialogueIndex != -1) {
                        currentDialogueIndex = nextDialogueIndex;
                        printf("Switching to dialogue: %s\n", dialogues[currentDialogueIndex].label);
                        updateScene(renderer, &bgTexture, &characterTexture, font, textColor, &textTexture, &button1Texture, &button2Texture);
                    } 
                    
                    else {
                        printf("Next dialogue not found: %s\n", dialogues[currentDialogueIndex].next[1]);
                    }
                }
            }
        }

        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, bgTexture, NULL, NULL);

        // 半透明黑色窗口
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128); // 黑色，50% 透明度
        SDL_Rect blackRect = { 0, SCREEN_HEIGHT - 200, SCREEN_WIDTH, 200 };
        SDL_RenderFillRect(renderer, &blackRect);

        // 人物
        SDL_Rect playerRect = { 100, SCREEN_HEIGHT - 300, 100, 150 };  // 調整人物位置
        SDL_RenderCopy(renderer, characterTexture, NULL, &playerRect);

        // 文字
        SDL_Rect textRect = { 50, SCREEN_HEIGHT - 180, SCREEN_WIDTH - 100, 100 };  // 調整文字位置和大小
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        // 按鈕背景
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // 藍色
        SDL_RenderFillRect(renderer, &button1Rect);
        SDL_RenderFillRect(renderer, &button2Rect);

        // 按鈕文字
        if (button1Texture != NULL) {
            SDL_Rect button1TextRect = {button1Rect.x + 10, button1Rect.y + 10, button1Rect.w - 20, button1Rect.h - 20};
            SDL_RenderCopy(renderer, button1Texture, NULL, &button1TextRect);
        }
        if (button2Texture != NULL) {
            SDL_Rect button2TextRect = {button2Rect.x + 10, button2Rect.y + 10, button2Rect.w - 20, button2Rect.h - 20};
            SDL_RenderCopy(renderer, button2Texture, NULL, &button2TextRect);
        }

        // 更新
        SDL_RenderPresent(renderer);
    }

    // 清理
    SDL_DestroyTexture(bgTexture);
    SDL_DestroyTexture(characterTexture);
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
