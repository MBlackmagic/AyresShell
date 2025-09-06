<p align="center">
  <img src="https://res.cloudinary.com/dxunooptp/image/upload/v1754359437/banner_github_20250804_225256_0000_i9yrwv.jpg" alt="AyresShell Banner" width="100%" />
</p>

# AyresShell

**AyresShell** is an interactive serial console for ESP32 that lets you execute administrative, debugging, file management, and JSON editing commands directly from the serial monitor.  
Created by [Daniel Salgado](https://github.com/ayresnet), it is compatible with both Arduino IDE and PlatformIO.
Translation and added functions by Mehmet Ersöz.

---

## 🚀 Features

- Shell-style commands to interact with the file system (LittleFS)
- Direct JSON field editing from the terminal
- Easily extendable with custom commands via `addCommand`
- Compatible with:
  - ✅ Arduino IDE
  - ✅ PlatformIO

---

## 📦 Installation

### 🔧 PlatformIO

```ini
lib_deps = https://github.com/ayresnet/AyresShell
```

### 🔧 Arduino IDE

1. Download the repository as ZIP
2. Extract into `Documents/Arduino/libraries/AyresShell`
3. Restart the IDE

---

## 🧪 Basic Example

```cpp
#include <AyresShell.h>
#include <FS.h>
#include <LittleFS.h>

AyresShell shell;

void setup() {
  Serial.begin(115200);
  LittleFS.begin();

  shell.begin();
}

void loop() {
  shell.handleInput();
}
```
---

## 📡 How to use the console

Open the **Serial Monitor** (at 115200 baud) from your IDE.

Then simply type commands and press Enter. For example:

```
DIR
TYPE /config.json
DEL /file.txt
JSONSET /config.json name "Daniel"
```

Use `HELP` to view the full list of available commands.

---

## 🧭 Built-in Commands

| Command                          | Description                                                |
|----------------------------------|------------------------------------------------------------|
| `DIR` or `LS`                    | Lists files in the file system                             |
| `TYPE <file>` or `CAT` <file>    | Displays the contents of a file                            |
| `DEL <file>` or `RM`             | Deletes a file                                             |
| `REN <a> <b>`                    | Renames a file                                             |
| `MV <a> <b>`                     | Moves a file to another folder                             |
| `MKDIR <folder>`                 | Creates a directory                                        |
| `RMDIR <folder>`                 | Removes an empty directory                                 |
| `CD <folder>`                    | Changes current directory (supports `..` and `/`)          |
| `CLS` or `CLEAR`                 | Clears the screen                                          |
| `HELP` or `MAN`                  | Displays the list of available commands                    |
| `FORMAT`                         | Formats the file system (confirm with `S`)                 |
| `UPTIME`                         | Uptime of the ESP                                          |
| `FREE`                           | Free memory heap                                           |
| `CHIPINFO`                       | Chipinfo of the ESP                                        |
| `VERSION`                        | Version of the Firmware                                    |
| `JSONSET <path> <key> "<value>"` | Edits a field inside a JSON file                           |

---

## 📂 Project Structure

```
AyresShell/
├── src/
│   ├── AyresShell.cpp
│   └── AyresShell.h
├── examples/
│   └── BasicShellDemo/
├── library.json
├── library.properties
├── keywords.txt
├── LICENSE
└── README.md
```

---

## 🪪 License

Distributed under the MIT license.  
Free to use, modify, and share.

---

> 📘 [Read this in Spanish → README.es.md](README.es.md)
