/*
 * AyresShell v1.0.1 (English Version)
 * --------------------------------------------------------
 * Interactive serial console for ESP32
 * Developed by Daniel Cristian Salgado - AyresNet
 * Translation into english and additional commands by Mehmet Ersöz
 * https://github.com/ayresnet/AyresShell
 * --------------------------------------------------------
 * This console allows you to execute custom commands,
 * interact with the file system (LittleFS), and
 * edit JSON files directly from the serial monitor.
 *
 * Compatible with Arduino IDE and PlatformIO.
 * License: MIT
 */

#include "AyresShell.h"

// Current working directory
String currentDir = "/";   

// --- JSON HANDLING FUNCTIONS ---

// JSON document size. Increase if your JSON files are larger/more complex.
const size_t JSON_DOC_SIZE = 256; 

// Read and parse a JSON file
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

  // serializeJsonPretty for formatted output; serializeJson for compact
  if (serializeJsonPretty(doc, file) == 0) { 
    Serial.println("❌ Failed to serialize JSON.");
    file.close();
    return false;
  }

  file.close();
  return true;
}

// Update a specific JSON field inside a file
bool updateJsonField(fs::FS &fs, const char *pathInput, const char *key, const char *newValue) {
  String path = pathInput;
  path.trim();
  if (!path.startsWith("/")) path = currentDir + path; // Handle relative paths

  Serial.print("Attempting to update field '");
  Serial.print(key);
  Serial.print("' in file: [");
  Serial.print(path);
  Serial.println("]");

  StaticJsonDocument<JSON_DOC_SIZE> doc;

  // 1) Load and parse the JSON file
  if (!loadJsonFile(fs, path.c_str(), doc)) {
    Serial.println("Failed to load JSON file for editing.");
    return false;
  }

  // 2) Modify the specific field (created if it doesn't exist)
  doc[key] = newValue;
  Serial.print("Field '");
  Serial.print(key);
  Serial.print("' updated to: '");
  Serial.print(newValue);
  Serial.println("'");

  // 3) Save the modified JsonDocument back to disk
  if (saveJsonFile(fs, path.c_str(), doc)) {
    Serial.println("✅ JSON file updated successfully.");
    return true;
  } else {
    Serial.println("❌ Failed to save the updated JSON file.");
    return false;
  }
}

// --- EXISTING FILE SYSTEM FUNCTIONS ---
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

  Serial.print("Opening: [");
  Serial.print(path);
  Serial.println("]");

  File file = fs.open(path, "r");
  if (!file) {
    Serial.println("File not found.");
    return;
  }

  while (file.available()) Serial.write(file.read());
  file.close();
  Serial.println(); // Newline after reading
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
    Serial.println("Failed to remove directory (is it empty?).");
  }
}

void clearScreen() {
  Serial.write(27);     // ESC
  Serial.print("[2J");  // Clear screen
  Serial.write(27);
  Serial.print("[H");   // Move cursor to top

  for (int i = 0; i < 50; i++) Serial.println();
}

void help() {
  Serial.println("AyresNet Shell v1.0 - Available commands:");
  Serial.println("DIR & LS                   - List files + info");
  Serial.println("TYPE <file> & CAT <file>   - Display file contents");
  Serial.println("DEL <file> & RM <file>     - Delete file");
  Serial.println("REN <a> <b>                - Rename file");
  Serial.println("MV <a> <b>                 - Move file to another folder");
  Serial.println("MKDIR <folder>             - Create directory");
  Serial.println("RMDIR <folder>             - Remove empty directory");
  Serial.println("CD <folder>                - Change directory (.. or / supported)");
  Serial.println("JSONSET <path> <key> \"<value>\" - Edit field in JSON file");
  Serial.println("FORMAT                     - Format LittleFS (ERASES ALL FILES!)");
  Serial.println("CLS & CLEAR                - Clear screen");
  Serial.println("REBOOT                     - Reboots ESP32");
  Serial.println("UPTIME                     - Show uptime since boot");
  Serial.println("FREE                       - Show free heap (and PSRAM if available)");
  Serial.println("CHIPINFO                   - Show ESP32 hardware information");
  Serial.println("VERSION                    - Show firmware version");
  Serial.println("HELP & MAN                 - Show this help message");
  Serial.println();
}

void reboot() {
  ESP.restart();
}

// --- handleSerialCommands() ---
void handleSerialCommands() {
  static String input;
  static bool confirmFormat = false;

  if (Serial.available()) {
    char c = Serial.read();

    if (c == '\n') {
      input.trim();
      String upperInput = input;
      upperInput.toUpperCase();

      if (confirmFormat) {
        confirmFormat = false;
        // Accept 'Y' (English) and 'S' (Spanish) for safety/compatibility
        if (upperInput == "Y" || upperInput == "S") {
          Serial.println("Formatting file system...");
          if (LittleFS.format()) {
            Serial.println("File system formatted successfully.");
          } else {
            Serial.println("Failed to format file system.");
          }
        } else {
          Serial.println("Format canceled.");
        }
      }
      // --- JSONSET command ---
      else if (upperInput.startsWith("JSONSET ")) {
        // Expected format: JSONSET <file_path> <key> "<value>"
        // We need to parse 3 arguments.
        int firstSpace = input.indexOf(' ');               // After "JSONSET"
        int secondSpace = input.indexOf(' ', firstSpace + 1); // After <file_path>
        
        if (firstSpace != -1 && secondSpace != -1) {
          String path = input.substring(firstSpace + 1, secondSpace);
          String remaining = input.substring(secondSpace + 1);

          int keyEndIndex = remaining.indexOf(' ');
          if (keyEndIndex != -1) {
            String key = remaining.substring(0, keyEndIndex);
            String value = remaining.substring(keyEndIndex + 1);

            // Remove quotes if user wrapped the value
            if (value.startsWith("\"") && value.endsWith("\"")) {
              value = value.substring(1, value.length() - 1);
            }
            
            updateJsonField(LittleFS, path.c_str(), key.c_str(), value.c_str());
          } else {
            Serial.println("Usage: JSONSET <file_path> <key> \"<value>\"");
            Serial.println("Ex:    JSONSET /config.json ssid MyNetwork");
            Serial.println("Ex:    JSONSET /creds.json password \"My Secret Password\"");
          }
        } else {
          Serial.println("Usage: JSONSET <file_path> <key> \"<value>\"");
          Serial.println("Ex:    JSONSET /config.json ssid MyNetwork");
          Serial.println("Ex:    JSONSET /creds.json password \"My Secret Password\"");
        }
      }
      // --- END JSONSET ---
      
      else if (upperInput == "DIR" || upperInput == "LS") {
        listDir(LittleFS, currentDir.c_str());
      }

      else if (upperInput.startsWith("TYPE ")) {
        String path = input.substring(4);
        readFile(LittleFS, path.c_str());
      }

       else if ( upperInput.startsWith("CAT ")) {
        String path = input.substring(3);
        readFile(LittleFS, path.c_str());
      }

      else if (upperInput.startsWith("DEL ")) {
        String path = input.substring(4);
        deleteFile(LittleFS, path.c_str());
      }

        else if (upperInput.startsWith("RM ")) {
        String path = input.substring(3);
        deleteFile(LittleFS, path.c_str());
      }

      else if (upperInput.startsWith("REN ")) {
        int sepIndex = input.indexOf(' ', 4);
        if (sepIndex > 0) {
          String oldName = input.substring(4, sepIndex);
          String newName = input.substring(sepIndex + 1);
          renameFile(LittleFS, oldName.c_str(), newName.c_str());
        } else {
          Serial.println("Usage: REN <old> <new>");
        }
      }

      else if (upperInput.startsWith("MV ")) {
        int sepIndex = input.indexOf(' ', 3);
        if (sepIndex > 0) {
          String src = input.substring(3, sepIndex);
          String dst = input.substring(sepIndex + 1);
          moveFile(LittleFS, src.c_str(), dst.c_str());
        } else {
          Serial.println("Usage: MV <source> <destination>");
        }
      }

      else if (upperInput.startsWith("MKDIR ")) {
        String path = input.substring(6);
        createDir(LittleFS, path.c_str());
      }

      else if (upperInput.startsWith("RMDIR ")) {
        String path = input.substring(6);
        removeDir(LittleFS, path.c_str());
      }

      else if (upperInput.startsWith("CD ")) {
        String path = input.substring(3);
        path.trim();

        if (path == "/") {
          currentDir = "/";
        } else if (path == "..") {
          if (currentDir != "/") {
            int lastSlash = currentDir.lastIndexOf('/', currentDir.length() - 2);
            currentDir = currentDir.substring(0, lastSlash + 1);
            if (currentDir.length() == 0) currentDir = "/";
          }
        } else {
          if (!path.startsWith("/")) path = currentDir + path;
          if (!path.endsWith("/")) path += "/";
          File dir = LittleFS.open(path);
          if (dir && dir.isDirectory()) {
            currentDir = path;
          } else {
            Serial.println("Invalid or non-existent directory.");
          }
        }

        Serial.print("Current directory: ");
        Serial.println(currentDir);
      }

      else if (upperInput == "FORMAT") {
        Serial.println("Are you sure you want to format LittleFS? This will ERASE ALL FILES.");
        Serial.print("Type Y to confirm or N to cancel: ");
        confirmFormat = true;
      }

      else if (upperInput == "CLS" || upperInput == "CLEAR") {
        clearScreen();
      }

      else if (upperInput == "HELP" || upperInput == "MAN") {
        help();
      }

       else if (upperInput == "REBOOT") {
        reboot();
      }

      else if (upperInput == "UPTIME") {
        unsigned long ms = millis();
        unsigned long sec = ms / 1000;
        unsigned long min = sec / 60;
        unsigned long hr  = min / 60;
        sec %= 60;
        min %= 60;
        Serial.printf("Uptime: %lu h %lu m %lu s\n", hr, min, sec);
      }

      else if (upperInput == "FREE") {
        Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
      #if defined(CONFIG_SPIRAM_SUPPORT) && CONFIG_SPIRAM_SUPPORT
        Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
      #endif
      }

      else if (upperInput == "CHIPINFO") {
        Serial.println("Chip Information:");
        Serial.printf("  Model: %s\n", ESP.getChipModel());
        Serial.printf("  Cores: %d\n", ESP.getChipCores());
        Serial.printf("  Revision: %d\n", ESP.getChipRevision());
        Serial.printf("  CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
        Serial.printf("  Flash Size: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
        Serial.printf("  Heap Free: %d bytes\n", ESP.getFreeHeap());
      #if defined(CONFIG_SPIRAM_SUPPORT) && CONFIG_SPIRAM_SUPPORT
        Serial.printf("  PSRAM Free: %d bytes\n", ESP.getFreePsram());
      #endif
      }

      else if (upperInput == "VERSION") {
        Serial.print("Firmware: 1.0");
        //Serial.println(FIRMWARE_VERSION); // Optional: Wenn du eine eigene Konstante definierst, z.B.:
      }


      else if (input.length() > 0) {
        Serial.println("Unrecognized command. Type 'HELP'.");
      }

      input = "";
    } else {
      input += c;
    }
  }
}

// ==================== AYRESSHELL CLASS IMPLEMENTATION ====================

void AyresShell::begin() {
  Serial.println("🟢 AyresShell ready. Type HELP to view commands.");
}

void AyresShell::handleInput() {
  handleSerialCommands();  // Call the global handler
}

void AyresShell::addCommand(const String& name, std::function<void(const String&)> callback) {
  // Future functionality. Not implemented yet.
  // You could register custom commands here using a map of handlers.
}
