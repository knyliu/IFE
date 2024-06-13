# IFE

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
