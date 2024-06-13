#include <curl/curl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 600;

const char* messages[] = {"Hello", "Where is it?"};
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

int runGame1() {
    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    // 創建窗口
    SDL_Window* window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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

    // 背景圖片
    SDL_Surface* bgSurface = IMG_Load("library.jpg");
    if (bgSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", "library.jpg", IMG_GetError());
        return -1;
    }
    SDL_Texture* bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_FreeSurface(bgSurface);

    // 人物
    SDL_Surface* playerSurface = IMG_Load("player_happy.png");
    if (playerSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", "player_happy.png", IMG_GetError());
        return -1;
    }
    SDL_Texture* playerTexture = SDL_CreateTextureFromSurface(renderer, playerSurface);
    SDL_FreeSurface(playerSurface);

    // 字型
    TTF_Font* font = TTF_OpenFont("NotoSansTC-Regular.ttf", 28);
    if (font == NULL) {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        return -1;
    }

    // 設定顏色
    SDL_Color textColor = {255, 255, 255, 255};

    SDL_Texture* textTexture = renderText(renderer, messages[currentMessageIndex], font, textColor);
    if (textTexture == NULL) {
        return -1;
    }

    // 按鈕文字
    SDL_Texture* button1Texture = renderText(renderer, "GO!", font, textColor);
    SDL_Texture* button2Texture = renderText(renderer, "Leave.", font, textColor);

    // 按鈕位置
    SDL_Rect button1Rect = {SCREEN_WIDTH - 150, SCREEN_HEIGHT - 190, 100, 50};
    SDL_Rect button2Rect = {SCREEN_WIDTH - 150, SCREEN_HEIGHT - 130, 100, 50};

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
                if (isMouseOverButton(x, y, button2Rect)) {
                    // 點擊Leave按鈕，退出並進入第二個遊戲
                    quit = true;
                    SDL_DestroyTexture(textTexture);
                    SDL_DestroyTexture(button1Texture);
                    SDL_DestroyTexture(button2Texture);
                    SDL_DestroyTexture(bgTexture);
                    SDL_DestroyTexture(playerTexture);
                    SDL_DestroyRenderer(renderer);
                    SDL_DestroyWindow(window);
                    TTF_CloseFont(font);
                    TTF_Quit();
                    IMG_Quit();
                    SDL_Quit();
                    return 1;  // 特定返回值表示需要進入第二個遊戲
                } else if (isMouseOverButton(x, y, button1Rect)) {
                    currentMessageIndex = (currentMessageIndex + 1) % 2;

                    // 更新文字
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

        // 半透明黑色窗口
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128); // 黑色，50% 透明度
        SDL_Rect blackRect = { 0, SCREEN_HEIGHT - 200, SCREEN_WIDTH, 200 };
        SDL_RenderFillRect(renderer, &blackRect);

        // 人物
        SDL_Rect playerRect = { 100, SCREEN_HEIGHT - 200, 100, 150 };  // 調整人物位置
        SDL_RenderCopy(renderer, playerTexture, NULL, &playerRect);

        // 文字
        SDL_Rect textRect = { 200, SCREEN_HEIGHT - 150, 100, 50 };  // 調整文字位置和大小
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        // 按鈕
        SDL_RenderCopy(renderer, button1Texture, NULL, &button1Rect);
        SDL_RenderCopy(renderer, button2Texture, NULL, &button2Rect);

        // 更新
        SDL_RenderPresent(renderer);
    }

    // 清理
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
        printf("not enough memory (realloc returned NULL)\n");
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
    FILE *fp;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        fp = fopen(outputFilename, "wb");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData);
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Authorization: Bearer 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");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(fp);
        return (res == CURLE_OK) ? 0 : -1;
    }
    return -1;
}

void renderTextInput(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Rect rect) {
    SDL_Color textColor = {255, 255, 255, 255}; // 白色
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_RenderCopy(renderer, textTexture, NULL, &rect);
    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
}

void renderInputBox(SDL_Renderer* renderer, SDL_Rect rect) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // 黑色
    SDL_RenderDrawRect(renderer, &rect);
}


int runGame2() {
    const char* geminiUrl = "https://generativelanguage.googleapis.com/v1beta/models/gemini-pro:generateContent?key=YOUR_GEMINI_API_KEY";
    const char* huggingFaceUrl = "https://api-inference.huggingface.co/models/Corcelio/mobius";
    const char* outputFilename = "1.jpg";
    const char* waitingFilename = "waiting.jpg";
    char inputText[256] = "";
    bool inputComplete = false;

    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    // 創建窗口
    SDL_Window* window = SDL_CreateWindow("Second Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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

    // 加載字體
    TTF_Font* font = TTF_OpenFont("NotoSansTC-Regular.ttf", 24);
    if (font == NULL) {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        return -1;
    }

    // 加載waiting圖片
    SDL_Surface* waitingSurface = IMG_Load(waitingFilename);
    if (waitingSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", waitingFilename, IMG_GetError());
        return -1;
    }
    SDL_Texture* waitingTexture = SDL_CreateTextureFromSurface(renderer, waitingSurface);
    SDL_FreeSurface(waitingSurface);

    // 顯示文字輸入框
    SDL_StartTextInput();
    SDL_Rect inputRect = {50, 50, 800, 50}; // 調整位置和大小

    // 主要循環
    bool quit = false;
    SDL_Event e;

    while (!quit && !inputComplete) {
        // 處理事件
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_TEXTINPUT) {
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
    snprintf(additional_text, sizeof(additional_text), "請讓一切方向依照我的故事情境走。你需要從用戶給的回饋推測即將發生第幾幕場景，並生成該幕文本。這裡是一些劇本內會用到的元素：道具有：蘑菇、樹枝。人物：老人智者、女狩獵家、男建築師。結局編號：有1,2,3,4。每一幕只會有一個人講一段話。第一幕開始前用戶的回覆可能是：這裡是哪裡。接著就進入第一幕，是一個無害的長者給你提示跟你說接下來需要找食物和房子。 第二幕開始前我的回覆可能是：好的我想要一些食物。接著就進入第二幕，是一個有害的女狩獵家跟你說接你可以吃一些果實充飢，你需要讓故事感覺起來像女狩獵家是好人。 第三幕開始前我的回覆可能是：好的我想要蓋房子。接著就進入第三幕，是一個有害的男建築師跟你說接你可以用這些木頭蓋房子，你需要讓故事感覺起來像男建築師家好人。 第四幕是結局，如果吃了果實就是毒死，如果用樹枝蓋房子就是樹枝建造的房子被河水暴漲沖散，只有不吃果實和不蓋房子的人得以存活。一次只需要回傳接下來那一幕的內容，一個人物的一段對話、以及兩個選項：接受該行為建議或是拒絕該建議，要把建議是什麼講出來，例如吃果實、蓋房子，選項最後都要包含前往下一幕的提示詞，例如，接受果實並前往下個場景（跳到結局，吃了果實所以中毒病倒）、例如：接受樹枝並前往下個場景（跳到結局，因為用樹枝蓋房子，所以樹枝房屋被沖散）、如果當已經是最後一幕，選項是結束遊戲和回到第一幕。用戶的回覆是：%s", inputText);

    char geminiRequestData[2048];
    snprintf(geminiRequestData, sizeof(geminiRequestData), "{\"contents\": [{\"parts\": [{\"text\": \"%s\"}]}]}", additional_text);

    char *geminiResponse = perform_query(geminiUrl, geminiRequestData, NULL);

    // 在屏幕上顯示 Gemini 回應
    SDL_Surface* geminiResponseSurface = TTF_RenderText_Solid(font, geminiResponse, (SDL_Color){255, 255, 255, 255});
    SDL_Texture* geminiResponseTexture = SDL_CreateTextureFromSurface(renderer, geminiResponseSurface);
    SDL_Rect geminiResponseRect = {50, 150, 800, geminiResponseSurface->h}; // 調整位置和大小

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, geminiResponseTexture, NULL, &geminiResponseRect);
    SDL_RenderPresent(renderer);

    SDL_Delay(5000); // 等待 5 秒以便用戶閱讀回應

    // 顯示waiting圖片
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, waitingTexture, NULL, NULL);
    SDL_RenderPresent(renderer);

    char postData[512];
    snprintf(postData, sizeof(postData), "{\"inputs\": \"%s\"}", geminiResponse);

    // 下載圖片
    if (downloadImage(huggingFaceUrl, postData, outputFilename) != 0) {
        printf("Failed to download image.\n");
        SDL_DestroyTexture(waitingTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_CloseFont(font);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    // 加載下載的圖片
    SDL_Surface* bgSurface = IMG_Load(outputFilename);
    if (bgSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", outputFilename, IMG_GetError());
        SDL_DestroyTexture(waitingTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_CloseFont(font);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return -1;
    }
    SDL_Texture* bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_FreeSurface(bgSurface);

    // 清理waiting圖片資源
    SDL_DestroyTexture(waitingTexture);
    SDL_DestroyTexture(geminiResponseTexture);
    SDL_FreeSurface(geminiResponseSurface);
    free(geminiResponse);

    // 主要循環
    quit = false;
    while (!quit) {
        // 處理事件
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, bgTexture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    // 清理
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
    // 進行遊戲1
    int result = runGame1();

    // 如果遊戲1返回特定值，則進入遊戲2
    if (result == 1) {
        runGame2();
    }

    return 0;
}
