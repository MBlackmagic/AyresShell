/*
 * AyresShell v1.0.0 (English Version)
 * --------------------------------------------------------
 * Interactive serial console for ESP32
 * Developed by Daniel Cristian Salgado - AyresNet
 * English version by Mehmet Ersöz
 * https://github.com/ayresnet/AyresShell
 * --------------------------------------------------------
 * This console allows execution of custom commands,
 * interaction with the file system (LittleFS), and
 * direct editing of JSON files through the serial monitor.
 *
 * Compatible with Arduino IDE and PlatformIO.
 * License: MIT
 */

#include "AyresShell.h"

// Current working directory
String currentDir = "/";

// --- JSON HANDLING FUNCTIONS ---

// JSON document size. Increase if your JSON files are larger or more complex.
const size_t JSON_DOC_SIZE = 256; 

// Load and parse a JSON file
bool loadJsonFile(fs::FS &fs, const char *path, JsonDocument &doc) {
  File file = fs.open(path, "r");
  if (!file) {
    Serial.println("❌ Failed to open JSON file for reading.");
    return false;
  }

  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    Serial.print("❌ Failed to parse JSON (possibly corrupt or invalid file): ");
    Serial.println(error.c_str());
    return false;
  }
  return true;
}

// Save a JsonDocument to a file
bool saveJsonFile(fs::FS &fs, const char *path, const JsonDocument &doc) {
  File file = fs.open(path, "w"); // Open in write mode (overwrite)
  if (!file) {
    Serial.println("❌ Failed to open JSON file for writing.");
    return false;
  }

  if (serializeJsonPretty(doc, file) == 0) { 
    Serial.println("❌ Failed to serialize JSON.");
    file.close();
    return false;
  }

  file.close();
  return true;
}

// Update a JSON field inside a file
bool updateJsonField(fs::FS &fs, const char *pathInput, const char *key, const char *newValue) {
  String path = pathInput;
  path.trim();
  if (!path.startsWith("/")) path = currentDir + path;

  Serial.print("Attempting to update field '");
  Serial.print(key);
  Serial.print("' in file: [");
  Serial.print(path);
  Serial.println("]");

  StaticJsonDocument<JSON_DOC_SIZE> doc;

  if (!loadJsonFile(fs, path.c_str(), doc)) {
    Serial.println("Failed to load JSON file for editing.");
    return false;
  }

  // Update or create the field
  doc[key] = newValue;
  Serial.print("Field '");
  Serial.print(key);
  Serial.print("' updated to: '");
  Serial.print(newValue);
  Serial.println("'");

  if (saveJsonFile(fs, path.c_str(), doc)) {
    Serial.println("✅ JSON file successfully updated.");
    return true;
  } else {
    Serial.println("❌ Failed to save updated JSON file.");
    return false;
  }
}

// --- FILE SYSTEM FUNCTIONS ---

void listDir(fs::FS &fs, const char * dirname) {
  File root = fs.open(dirname);
  if (!root || !root.isDirectory()) {
    Serial.println("Unable to open directory.");
    return;
  }

  bool hasFiles = false;
  File file = root.openNextFile();
  while (file) {
    hasFiles = true;
    if (file.isDirectory()) {
      Serial.printf("     <dir>  %s\n", file.name());
    } else {
      Serial.printf("%10d  %s\n", file.size(), file.name());
    }
    file = root.openNextFile();
  }

  if (!hasFiles) {
    Serial.println("(No files in the file system)");
  }

  Serial.printf("\nUsed space: %d bytes\n", LittleFS.usedBytes());
  Serial.printf("Free space: %d bytes\n", LittleFS.totalBytes() - LittleFS.usedBytes());
  Serial.printf("Total space: %d bytes\n", LittleFS.totalBytes());
  Serial.println();
}

void readFile(fs::FS &fs, const char * pathInput) {
  String path = pathInput;
  path.trim();
  if (!path.startsWith("/")) path = currentDir + path;

  Serial.print("Opening file: [");
  Serial.print(path);
  Serial.println("]");

  File file = fs.open(path, "r");
  if (!file) {
    Serial.println("File not found.");
    return;
  }

  while (file.available()) Serial.write(file.read());
  file.close();
  Serial.println();
}

void deleteFile(fs::FS &fs, const char * pathInput) {
  String path = pathInput;
  path.trim();
  if (!path.startsWith("/")) path = currentDir + path;
  if (fs.remove(path)) {
    Serial.println("File deleted.");
  } else {
    Serial.println("Failed to delete file.");
  }
}

void renameFile(fs::FS &fs, const char * oldNameInput, const char * newNameInput) {
  String oldName = oldNameInput;
  String newName = newNameInput;
  oldName.trim();
  newName.trim();
  if (!oldName.startsWith("/")) oldName = currentDir + oldName;
  if (!newName.startsWith("/")) newName = currentDir + newName;

  if (fs.rename(oldName.c_str(), newName.c_str())) {
    Serial.println("File renamed successfully.");
  } else {
    Serial.println("Failed to rename file.");
  }
}

void moveFile(fs::FS &fs, const char * fromInput, const char * toInput) {
  String from = fromInput;
  String to = toInput;
  from.trim(); to.trim();

  if (!from.startsWith("/")) from = currentDir + from;
  if (!to.startsWith("/")) to = currentDir + to;

  File testDir = fs.open(to);
  if (to.endsWith("/") || (testDir && testDir.isDirectory())) {
    int slashIndex = from.lastIndexOf('/');
    String fileName = from.substring(slashIndex + 1);
    if (!to.endsWith("/")) to += "/";
    to += fileName;
  }

  if (fs.rename(from.c_str(), to.c_str())) {
    Serial.println("File moved successfully.");
  } else {
    Serial.println("Failed to move file.");
  }
}

void createDir(fs::FS &fs, const char * pathInput) {
  String path = pathInput;
  path.trim();
  if (!path.startsWith("/")) path = currentDir + path;
  if (fs.mkdir(path)) {
    Serial.println("Directory created successfully.");
  } else {
    Serial.println("Failed to create directory.");
  }
}

void removeDir(fs::FS &fs, const char * pathInput) {
  String path = pathInput;
  path.trim();
  if (!path.startsWith("/")) path = currentDir + path;
  if (fs.rmdir(path)) {
    Serial.println("Directory removed successfully.");
  } else {
    Serial.println("Failed to remove directory (must be empty). ");
  }
}

void clearScreen() {
  Serial.write(27);     // ESC
  Serial.print("[2J");  // Clear screen
  Serial.write(27);
  Serial.print("[H");   // Move cursor to top

  for (int i = 0; i < 50; i++) Serial.println();
}
