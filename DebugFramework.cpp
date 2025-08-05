#include "DebugFramework.h"

// Global debug instance
DebugFramework Debug;

DebugFramework::DebugFramework() {
    debugStream = nullptr;
    debugLevel = DEBUG_LEVEL_INFO;
    enabledModules = DEBUG_MODULE_ALL;
    showTimestamp = true;
    showModule = true;
    showLevel = true;
}

void DebugFramework::setDebugStream(Stream* stream) {
    debugStream = stream;
}

void DebugFramework::setDebugLevel(uint8_t level) {
    debugLevel = level;
}

void DebugFramework::enableModule(uint16_t module) {
    enabledModules |= module;
}

void DebugFramework::disableModule(uint16_t module) {
    enabledModules &= ~module;
}

void DebugFramework::setShowTimestamp(bool show) {
    showTimestamp = show;
}

void DebugFramework::setShowModule(bool show) {
    showModule = show;
}

void DebugFramework::setShowLevel(bool show) {
    showLevel = show;
}

bool DebugFramework::shouldPrint(uint8_t level, uint16_t module) {
    if (debugStream == nullptr) return false;
    if (level > debugLevel) return false;
    if ((enabledModules & module) == 0) return false;
    return true;
}

void DebugFramework::printTimestamp() {
    if (!showTimestamp || debugStream == nullptr) return;
    
    unsigned long now = millis();
    unsigned long seconds = now / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    
    seconds %= 60;
    minutes %= 60;
    
    debugStream->print("[");
    if (hours < 10) debugStream->print("0");
    debugStream->print(hours);
    debugStream->print(":");
    if (minutes < 10) debugStream->print("0");
    debugStream->print(minutes);
    debugStream->print(":");
    if (seconds < 10) debugStream->print("0");
    debugStream->print(seconds);
    debugStream->print("] ");
}

void DebugFramework::printModule(uint16_t module) {
    if (!showModule || debugStream == nullptr) return;
    
    debugStream->print("[");
    switch (module) {
        case DEBUG_MODULE_MAIN:
            debugStream->print("MAIN");
            break;
        case DEBUG_MODULE_THERMOSTAT:
            debugStream->print("THERM");
            break;
        case DEBUG_MODULE_FCU:
            debugStream->print("FCU");
            break;
        case DEBUG_MODULE_MODBUS:
            debugStream->print("MODBUS");
            break;
        case DEBUG_MODULE_SENSOR:
            debugStream->print("SENSOR");
            break;
        case DEBUG_MODULE_LED:
            debugStream->print("LED");
            break;
        default:
            debugStream->print("UNKNOWN");
            break;
    }
    debugStream->print("] ");
}

void DebugFramework::printLevel(uint8_t level) {
    if (!showLevel || debugStream == nullptr) return;
    
    debugStream->print("[");
    switch (level) {
        case DEBUG_LEVEL_ERROR:
            debugStream->print("ERROR");
            break;
        case DEBUG_LEVEL_WARN:
            debugStream->print("WARN");
            break;
        case DEBUG_LEVEL_INFO:
            debugStream->print("INFO");
            break;
        case DEBUG_LEVEL_DEBUG:
            debugStream->print("DEBUG");
            break;
        case DEBUG_LEVEL_TRACE:
            debugStream->print("TRACE");
            break;
        default:
            debugStream->print("UNKNOWN");
            break;
    }
    debugStream->print("] ");
}

// Error level methods
void DebugFramework::error(uint16_t module, const char* message) {
    if (!shouldPrint(DEBUG_LEVEL_ERROR, module)) return;
    printTimestamp();
    printModule(module);
    printLevel(DEBUG_LEVEL_ERROR);
    debugStream->println(message);
}

void DebugFramework::error(uint16_t module, const char* message, int value) {
    if (!shouldPrint(DEBUG_LEVEL_ERROR, module)) return;
    printTimestamp();
    printModule(module);
    printLevel(DEBUG_LEVEL_ERROR);
    debugStream->print(message);
    debugStream->print(": ");
    debugStream->println(value);
}

void DebugFramework::error(uint16_t module, const char* message, float value) {
    if (!shouldPrint(DEBUG_LEVEL_ERROR, module)) return;
    printTimestamp();
    printModule(module);
    printLevel(DEBUG_LEVEL_ERROR);
    debugStream->print(message);
    debugStream->print(": ");
    debugStream->println(value);
}

void DebugFramework::error(uint16_t module, const char* message, const char* value) {
    if (!shouldPrint(DEBUG_LEVEL_ERROR, module)) return;
    printTimestamp();
    printModule(module);
    printLevel(DEBUG_LEVEL_ERROR);
    debugStream->print(message);
    debugStream->print(": ");
    debugStream->println(value);
}

// Warning level methods
void DebugFramework::warn(uint16_t module, const char* message) {
    if (!shouldPrint(DEBUG_LEVEL_WARN, module)) return;
    printTimestamp();
    printModule(module);
    printLevel(DEBUG_LEVEL_WARN);
    debugStream->println(message);
}

void DebugFramework::warn(uint16_t module, const char* message, int value) {
    if (!shouldPrint(DEBUG_LEVEL_WARN, module)) return;
    printTimestamp();
    printModule(module);
    printLevel(DEBUG_LEVEL_WARN);
    debugStream->print(message);
    debugStream->print(": ");
    debugStream->println(value);
}

void DebugFramework::warn(uint16_t module, const char* message, float value) {
    if (!shouldPrint(DEBUG_LEVEL_WARN, module)) return;
    printTimestamp();
    printModule(module);
    printLevel(DEBUG_LEVEL_WARN);
    debugStream->print(message);
    debugStream->print(": ");
    debugStream->println(value);
}

void DebugFramework::warn(uint16_t module, const char* message, const char* value) {
    if (!shouldPrint(DEBUG_LEVEL_WARN, module)) return;
    printTimestamp();
    printModule(module);
    printLevel(DEBUG_LEVEL_WARN);
    debugStream->print(message);
    debugStream->print(": ");
    debugStream->println(value);
}

// Info level methods
void DebugFramework::info(uint16_t module, const char* message) {
    if (!shouldPrint(DEBUG_LEVEL_INFO, module)) return;
    printTimestamp();
    printModule(module);
    printLevel(DEBUG_LEVEL_INFO);
    debugStream->println(message);
}

void DebugFramework::info(uint16_t module, const char* message, int value) {
    if (!shouldPrint(DEBUG_LEVEL_INFO, module)) return;
    printTimestamp();
    printModule(module);
    printLevel(DEBUG_LEVEL_INFO);
    debugStream->print(message);
    debugStream->print(": ");
    debugStream->println(value);
}

void DebugFramework::info(uint16_t module, const char* message, float value) {
    if (!shouldPrint(DEBUG_LEVEL_INFO, module)) return;
    printTimestamp();
    printModule(module);
    printLevel(DEBUG_LEVEL_INFO);
    debugStream->print(message);
    debugStream->print(": ");
    debugStream->println(value);
}

void DebugFramework::info(uint16_t module, const char* message, const char* value) {
    if (!shouldPrint(DEBUG_LEVEL_INFO, module)) return;
    printTimestamp();
    printModule(module);
    printLevel(DEBUG_LEVEL_INFO);
    debugStream->print(message);
    debugStream->print(": ");
    debugStream->println(value);
}

// Debug level methods
void DebugFramework::debug(uint16_t module, const char* message) {
    if (!shouldPrint(DEBUG_LEVEL_DEBUG, module)) return;
    printTimestamp();
    printModule(module);
    printLevel(DEBUG_LEVEL_DEBUG);
    debugStream->println(message);
}

void DebugFramework::debug(uint16_t module, const char* message, int value) {
    if (!shouldPrint(DEBUG_LEVEL_DEBUG, module)) return;
    printTimestamp();
    printModule(module);
    printLevel(DEBUG_LEVEL_DEBUG);
    debugStream->print(message);
    debugStream->print(": ");
    debugStream->println(value);
}

void DebugFramework::debug(uint16_t module, const char* message, float value) {
    if (!shouldPrint(DEBUG_LEVEL_DEBUG, module)) return;
    printTimestamp();
    printModule(module);
    printLevel(DEBUG_LEVEL_DEBUG);
    debugStream->print(message);
    debugStream->print(": ");
    debugStream->println(value);
}

void DebugFramework::debug(uint16_t module, const char* message, const char* value) {
    if (!shouldPrint(DEBUG_LEVEL_DEBUG, module)) return;
    printTimestamp();
    printModule(module);
    printLevel(DEBUG_LEVEL_DEBUG);
    debugStream->print(message);
    debugStream->print(": ");
    debugStream->println(value);
}

// Trace level methods
void DebugFramework::trace(uint16_t module, const char* message) {
    if (!shouldPrint(DEBUG_LEVEL_TRACE, module)) return;
    printTimestamp();
    printModule(module);
    printLevel(DEBUG_LEVEL_TRACE);
    debugStream->println(message);
}

void DebugFramework::trace(uint16_t module, const char* message, int value) {
    if (!shouldPrint(DEBUG_LEVEL_TRACE, module)) return;
    printTimestamp();
    printModule(module);
    printLevel(DEBUG_LEVEL_TRACE);
    debugStream->print(message);
    debugStream->print(": ");
    debugStream->println(value);
}

void DebugFramework::trace(uint16_t module, const char* message, float value) {
    if (!shouldPrint(DEBUG_LEVEL_TRACE, module)) return;
    printTimestamp();
    printModule(module);
    printLevel(DEBUG_LEVEL_TRACE);
    debugStream->print(message);
    debugStream->print(": ");
    debugStream->println(value);
}

void DebugFramework::trace(uint16_t module, const char* message, const char* value) {
    if (!shouldPrint(DEBUG_LEVEL_TRACE, module)) return;
    printTimestamp();
    printModule(module);
    printLevel(DEBUG_LEVEL_TRACE);
    debugStream->print(message);
    debugStream->print(": ");
    debugStream->println(value);
}

// Raw output methods (bypass level checking)
void DebugFramework::print(const char* message) {
    if (debugStream != nullptr) {
        debugStream->print(message);
    }
}

void DebugFramework::print(int value) {
    if (debugStream != nullptr) {
        debugStream->print(value);
    }
}

void DebugFramework::print(float value) {
    if (debugStream != nullptr) {
        debugStream->print(value);
    }
}

void DebugFramework::println(const char* message) {
    if (debugStream != nullptr) {
        debugStream->println(message);
    }
}

void DebugFramework::println(int value) {
    if (debugStream != nullptr) {
        debugStream->println(value);
    }
}

void DebugFramework::println(float value) {
    if (debugStream != nullptr) {
        debugStream->println(value);
    }
}

// Utility methods
void DebugFramework::printHex(uint16_t module, const char* label, uint8_t* data, int length) {
    if (!shouldPrint(DEBUG_LEVEL_DEBUG, module)) return;
    
    printTimestamp();
    printModule(module);
    printLevel(DEBUG_LEVEL_DEBUG);
    debugStream->print(label);
    debugStream->print(": ");
    
    for (int i = 0; i < length; i++) {
        if (data[i] < 16) debugStream->print("0");
        debugStream->print(data[i], HEX);
        debugStream->print(" ");
    }
    debugStream->println();
}

void DebugFramework::printStatus(uint16_t module, const char* status) {
    if (!shouldPrint(DEBUG_LEVEL_INFO, module)) return;
    
    printTimestamp();
    printModule(module);
    printLevel(DEBUG_LEVEL_INFO);
    debugStream->print("Status: ");
    debugStream->println(status);
} 