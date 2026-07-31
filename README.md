Markdown
# 🎮 Pocket Engine

## 🌟 What is Pocket Engine?
Pocket Engine is an ultra-lightweight, zero-dependency C game engine designed strictly for the terminal.
It uses only standard C libraries and OS-native APIs (ANSI escape sequences, termios). It is perfect for learning low-level programming, data-structure-centric design, and C language fundamentals.

### ✨ Key Features
* **Flicker-Free Rendering:** Built-in double buffering for smooth terminal graphics.
* **Event System:** Simple event polling system with support for special keys (Arrow keys, etc.).
* **Scene Management:** Built-in scene registry to easily switch between Title, Gameplay, and Result screens.
* **Safe Memory Management:** Integrated Arena Allocator that automatically cleans up scratch memory every frame.

---

## 🚀 How to Setup
Pocket Engine is extremely simple to integrate. Choose one of the two methods below.

### Method A: Git Submodule (Recommended)
If you are using Git for your game project, this is the best way to keep the engine updated.

1. Open your terminal and navigate to your game's repository.
2. Run the following command to add the engine into a `vendor/pocket_engine` directory:
```bash
git submodule add [https://github.com/YOUR_USERNAME/pocket_engine.git](https://github.com/YOUR_USERNAME/pocket_engine.git) vendor/pocket_engine
```

### Method B: Manual Copy (Easiest)
If you are not familiar with Git, simply download and copy the files.

Download this repository.

Create a vendor/pocket_engine/ folder in your game project.

Copy the include/ and src/ folders from the engine into your new folder.

📝 Quick Start
Once you have the engine in your project, create a main.c file and paste the following code:

```C
#include <stdio.h>
#include "pocket.h" // Include the engine API

// 1. Initialization (Called once)
void my_init(void *user_data) {
        // Initialize your game data here
        (void)user_data;
}

// 2. Update logic (Called every frame)
void my_update(void *user_data, float dt) {
        (void)user_data;
        (void)dt;
        struct pkt_event e;
        // Poll input events
        while (pkt_poll_event(&e) == 0) {
                if (e.type == PKT_EVENT_KEY_PRESSED) {
                        // Quit if ESC is pressed
                        if (e.data.key.key_code == PKT_KEY_ESCAPE) {
                                pkt_quit();
                        }
                }
        }
}

// 3. Draw logic (Called every frame)
void my_draw(void *user_data) {
        // Draw green text at (x:10, y:10)
        (void)user_data;
        pkt_puts(10, 10, PKT_COLOR_GREEN, PKT_COLOR_BLACK, "Hello, Pocket Engine!");
        pkt_puts(10, 11, PKT_COLOR_YELLOW, PKT_COLOR_BLACK, "Press Escape to quit, and start coding your own special game!!");
}

int main(void) {
        struct pkt_config config = pkt_get_default_config();
        config.on_init = my_init;

        if (pkt_init(&config) < 0) {
                printf("Failed to initialize engine.\n");
                return -1;
        }

        // Register and swap to your scene
        struct pkt_scene scene = {0};
        scene.on_update = my_update;
        scene.on_draw = my_draw;

        pkt_register_scene(0, &scene);
        pkt_swap_scene(0);

        // Start the game loop!
        pkt_ignite();

        // Restore terminal attributes
        pkt_cleanup();

        return 0;
}
```

## 🛠️ Compilation

### Option 1: Simple GCC Command

You can compile your game directly from the terminal:

```Bash
gcc main.c vendor/pocket_engine/src/engine/*.c -I vendor/pocket_engine/include -o my_game
./my_game
```

### Option 2: Using a Makefile

For a better development experience, create a Makefile in your project root and paste this template:

```Makefile
CC = gcc
CFLAGS = -Wall -Wextra -O2 -I vendor/pocket_engine/include

# Your game source files
SRC = main.c

# Engine source files
ENGINE_SRC = $(wildcard vendor/pocket_engine/src/engine/*.c)

# Output executable name
TARGET = my_game

all: $(TARGET)

$(TARGET): $(SRC)$(ENGINE_SRC)
	$(CC)$(CFLAGS) $^ -o$@

clean:
	rm -f $(TARGET)
```

Now, you only need to type make in your terminal to build your game!

## 📚 API Reference
Pocket Engine provides various functions for drawing characters, colored text, formatted strings (like printf), and more.

All available functions and their detailed usages are thoroughly documented in English inside the core header file: include/pocket.h. Please refer to pocket.h for the complete API documentation!
