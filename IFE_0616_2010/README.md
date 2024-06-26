# 寶藏世界探索遊戲

## Environment
* 必要函式庫
  * SDL2
  * SDL2_image
  * SDL2_ttf
  * curl
  * toml
* 安裝指南（如果你沒有以上必要函式庫）
  * 在Ubuntu上可以使用以下指令安裝必要的函式庫：
    ```
    sudo apt-get update
    sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libcurl4-gnutls-dev
    ```
  * 在Ubuntu上安裝 `tomlc99` ：
    * 你可以從 GitHub 下載原始碼    
    ```
     git clone https://github.com/cktan/tomlc99.git
    ```
    * 進入下載的目錄並編譯
    ```
     cd tomlc99
     make
    ```

## 如何編譯這份程式碼
* 在終端機輸入：
  ```
  make
  ```
* 編譯完成後，輸入：
  ```
  ./engine ./example-game
  ```
  就可以看到遊戲介面。

## 遊戲遊玩注意事項
* 此遊戲分為兩大主軸，解鎖第一章即可進入第二章。
* 第一章提供選項與遊戲互動，第二章使用自然語言輸入互動。
  * 在第二章的自然語言輸入時，若習慣使用注音輸入，看不到注音，為正常現象。在打字完成後按下Enter即可送出請求。
  * 因此，進入第二張時，可以直接輸入文字。

## 其他說明

1. **劇本檔格式**：
   * 劇本檔是遊戲的核心，描述了遊戲中的情節、角色對話和選項等。
   * 每個劇本檔以 `.toml` 格式儲存。

2. **命名方式與存放地點**：
   * 劇本檔存放在 `example-game/script.toml`。

### 已知問題與注意事項
* **注音輸入顯示**：在第二章的自然語言輸入時，注音輸入法可能會無法顯示注音符號，但這不會影響實際輸入。
* **特殊字元處理**：某些特殊字元可能無法正確顯示或輸入，建議避免在劇本中使用。
* **LLM伺服器可能拒絕不正當的輸入**：在伺服器端可能拒絕帶有不正當要求或不合理的輸入，若遊戲因輸入文字後終止，請重啟遊戲並避免使用類似的文字敘述。
* **生成式AI的不穩定性**：生成式AI的生成內容可能不符合預期，請多見諒。
* **Keys的使用**：所有金鑰將於程式設計（二）學期成績公布後刪除。此外，不得將金鑰用於其他用途。

希望這些指南能幫助你順利安裝並開始遊玩《寶藏世界探索遊戲》。祝你遊戲愉快！
