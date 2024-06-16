#include <curl/curl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "cJSON_Utils.h"
#include "toml.h"

#define PTOMPT_ENGINEERING "請推測以上這句話是要銜接近入第幾幕會有的對話，並生成將要發生的那一幕的文本（像是智慧長者或女狩獵家或男建築師要對玩家說的話）以及讓用戶可以選擇的兩個選項，規則如下：請讓一切方向依照我的故事情境走。你需要從用戶給的回饋推測即將發生第幾幕場景，並生成該幕文本。這裡是一些劇本內會用到的元素：道具有：蘑菇、樹枝。人物：老人智者、女狩獵家、男建築師。每一幕只會有一個人講一段話。第一幕開始前用戶的回覆可能是：這裡是哪裡。接著就進入第一幕，是一個無害的長者給你提示跟你說接下來需要找食物和房子。 第二幕開始前我的回覆可能是：好的我想要一些食物。接著就進入第二幕，是一個有害的女狩獵家跟你說接你可以吃一些果實充飢，你需要讓故事感覺起來像女狩獵家是好人。 第三幕開始前我的回覆可能是：好的我想要蓋房子。接著就進入第三幕，是一個有害的男建築師跟你說接你可以用這些木頭蓋房子，你需要讓故事感覺起來像男建築師家好人。 第四幕是結局，開始前我的回覆可能是吃了果實或是接受樹枝蓋了房子，如果吃了果實就顯示結局就是毒死，如果用樹枝蓋房子就顯示結局是樹枝建造的房子被河水暴漲沖散，只有不吃果實和不蓋房子的人就顯示結局得以存活，要把結局告訴玩家。一次只需要回傳接下來那一幕的內容，一個人物的一段對話、以及兩個選項：接受該行為建議或是拒絕該建議，要把建議是什麼講出來，例如吃果實、蓋房子，選項最後都要包含前往下一幕的提示詞，例如，接受果實並前往下個場景（跳到結局，吃了果實所以中毒病倒）、例如：接受樹枝並前往下個場景（跳到結局，因為用樹枝蓋房子，所以樹枝房屋被沖散）、如果當已經是最後一幕，選項是結束遊戲和回到第一幕。請基於接下來的用回覆推測是第幾幕，當然，你可以自己規劃一些新的幕以及劇情發展，並只生成那一幕的內容和提供給玩家的兩個選項。"


const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 600;

#define SCALE 0.75

const char* messages[] = {"Hello", "Where is it?"};
int currentMessageIndex = 0;

typedef struct {
    char scene_name[256];
    char background[256];
} Scene;

typedef struct {
    char label[256];       
    char character[256];   
    char text[256];       
    char options[2][256]; 
    char next[2][256];   
    char background[256];    
    bool is_final;     
} Dialogue;


typedef struct {
    char name[256];       
    char avatar[256];   
} Character;

Scene scenes[10];
Dialogue dialogues[30];
Character characters[10];
int currentDialogueIndex = 0;

bool isMouseOverButton(int x, int y, SDL_Rect buttonRect) {
    return x >= buttonRect.x && x <= buttonRect.x + buttonRect.w &&
           y >= buttonRect.y && y <= buttonRect.y + buttonRect.h;
}

SDL_Texture* renderText(SDL_Renderer* renderer, const char* message, TTF_Font* font, SDL_Color color) {
    SDL_Surface* textSurface = TTF_RenderUTF8_Solid(font, message, color);
    if (textSurface == NULL) {
        printf("\nUnable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
        return NULL;
    }
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);
    return textTexture;
}

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

    toml_table_t* dialogue_table = toml_table_in(conf, "dialogue");
    toml_table_t* character_table = toml_table_in(conf, "character");

    int i = 0;
    const char* key;
    toml_table_t* tab;
    
    while ((key = toml_key_in(dialogue_table, i++))) {
        tab = toml_table_in(dialogue_table, key);
        if (tab) {
            char* text = NULL;
            char* background = NULL;
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

                int is_final;
                if (toml_rtob(toml_raw_in(opt_tab, "is_final"), &is_final) == 0) {
                    dialogues[i-1].is_final = (bool)is_final;
                } else {
                    dialogues[i-1].is_final = false;
                }
            }
        }
    }

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
    return -1;
}

int findCharacterIndex(const char* characterName) {
    for (int i = 0; i < 10; i++) {
        if (strcmp(characterName, characters[i].name) == 0) {
            return i;
        }
    }
    return -1;
}

void updateScene(SDL_Renderer* renderer, SDL_Texture** bgTexture, SDL_Texture** characterTexture, TTF_Font* font, SDL_Color textColor, SDL_Texture** textTexture, SDL_Texture** button1Texture, SDL_Texture** button2Texture) {
    printf("Updating scene to dialogue: %s\n", dialogues[currentDialogueIndex].label);

    printf("Loading background image: %s\n", dialogues[currentDialogueIndex].background);
    SDL_Surface* newBgSurface = IMG_Load(dialogues[currentDialogueIndex].background);
    if (newBgSurface) {
        SDL_DestroyTexture(*bgTexture);
        *bgTexture = SDL_CreateTextureFromSurface(renderer, newBgSurface);
        SDL_FreeSurface(newBgSurface);
    } else {
        printf("Unable to load image %s! SDL_image Error: %s\n", dialogues[currentDialogueIndex].background, IMG_GetError());
    }

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

    SDL_DestroyTexture(*textTexture);
    SDL_DestroyTexture(*button1Texture);
    SDL_DestroyTexture(*button2Texture);

    *textTexture = renderText(renderer, dialogues[currentDialogueIndex].text, font, textColor);

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

int runGame1() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("\nSDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("\nRenderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    int imgFlags = IMG_INIT_JPG | IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("\nSDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return -1;
    }

    if (TTF_Init() == -1) {
        printf("\nSDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        return -1;
    }

    loadScenesAndDialogues("example-game/script.toml");

    printf("Initial dialogue background: %s\n", dialogues[currentDialogueIndex].background);
    if (strlen(dialogues[currentDialogueIndex].background) == 0) {
        printf("Error: Initial background image not specified!\n");
        return -1;
    }

    printf("Loading initial background image: %s\n", dialogues[currentDialogueIndex].background);
    SDL_Surface* bgSurface = IMG_Load(dialogues[currentDialogueIndex].background);
    if (bgSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", dialogues[currentDialogueIndex].background, IMG_GetError());
        return -1;
    }
    SDL_Texture* bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_FreeSurface(bgSurface);

    SDL_Surface* playerSurface = IMG_Load("example-game/assets/player_happy.png");
    if (playerSurface == NULL) {
        printf("\nUnable to load image %s! SDL_image Error: %s\n", "example-game/assets/player_happy.png", IMG_GetError());
        return -1;
    }
    SDL_Texture* characterTexture = SDL_CreateTextureFromSurface(renderer, playerSurface);
    SDL_FreeSurface(playerSurface);

    TTF_Font* font = TTF_OpenFont("example-game/assets/NotoSansTC-Regular.ttf", 28);
    if (font == NULL) {
        printf("\nFailed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        return -1;
    }

    SDL_Color textColor = {255, 255, 255, 255};

    SDL_Texture* textTexture = renderText(renderer, dialogues[currentDialogueIndex].text, font, textColor);
    if (textTexture == NULL) {
        return -1;
    }

    SDL_Texture* button1Texture = renderText(renderer, dialogues[currentDialogueIndex].options[0], font, textColor);
    SDL_Texture* button2Texture = renderText(renderer, dialogues[currentDialogueIndex].options[1], font, textColor);

    int scaled_width = 400 * SCALE;
    int scaled_height = 50 * SCALE;
    int scaled_x1 = (SCREEN_WIDTH * SCALE / 4) - (scaled_width / 2);
    int scaled_x2 = (3 * SCREEN_WIDTH * SCALE / 4) - (scaled_width / 2);
    int scaled_y = SCREEN_HEIGHT * SCALE - 60;

    SDL_Rect button1Rect = {scaled_x1, scaled_y, scaled_width, scaled_height};
    SDL_Rect button2Rect = {scaled_x2, scaled_y, scaled_width, scaled_height};

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                if (isMouseOverButton(x, y, button2Rect)) {
                    if (dialogues[currentDialogueIndex].is_final) {
                        quit = true;
                        SDL_DestroyTexture(textTexture);
                        SDL_DestroyTexture(button1Texture);
                        SDL_DestroyTexture(button2Texture);
                        SDL_DestroyTexture(bgTexture);
                        SDL_DestroyTexture(characterTexture);
                        SDL_DestroyRenderer(renderer);
                        SDL_DestroyWindow(window);
                        TTF_CloseFont(font);
                        TTF_Quit();
                        IMG_Quit();
                        SDL_Quit();
                        return 1; 
                    } else {
                        currentDialogueIndex = findDialogueIndex(dialogues[currentDialogueIndex].next[1]);
                        updateScene(renderer, &bgTexture, &characterTexture, font, textColor, &textTexture, &button1Texture, &button2Texture);
                    }
                } else if (isMouseOverButton(x, y, button1Rect)) {
                    currentDialogueIndex = findDialogueIndex(dialogues[currentDialogueIndex].next[0]);
                    updateScene(renderer, &bgTexture, &characterTexture, font, textColor, &textTexture, &button1Texture, &button2Texture);
                }
            }
        }

        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, bgTexture, NULL, NULL);

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128); 
        SDL_Rect blackRect = { 0, SCREEN_HEIGHT * SCALE - 200 * SCALE, SCREEN_WIDTH * SCALE, 200 * SCALE };
        SDL_RenderFillRect(renderer, &blackRect);

        SDL_Rect playerRect = { 100 * SCALE, SCREEN_HEIGHT * SCALE - 300 * SCALE, 100 * SCALE, 150 * SCALE }; 
        SDL_RenderCopy(renderer, characterTexture, NULL, &playerRect);

        SDL_Rect textRect = { 50 * SCALE, SCREEN_HEIGHT * SCALE - 200 * SCALE, SCREEN_WIDTH * SCALE - 100 * SCALE, 100 * SCALE };
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        SDL_SetRenderDrawColor(renderer, 50, 67, 128, 128);
        SDL_RenderFillRect(renderer, &button1Rect);
        SDL_RenderFillRect(renderer, &button2Rect);

        if (button1Texture != NULL) {
            SDL_Rect button1TextRect = {button1Rect.x + 50 * SCALE, button1Rect.y + 10 * SCALE, button1Rect.w - 150 * SCALE, button1Rect.h - 20 * SCALE};
            SDL_RenderCopy(renderer, button1Texture, NULL, &button1TextRect);
        }
        if (button2Texture != NULL) {
            SDL_Rect button2TextRect = {button2Rect.x + 50 * SCALE, button2Rect.y + 10 * SCALE, button2Rect.w - 150 * SCALE, button2Rect.h - 20 * SCALE};
            SDL_RenderCopy(renderer, button2Texture, NULL, &button2TextRect);
        }

        SDL_RenderPresent(renderer);
    }

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


// ----

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

typedef struct {
    char *memory;
    size_t size;
} MemoryStruct;

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    MemoryStruct *mem = (MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL) {
        printf("\nnot enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

char *perform_query(const char *url, const char *data, const char *auth_header) {
    CURL *curl;
    CURLcode res;
    MemoryStruct chunk;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);

    curl = curl_easy_init();
    if(curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        if (auth_header != NULL) {
            headers = curl_slist_append(headers, auth_header);
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    curl_global_cleanup();

    return chunk.memory;
}

int downloadImage(const char* url, const char* postData, const char* outputFilename) {
    CURL *curl;
    CURLcode res;

    FILE *fp = fopen(outputFilename, "wb");
    if (!fp) {
        fprintf(stderr, "Failed to open file %s for writing\n", outputFilename);
        return -1;
    }

    curl = curl_easy_init();
    if(curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Authorization: Bearer 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");

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            fclose(fp);
            return -1;
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    fclose(fp);
    return 0;
}

char *get_refined_prompt(const char *original_text) {
    char prompt[2048];
    snprintf(prompt, sizeof(prompt), "{\"contents\": [{\"parts\": [{\"text\": \"%s 請幫我整理、縮短、精簡以上這段話，用明白精簡的話，描述這段話可能會有的視覺情境，著重在於人物是誰、背景在哪、一項明顯的道具或物品（例如：果實或樹枝），並強調畫面的寫實。以英文在一段話內呈現。\"}]}]}", original_text);

    printf("\nRefining prompt with data: %s\n", prompt);

    char *response_text = perform_query("https://generativelanguage.googleapis.com/v1/models/gemini-1.0-pro-latest:generateContent?key=AIzaSyDLKFLtYLdnCamnpNCKo5mZZenAvIhoPJs", prompt, NULL);

    printf("\nRefined prompt response: %s\n", response_text);

    cJSON *json = cJSON_Parse(response_text);
    if (json == NULL) {
        fprintf(stderr, "JSON parse error in get_refined_prompt\n");
        free(response_text);
        return NULL;
    }

    cJSON *candidates = cJSON_GetObjectItem(json, "candidates");
    if (!candidates) {
        fprintf(stderr, "No candidates found in refined prompt response\n");
        cJSON_Delete(json);
        free(response_text);
        return NULL;
    }

    cJSON *first_candidate = cJSON_GetArrayItem(candidates, 0);
    cJSON *content = cJSON_GetObjectItem(first_candidate, "content");
    cJSON *parts = cJSON_GetObjectItem(content, "parts");
    cJSON *first_part = cJSON_GetArrayItem(parts, 0);
    cJSON *text = cJSON_GetObjectItem(first_part, "text");

    char *refined_prompt = strdup(cJSON_GetStringValue(text));

    printf("\nRefined prompt: %s\n", refined_prompt);

    cJSON_Delete(json);
    free(response_text);

    return refined_prompt;
}

void renderTextInput(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Rect rect) {
    if (strlen(text) == 0) {
        return;
    }
    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Surface* textSurface = TTF_RenderUTF8_Blended_Wrapped(font, text, textColor, rect.w);
    if (textSurface == NULL) {
        printf("\nUnable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
        return;
    }
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect renderQuad = { rect.x, rect.y, textSurface->w, textSurface->h }; 
    SDL_RenderCopy(renderer, textTexture, NULL, &renderQuad);
    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
}

void renderInputBox(SDL_Renderer* renderer, SDL_Rect rect) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 
    SDL_RenderFillRect(renderer, &rect);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &rect);
}


void renderTextOnImage(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Rect rect) {
    if (strlen(text) == 0) {
        return;
    }
    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Surface* textSurface = TTF_RenderUTF8_Blended_Wrapped(font, text, textColor, rect.w);
    if (textSurface == NULL) {
        printf("\nUnable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
        return;
    }
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect renderQuad = { rect.x, rect.y, textSurface->w, textSurface->h }; 
    SDL_RenderCopy(renderer, textTexture, NULL, &renderQuad);
    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
}

int runGame2() {
    const char* geminiUrl = "https://generativelanguage.googleapis.com/v1/models/gemini-1.0-pro-latest:generateContent?key=AIzaSyDLKFLtYLdnCamnpNCKo5mZZenAvIhoPJs";
    const char* huggingFaceUrl = "https://api-inference.huggingface.co/models/runwayml/stable-diffusion-v1-5";
    const char* outputFilename = "example-game/assets/output.jpg";
    const char* waitingFilename = "example-game/assets/waiting.png";
    const char* replaceFilename = "example-game/assets/replace.png";  
    char inputText[256] = "透過詢問來開始遊戲。你或許可以試試「請智者告訴我，這裡是哪裡」..."; 
    char nextInputText[256] = "";
    bool inputComplete = false;
    bool nextInputComplete = false;
    char *geminiTextContent = NULL;
    SDL_Texture* geminiResponseTexture = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("\nSDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Second Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("\nWindow could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("\nRenderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    int imgFlags = IMG_INIT_JPG | IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("\nSDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return -1;
    }

    if (TTF_Init() == -1) {
        printf("\nSDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        return -1;
    }

    TTF_Font* font = TTF_OpenFont("example-game/assets/NotoSansTC-Regular.ttf", 18);  
    if (font == NULL) {
        printf("\nFailed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        return -1;
    }

    SDL_Surface* waitingSurface = IMG_Load(waitingFilename);
    if (waitingSurface == NULL) {
        printf("\nUnable to load image %s! SDL_image Error: %s\n", waitingFilename, IMG_GetError());
        return -1;
    }
    SDL_Texture* waitingTexture = SDL_CreateTextureFromSurface(renderer, waitingSurface);
    SDL_FreeSurface(waitingSurface);

    SDL_StartTextInput();
    SDL_Rect inputRect = {50, 50, 800, 50}; 
    SDL_Rect nextInputRect = {50, SCREEN_HEIGHT - 100, 800, 50}; 

    bool quit = false;
    SDL_Event e;

    while (!quit && !inputComplete) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_TEXTINPUT) {
                if (strcmp(inputText, "透過詢問來開始遊戲。你或許可以試試「請智者告訴我，這裡是哪裡」...") == 0) {
                    inputText[0] = '\0';
                }
                strcat(inputText, e.text.text);
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_RETURN) {
                    inputComplete = true;
                } else if (e.key.keysym.sym == SDLK_BACKSPACE && strlen(inputText) > 0) {
                    inputText[strlen(inputText) - 1] = '\0';
                }
            }
        }

        SDL_RenderClear(renderer);
        renderInputBox(renderer, inputRect);
        renderTextInput(renderer, font, inputText, inputRect);
        SDL_RenderPresent(renderer);
    }

    SDL_StopTextInput();

    if (quit) {
        SDL_DestroyTexture(waitingTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_CloseFont(font);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 0;
    }

    char additional_text[1024];
    snprintf(additional_text, sizeof(additional_text), "%-.768s %-.255s", inputText, PTOMPT_ENGINEERING);

    char geminiRequestData[2048];
    snprintf(geminiRequestData, sizeof(geminiRequestData), "{\"contents\": [{\"parts\": [{\"text\": \"%s\"}]}]}", additional_text);

    char *geminiResponse = perform_query(geminiUrl, geminiRequestData, NULL);

    printf("\nGemini response: %s\n", geminiResponse); 

    cJSON *geminiJson = cJSON_Parse(geminiResponse);
    if (geminiJson == NULL) {
        fprintf(stderr, "JSON parse error in runGame2\n");
        SDL_DestroyTexture(waitingTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_CloseFont(font);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        free(geminiResponse);
        return -1;
    }

    cJSON *geminiCandidates = cJSON_GetObjectItem(geminiJson, "candidates");
    cJSON *geminiFirstCandidate = cJSON_GetArrayItem(geminiCandidates, 0);
    cJSON *geminiContent = cJSON_GetObjectItem(geminiFirstCandidate, "content");
    cJSON *geminiParts = cJSON_GetObjectItem(geminiContent, "parts");
    cJSON *geminiFirstPart = cJSON_GetArrayItem(geminiParts, 0);
    cJSON *geminiText = cJSON_GetObjectItem(geminiFirstPart, "text");

    geminiTextContent = strdup(cJSON_GetStringValue(geminiText));
    char *p = geminiTextContent;
    char *q = geminiTextContent;
    while (*q) {
        *p = *q++;
        p += (*p != '*' && *p != '"');
    }
    *p = '\0';

    cJSON_Delete(geminiJson);

    printf("\nGemini extracted text: %s\n", geminiTextContent); 

    SDL_Surface* geminiResponseSurface = TTF_RenderUTF8_Blended_Wrapped(font, geminiTextContent, (SDL_Color){255, 255, 255, 255}, SCREEN_WIDTH - 100);
    geminiResponseTexture = SDL_CreateTextureFromSurface(renderer, geminiResponseSurface);
    SDL_Rect geminiResponseRect = {50, 150, SCREEN_WIDTH - 100, geminiResponseSurface->h};

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, geminiResponseTexture, NULL, &geminiResponseRect);
    SDL_RenderPresent(renderer);

    SDL_Delay(5000);

    char *refinedPrompt = get_refined_prompt(geminiTextContent);

    if (refinedPrompt == NULL) {
        printf("\nFailed to get refined prompt\n");
        SDL_DestroyTexture(waitingTexture);
        SDL_DestroyTexture(geminiResponseTexture);
        SDL_FreeSurface(geminiResponseSurface);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_CloseFont(font);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        free(geminiResponse);
        free(geminiTextContent);
        return -1;
    }

    printf("\nRefined prompt for Hugging Face: %s\n", refinedPrompt);

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, waitingTexture, NULL, NULL);
    SDL_RenderPresent(renderer);

    char postData[512];
    snprintf(postData, sizeof(postData), "{\"inputs\": \"%s\"}", refinedPrompt);

    if (downloadImage(huggingFaceUrl, postData, outputFilename) != 0) {
        printf("\nFailed to download image.\n");
        SDL_DestroyTexture(waitingTexture);
        SDL_DestroyTexture(geminiResponseTexture);
        SDL_FreeSurface(geminiResponseSurface);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_CloseFont(font);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        free(geminiResponse);
        free(refinedPrompt);
        free(geminiTextContent);
        return -1;
    }

    SDL_Surface* bgSurface = IMG_Load(outputFilename);
    if (bgSurface == NULL) {
        printf("\nUnable to load image %s! SDL_image Error: %s\n", outputFilename, IMG_GetError());
        bgSurface = IMG_Load(replaceFilename);
        if (bgSurface == NULL) {
            printf("\nUnable to load replace image %s! SDL_image Error: %s\n", replaceFilename, IMG_GetError());
            SDL_DestroyTexture(waitingTexture);
            SDL_DestroyTexture(geminiResponseTexture);
            SDL_FreeSurface(geminiResponseSurface);
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            TTF_CloseFont(font);
            TTF_Quit();
            IMG_Quit();
            SDL_Quit();
            free(geminiResponse);
            free(refinedPrompt);
            free(geminiTextContent);
            return -1;
        }
    }
    SDL_Texture* bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_FreeSurface(bgSurface);

    SDL_DestroyTexture(waitingTexture);
    SDL_DestroyTexture(geminiResponseTexture);
    SDL_FreeSurface(geminiResponseSurface);
    free(geminiResponse);
    free(refinedPrompt);

    SDL_StartTextInput();
    quit = false;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_TEXTINPUT) {
                strcat(nextInputText, e.text.text);
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_RETURN) {
                    nextInputComplete = true;
                } else if (e.key.keysym.sym == SDLK_BACKSPACE && strlen(nextInputText) > 0) {
                    nextInputText[strlen(nextInputText) - 1] = '\0';
                }
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, bgTexture, NULL, NULL);

        SDL_Rect textRect = {50, 50, SCREEN_WIDTH - 100, SCREEN_HEIGHT - 200};
        renderTextOnImage(renderer, font, geminiTextContent, textRect);

        renderInputBox(renderer, nextInputRect);
        renderTextInput(renderer, font, nextInputText, nextInputRect);

        SDL_RenderPresent(renderer);

        if (nextInputComplete) {
            free(geminiTextContent);
            SDL_DestroyTexture(geminiResponseTexture);

            nextInputComplete = false;

            snprintf(additional_text, sizeof(additional_text), "%-.768s %-.255s", nextInputText, PTOMPT_ENGINEERING);

            snprintf(geminiRequestData, sizeof(geminiRequestData), "{\"contents\": [{\"parts\": [{\"text\": \"%s\"}]}]}", additional_text);

            geminiResponse = perform_query(geminiUrl, geminiRequestData, NULL);

            printf("\nGemini response: %s\n", geminiResponse); 

            cJSON *geminiJson = cJSON_Parse(geminiResponse);
            if (geminiJson == NULL) {
                fprintf(stderr, "JSON parse error in runGame2\n");
                SDL_DestroyTexture(waitingTexture);
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                TTF_CloseFont(font);
                TTF_Quit();
                IMG_Quit();
                SDL_Quit();
                free(geminiResponse);
                break;
            }

            cJSON *geminiCandidates = cJSON_GetObjectItem(geminiJson, "candidates");
            cJSON *geminiFirstCandidate = cJSON_GetArrayItem(geminiCandidates, 0);
            cJSON *geminiContent = cJSON_GetObjectItem(geminiFirstCandidate, "content");
            cJSON *geminiParts = cJSON_GetObjectItem(geminiContent, "parts");
            cJSON *geminiFirstPart = cJSON_GetArrayItem(geminiParts, 0);
            cJSON *geminiText = cJSON_GetObjectItem(geminiFirstPart, "text");

            geminiTextContent = strdup(cJSON_GetStringValue(geminiText));
            p = geminiTextContent;
            q = geminiTextContent;
            while (*q) {
                *p = *q++;
                p += (*p != '*' && *p != '"');
            }
            *p = '\0';

            cJSON_Delete(geminiJson);

            printf("\nGemini extracted text: %s\n", geminiTextContent);  

            refinedPrompt = get_refined_prompt(geminiTextContent);

            if (refinedPrompt == NULL) {
                printf("\nFailed to get refined prompt\n");
                SDL_DestroyTexture(bgTexture);
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                TTF_CloseFont(font);
                TTF_Quit();
                IMG_Quit();
                SDL_Quit();
                free(geminiResponse);
                break;
            }

            printf("\nRefined prompt for Hugging Face: %s\n", refinedPrompt);

            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, waitingTexture, NULL, NULL);
            SDL_RenderPresent(renderer);

            snprintf(postData, sizeof(postData), "{\"inputs\": \"%s\"}", refinedPrompt);

            if (downloadImage(huggingFaceUrl, postData, outputFilename) != 0) {
                printf("\nFailed to download image.\n");
                SDL_DestroyTexture(bgTexture);
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                TTF_CloseFont(font);
                TTF_Quit();
                IMG_Quit();
                SDL_Quit();
                free(geminiResponse);
                free(refinedPrompt);
                break;
            }

            bgSurface = IMG_Load(outputFilename);
            if (bgSurface == NULL) {
                printf("\nUnable to load image %s! SDL_image Error: %s\n", outputFilename, IMG_GetError());
                bgSurface = IMG_Load(replaceFilename);
                if (bgSurface == NULL) {
                    printf("\nUnable to load replace image %s! SDL_image Error: %s\n", replaceFilename, IMG_GetError());
                    SDL_DestroyTexture(bgTexture);
                    SDL_DestroyRenderer(renderer);
                    SDL_DestroyWindow(window);
                    TTF_CloseFont(font);
                    TTF_Quit();
                    IMG_Quit();
                    SDL_Quit();
                    free(geminiResponse);
                    free(refinedPrompt);
                    break;
                }
            }
            SDL_DestroyTexture(bgTexture);
            bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
            SDL_FreeSurface(bgSurface);

            free(geminiResponse);
            free(refinedPrompt);
        }
    }
    SDL_StopTextInput();

    SDL_DestroyTexture(bgTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}

int main(int argc, char* argv[]) {
    int result = runGame1();

    if (result == 1) {
        runGame2();
    }

    return 0;
}
