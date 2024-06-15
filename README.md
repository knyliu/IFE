# IFE

## For merge.c
```gcc -o merge merge.c toml.c -lSDL2 -lSDL2_image -lSDL2_ttf -lcjson -lcurl```
### Code Explanation
* 這個版本的程式碼結合了Game1與Game2
*  程式裡面的```// ----```分割了Game1與Game2
*  ```main()```會變得很簡單，如：
   ```c
   int main(int argc, char* argv[]) {
       // 進行遊戲1
       int result = runGame1();
   
       // 如果遊戲1返回特定值，則進入遊戲2
       if (result == 1) {
           runGame2();
       }
   
       return 0;
   }
   ```
* 需要修改一律不動main
* 原本自己寫的main已經在```runGame1()```和```runGame2()```裡面了，請在此修改
* btw, 我有修改```#include "toml.h"```include方式，如有需要可以修改回```#include <toml.h>```

### 此處說明如何實現Game1進入Game2
* 我在toml檔案裡面新增了
  ```
  [dialogue.last_scene]
  text = "This is the end of the game."
  character = "Old Wise Man"
  scene = "Library"
  options = [
      {text = "End the game", next = ""},
      {text = "Continue to next game", next = "game2", is_final = true}
  ]
  ```
* 並且把關鍵點設定在「拿起鑰匙」被按下之後
* 當然，你可以視劇本修改

## For gui_2games.c
```gcc -o gui gui.c -lSDL2 -lSDL2_image -lSDL2_ttf -lcurl -lcjson```

## Environment
* curl, libevent
  * We assume you already had libcurl
  * For libevent, please run ```sudo apt-get install libevent-dev```

## How to build and run the code

* We treat main.c as the server.
  * ```gcc main.c -o server -levent -lcurl -lm```
  * ```./server```

* We treat index.html as the front-end page
  * open index.html
