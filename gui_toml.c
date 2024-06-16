#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <toml.h>
#include <stdint.h>

const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 700;

#define SCALE 1

typedef struct {
    char label[256];       // 對話標籤
    char character[256];   // 說話的角色
    char text[256];        // 對話文字
    char options[2][256];  // 兩個選項文字
    char next[2][256];     // 兩個選項對應的下一個對話標籤
    char background[256];  // 背景圖片
    char inventory_add[256]; // 物品新增到庫存
    int affinity_change[2]; // 好感度改變
} Dialogue;

typedef struct {
    char name[256];        // 人物名稱
    char avatar[256];      // 人物圖片
} Character;

typedef struct {
    char name[256];        // 物品名稱
    char description[256]; // 物品描述
    char icon[256];        // 物品圖標
} Item;

Dialogue dialogues[13];
Character characters[10];
Item items[10];
int currentDialogueIndex = 0;
SDL_Rect itemRect;
SDL_Texture* itemTexture = NULL;
int characterAffinity = 40; // 初始好感度

void loadDialoguesAndCharacters(const char* filename);
int findDialogueIndex(const char* dialogueLabel);
int findCharacterIndex(const char* characterName);
int findItemIndex(const char* itemName);
SDL_Texture* renderText(SDL_Renderer* renderer, const char* message, TTF_Font* font, SDL_Color color);
void updateScene(SDL_Renderer* renderer, SDL_Texture** bgTexture, SDL_Texture** characterTexture, TTF_Font* font, SDL_Color textColor, SDL_Texture** textTexture, SDL_Texture** button1Texture, SDL_Texture** button2Texture, SDL_Texture** itemTexture, SDL_Texture** affinityTexture);
bool isMouseOverButton(int x, int y, SDL_Rect buttonRect);
SDL_Texture* loadTexture(const char* path, SDL_Renderer* renderer);

void loadDialoguesAndCharacters(const char* filename) {
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

    toml_table_t* dialogue_table = toml_table_in(conf, "dialogue");
    toml_table_t* character_table = toml_table_in(conf, "character");
    toml_table_t* item_table = toml_table_in(conf, "item");

    // Load dialogues
    int i = 0;
    const char* key;
    toml_table_t* tab;
    while ((key = toml_key_in(dialogue_table, i++))) {
        tab = toml_table_in(dialogue_table, key);
        if (tab) {
            char* text = NULL;
            char* background = NULL;
            char* inventory_add = NULL;

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

            if (toml_rtos(toml_raw_in(tab, "background"), &background) == 0 && background) {
                snprintf(dialogues[i-1].background, sizeof(dialogues[i-1].background), "%s", background);
                free(background);
            }

            if (toml_rtos(toml_raw_in(tab, "inventory_add"), &inventory_add) == 0 && inventory_add) {
                snprintf(dialogues[i-1].inventory_add, sizeof(dialogues[i-1].inventory_add), "%s", inventory_add);
                free(inventory_add);
            }

            toml_array_t* opt_array = toml_array_in(tab, "options");
            for (int j = 0; j < toml_array_nelem(opt_array); j++) {
                toml_table_t* opt_tab = toml_table_at(opt_array, j);
                char* opt_text = NULL;
                char* opt_next = NULL;
                int64_t affinity_change = 0;
                if (toml_rtos(toml_raw_in(opt_tab, "text"), &opt_text) == 0 && opt_text) {
                    snprintf(dialogues[i-1].options[j], sizeof(dialogues[i-1].options[j]), "%s", opt_text);
                    free(opt_text);
                }
                if (toml_rtos(toml_raw_in(opt_tab, "next"), &opt_next) == 0 && opt_next) {
                    snprintf(dialogues[i-1].next[j], sizeof(dialogues[i-1].next[j]), "%s", opt_next);
                    free(opt_next);
                }
                if (toml_rtoi(toml_raw_in(opt_tab, "affinity_change"), &affinity_change) == 0) {
                    dialogues[i-1].affinity_change[j] = (int)affinity_change;
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

    // Load items
    i = 0;
    while ((key = toml_key_in(item_table, i++))) {
        tab = toml_table_in(item_table, key);
        if (tab) {
            char* name = NULL;
            char* description = NULL;
            char* icon = NULL;
            if (toml_rtos(toml_raw_in(tab, "name"), &name) == 0 && name) {
                snprintf(items[i-1].name, sizeof(items[i-1].name), "%s", name);
                free(name);
            }
            if (toml_rtos(toml_raw_in(tab, "description"), &description) == 0 && description) {
                snprintf(items[i-1].description, sizeof(items[i-1].description), "%s", description);
                free(description);
            }
            if (toml_rtos(toml_raw_in(tab, "icon"), &icon) == 0 && icon) {
                snprintf(items[i-1].icon, sizeof(items[i-1].icon), "%s", icon);
                free(icon);
            }
        }
    }

    toml_free(conf);
}

int findDialogueIndex(const char* dialogueLabel) {
    for (int i = 0; i < 13; i++) {
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

int findItemIndex(const char* itemName) {
    for (int i = 0; i < 10; i++) {
        if (strcmp(itemName, items[i].name) == 0) {
            return i;
        }
    }
    return -1; // 找不到對應的物品
}

SDL_Texture* renderText(SDL_Renderer* renderer, const char* message, TTF_Font* font, SDL_Color color) {
    if (strlen(message) == 0) {
        printf("Error: Text is empty!\n");
        return NULL;
    }
    SDL_Surface* textSurface = TTF_RenderUTF8_Solid(font, message, color);
    if (textSurface == NULL) {
        printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
        return NULL;
    }
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);
    return textTexture;
}

SDL_Texture* loadTexture(const char* path, SDL_Renderer* renderer) {
    SDL_Texture* newTexture = NULL;
    SDL_Surface* loadedSurface = IMG_Load(path);
    if (loadedSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
    } else {
        newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        if (newTexture == NULL) {
            printf("Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError());
        }
        SDL_FreeSurface(loadedSurface);
    }
    return newTexture;
}

void updateScene(SDL_Renderer* renderer, SDL_Texture** bgTexture, SDL_Texture** characterTexture, TTF_Font* font, SDL_Color textColor, SDL_Texture** textTexture, SDL_Texture** button1Texture, SDL_Texture** button2Texture, SDL_Texture** itemTexture, SDL_Texture** affinityTexture) {
    printf("Updating scene to dialogue: %s\n", dialogues[currentDialogueIndex].label);

    // 獲取當前對話的背景
    printf("Loading background image: %s\n", dialogues[currentDialogueIndex].background);
    SDL_Surface* newBgSurface = IMG_Load(dialogues[currentDialogueIndex].background);
    if (newBgSurface) {
        SDL_DestroyTexture(*bgTexture);
        *bgTexture = SDL_CreateTextureFromSurface(renderer, newBgSurface);
        SDL_FreeSurface(newBgSurface);
    } else {
        printf("Unable to load image %s! SDL_image Error: %s\n", dialogues[currentDialogueIndex].background, IMG_GetError());
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

    // 更新物品圖片
    if (strlen(dialogues[currentDialogueIndex].inventory_add) > 0) {
        int itemIndex = findItemIndex(dialogues[currentDialogueIndex].inventory_add);
        if (itemIndex != -1) {
            *itemTexture = loadTexture(items[itemIndex].icon, renderer);
            if (*itemTexture == NULL) {
                printf("Unable to load item image %s! SDL_image Error: %s\n", items[itemIndex].icon, IMG_GetError());
            } else {
                itemRect.x = (SCREEN_WIDTH * SCALE) / 2 - (50 * SCALE) / 2;
                itemRect.y = (SCREEN_HEIGHT * SCALE) / 2 - (50 * SCALE) / 2;
                itemRect.w = 50 * SCALE;
                itemRect.h = 50 * SCALE;
                printf("Item displayed at (%d, %d)\n", itemRect.x, itemRect.y);
            }
        }
    } else {
        *itemTexture = NULL;
    }

    // 更新好感度圖片
    if (*affinityTexture != NULL) {
        SDL_DestroyTexture(*affinityTexture);
    }
    if (characterAffinity <= 20) {
        *affinityTexture = loadTexture("20%.png", renderer);
    } else if (characterAffinity <= 40) {
        *affinityTexture = loadTexture("40%.png", renderer);
    } else if (characterAffinity <= 60) {
        *affinityTexture = loadTexture("60%.png", renderer);
    } else if (characterAffinity <= 80) {
        *affinityTexture = loadTexture("80%.png", renderer);
    } else {
        *affinityTexture = loadTexture("100%.png", renderer);
    }

    if (*affinityTexture == NULL) {
        printf("Failed to load affinity texture!\n");
    }
}

bool isMouseOverButton(int x, int y, SDL_Rect buttonRect) {
    return x >= buttonRect.x && x <= buttonRect.x + buttonRect.w &&
           y >= buttonRect.y && y <= buttonRect.y + buttonRect.h;
}

int main(void) {
    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    // 創建窗口
    SDL_Window* window = SDL_CreateWindow("SDL IFE game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    // 創建渲染器
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
    loadDialoguesAndCharacters("story_CN2.toml");

    // 確認初始背景圖片存在
    printf("Initial dialogue background: %s\n", dialogues[currentDialogueIndex].background);
    if (strlen(dialogues[currentDialogueIndex].background) == 0) {
        printf("Error: Initial background image not specified!\n");
        return -1;
    }

    // 加載初始背景圖片
    printf("Loading initial background image: %s\n", dialogues[currentDialogueIndex].background);
    SDL_Surface* bgSurface = IMG_Load(dialogues[currentDialogueIndex].background);
    if (bgSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", dialogues[currentDialogueIndex].background, IMG_GetError());
        return -1;
    }
    SDL_Texture* bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_FreeSurface(bgSurface);

    // 加載初始角色圖片
    int characterIndex = findCharacterIndex(dialogues[currentDialogueIndex].character);
    SDL_Surface* playerSurface = NULL;
    if (characterIndex != -1) {
        playerSurface = IMG_Load(characters[characterIndex].avatar); // 預設角色圖片
    } else {
        playerSurface = IMG_Load("player_happy.png"); // 預設角色圖片
    }
    if (playerSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", "player_happy.png", IMG_GetError());
        return -1;
    }
    SDL_Texture* characterTexture = SDL_CreateTextureFromSurface(renderer, playerSurface);
    SDL_FreeSurface(playerSurface);

    // 加載字型
    TTF_Font* font = TTF_OpenFont("NotoSansTC-Regular.ttf", 50 * SCALE);
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

    // 計算按鈕位置和大小
    int scaled_width = 400 * SCALE;
    int scaled_height = 50 * SCALE;
    int scaled_x1 = (SCREEN_WIDTH * SCALE / 4) - (scaled_width / 2);
    int scaled_x2 = (3 * SCREEN_WIDTH * SCALE / 4) - (scaled_width / 2);
    int scaled_y = SCREEN_HEIGHT * SCALE - 80;  // 保持原始 Y 位置不變

    SDL_Rect button1Rect = {scaled_x1, scaled_y, scaled_width, scaled_height};
    SDL_Rect button2Rect = {scaled_x2, scaled_y, scaled_width, scaled_height};

    // 初始化好感度
    SDL_Texture* affinityTexture = NULL;
    if (characterAffinity <= 20) {
        affinityTexture = loadTexture("20%.png", renderer);
    } else if (characterAffinity <= 40) {
        affinityTexture = loadTexture("40%.png", renderer);
    } else if (characterAffinity <= 60) {
        affinityTexture = loadTexture("60%.png", renderer);
    } else if (characterAffinity <= 80) {
        affinityTexture = loadTexture("80%.png", renderer);
    } else {
        affinityTexture = loadTexture("100%.png", renderer);
    }

    if (affinityTexture == NULL) {
        printf("Failed to load initial affinity texture!\n");
        return -1;
    }

    // 主要循環
    bool quit = false;
    SDL_Event e;

    while (!quit) {
        // 處理事件
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                printf("Mouse Clicked at (%d, %d)\n", x, y); // 打印鼠標點擊位置
                if (isMouseOverButton(x, y, button1Rect)) {
                    printf("Button 1 Clicked\n");
                    characterAffinity += dialogues[currentDialogueIndex].affinity_change[0];
                    int nextDialogueIndex = findDialogueIndex(dialogues[currentDialogueIndex].next[0]);
                    if (nextDialogueIndex != -1) {
                        currentDialogueIndex = nextDialogueIndex;
                        printf("Switching to dialogue: %s\n", dialogues[currentDialogueIndex].label);
                        updateScene(renderer, &bgTexture, &characterTexture, font, textColor, &textTexture, &button1Texture, &button2Texture, &itemTexture, &affinityTexture);
                    } else {
                        printf("Next dialogue not found: %s\n", dialogues[currentDialogueIndex].next[0]);
                    }
                } else if (isMouseOverButton(x, y, button2Rect)) {
                    printf("Button 2 Clicked\n");
                    characterAffinity += dialogues[currentDialogueIndex].affinity_change[1];
                    int nextDialogueIndex = findDialogueIndex(dialogues[currentDialogueIndex].next[1]);
                    if (nextDialogueIndex != -1) {
                        currentDialogueIndex = nextDialogueIndex;
                        printf("Switching to dialogue: %s\n", dialogues[currentDialogueIndex].label);
                        updateScene(renderer, &bgTexture, &characterTexture, font, textColor, &textTexture, &button1Texture, &button2Texture, &itemTexture, &affinityTexture);
                    } else {
                        printf("Next dialogue not found: %s\n", dialogues[currentDialogueIndex].next[1]);
                    }
                }
            } else if (e.type == SDL_MOUSEBUTTONDOWN && itemTexture != NULL) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                if (x >= itemRect.x && x <= itemRect.x + itemRect.w &&
                    y >= itemRect.y && y <= itemRect.y + itemRect.h) {
                    printf("Item Clicked\n");
                    SDL_DestroyTexture(itemTexture);
                    itemTexture = NULL;
                    // 切換到下一個對話
                    int nextDialogueIndex = findDialogueIndex(dialogues[currentDialogueIndex].next[0]);
                    if (nextDialogueIndex != -1) {
                        currentDialogueIndex = nextDialogueIndex;
                        updateScene(renderer, &bgTexture, &characterTexture, font, textColor, &textTexture, &button1Texture, &button2Texture, &itemTexture, &affinityTexture);
                    }
                }
            }
        }

        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, bgTexture, NULL, NULL);

        // 半透明黑色窗口
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128); // 黑色，50% 透明度
        SDL_Rect blackRect = { 0, SCREEN_HEIGHT * SCALE - 200 * SCALE, SCREEN_WIDTH * SCALE, 200 * SCALE };
        SDL_RenderFillRect(renderer, &blackRect);

        // 人物
        SDL_Rect playerRect = { 100 * SCALE, SCREEN_HEIGHT * SCALE - 300 * SCALE, 100 * SCALE, 150 * SCALE };  // 調整人物位置
        SDL_RenderCopy(renderer, characterTexture, NULL, &playerRect);

        // 顯示物品圖標
        if (itemTexture != NULL) {
            SDL_RenderCopy(renderer, itemTexture, NULL, &itemRect);
        }

        // 顯示角色好感度
        SDL_Rect affinityRect = { SCREEN_WIDTH * SCALE - 250 * SCALE, 50 * SCALE, 200 * SCALE, 50 * SCALE }; // 顯示位置和大小 (右上角)
        SDL_RenderCopy(renderer, affinityTexture, NULL, &affinityRect);

        // 文字
        SDL_Rect textRect = { 50 * SCALE, SCREEN_HEIGHT * SCALE - 180 * SCALE, SCREEN_WIDTH * SCALE - 100 * SCALE, 100 * SCALE };  // 調整文字位置和大小
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        // 按鈕背景
        SDL_SetRenderDrawColor(renderer, 50, 67, 128, 128); // 藍色
        SDL_RenderFillRect(renderer, &button1Rect);
        SDL_RenderFillRect(renderer, &button2Rect);

        // 按鈕文字
        if (button1Texture != NULL) {
            SDL_Rect button1TextRect = {button1Rect.x + 10 * SCALE, button1Rect.y + 10 * SCALE, button1Rect.w - 20 * SCALE, button1Rect.h - 20 * SCALE};
            SDL_RenderCopy(renderer, button1Texture, NULL, &button1TextRect);
        }
        if (button2Texture != NULL) {
            SDL_Rect button2TextRect = {button2Rect.x + 10 * SCALE, button2Rect.y + 10 * SCALE, button2Rect.w - 20 * SCALE, button2Rect.h - 20 * SCALE};
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
    SDL_DestroyTexture(itemTexture);
    SDL_DestroyTexture(affinityTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
