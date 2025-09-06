#ifndef AYRESSHELL_H
#define AYRESSHELL_H

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <WebServer.h>   // Include if used in your project
#include <ArduinoJson.h> // Required for JSON manipulation

// Current working directory (declared as global if used elsewhere)
extern String currentDir;

// ==================== MAIN CLASS ====================
class AyresShell {
public:
  void begin();        // Welcome message or initialization logic
  void handleInput();  // Encapsulates handleSerialCommands()
  void addCommand(const String& name, std::function<void(const String&)> callback);
};

// Shell command functions
void listDir(fs::FS &fs, const char * dirname);
void readFile(fs::FS &fs, const char * pathInput);
void deleteFile(fs::FS &fs, const char * pathInput);
void renameFile(fs::FS &fs, const char * oldNameInput, const char * newNameInput);
void moveFile(fs::FS &fs, const char * fromInput, const char * toInput);
void createDir(fs::FS &fs, const char * pathInput);
void removeDir(fs::FS &fs, const char * pathInput);
void clearScreen();
void help();
void handleSerialCommands();

// --- JSON SUPPORT FUNCTIONS ---
// Helper functions for JSON handling
bool loadJsonFile(fs::FS &fs, const char *path, JsonDocument &doc);
bool saveJsonFile(fs::FS &fs, const char *path, const JsonDocument &doc);
// Main function to update a JSON field via command
bool updateJsonField(fs::FS &fs, const char *path, const char *key, const char *newValue);

#endif
