#ifndef DEBUG_FRAMEWORK_H
#define DEBUG_FRAMEWORK_H

#include <Arduino.h>
#include <Stream.h>

// Debug levels - higher numbers mean more verbose output
#define DEBUG_LEVEL_NONE    0
#define DEBUG_LEVEL_ERROR   1
#define DEBUG_LEVEL_WARN    2
#define DEBUG_LEVEL_INFO    3
#define DEBUG_LEVEL_DEBUG   4
#define DEBUG_LEVEL_TRACE   5

// Module definitions for filtering
#define DEBUG_MODULE_MAIN       0x0001
#define DEBUG_MODULE_THERMOSTAT 0x0002
#define DEBUG_MODULE_FCU        0x0004
#define DEBUG_MODULE_MODBUS     0x0008
#define DEBUG_MODULE_SENSOR     0x0010
#define DEBUG_MODULE_LED        0x0020
#define DEBUG_MODULE_ALL        0xFFFF

class DebugFramework {
private:
    Stream* debugStream;
    uint8_t debugLevel;
    uint16_t enabledModules;
    bool showTimestamp;
    bool showModule;
    bool showLevel;
    
    // Internal helper methods
    void printTimestamp();
    void printModule(uint16_t module);
    void printLevel(uint8_t level);
    bool shouldPrint(uint8_t level, uint16_t module);
    
public:
    DebugFramework();
    
    // Configuration methods
    void setDebugStream(Stream* stream);
    void setDebugLevel(uint8_t level);
    void enableModule(uint16_t module);
    void disableModule(uint16_t module);
    void setShowTimestamp(bool show);
    void setShowModule(bool show);
    void setShowLevel(bool show);
    
    // Debug output methods
    void error(uint16_t module, const char* message);
    void error(uint16_t module, const char* message, int value);
    void error(uint16_t module, const char* message, float value);
    void error(uint16_t module, const char* message, const char* value);
    
    void warn(uint16_t module, const char* message);
    void warn(uint16_t module, const char* message, int value);
    void warn(uint16_t module, const char* message, float value);
    void warn(uint16_t module, const char* message, const char* value);
    
    void info(uint16_t module, const char* message);
    void info(uint16_t module, const char* message, int value);
    void info(uint16_t module, const char* message, float value);
    void info(uint16_t module, const char* message, const char* value);
    
    void debug(uint16_t module, const char* message);
    void debug(uint16_t module, const char* message, int value);
    void debug(uint16_t module, const char* message, float value);
    void debug(uint16_t module, const char* message, const char* value);
    
    void trace(uint16_t module, const char* message);
    void trace(uint16_t module, const char* message, int value);
    void trace(uint16_t module, const char* message, float value);
    void trace(uint16_t module, const char* message, const char* value);
    
    // Raw output methods (bypass level checking)
    void print(const char* message);
    void print(int value);
    void print(float value);
    void println(const char* message);
    void println(int value);
    void println(float value);
    
    // Utility methods
    void printHex(uint16_t module, const char* label, uint8_t* data, int length);
    void printStatus(uint16_t module, const char* status);
};

// Global debug instance
extern DebugFramework Debug;

// Convenience macros for easier usage
#define DEBUG_ERROR(module, msg) Debug.error(module, msg)
#define DEBUG_ERROR_VAL(module, msg, val) Debug.error(module, msg, val)
#define DEBUG_WARN(module, msg) Debug.warn(module, msg)
#define DEBUG_WARN_VAL(module, msg, val) Debug.warn(module, msg, val)
#define DEBUG_INFO(module, msg) Debug.info(module, msg)
#define DEBUG_INFO_VAL(module, msg, val) Debug.info(module, msg, val)
#define DEBUG_DEBUG(module, msg) Debug.debug(module, msg)
#define DEBUG_DEBUG_VAL(module, msg, val) Debug.debug(module, msg, val)
#define DEBUG_TRACE(module, msg) Debug.trace(module, msg)
#define DEBUG_TRACE_VAL(module, msg, val) Debug.trace(module, msg, val)

// Type-specific macros for better type handling
#define DEBUG_INFO_INT(module, msg, val) Debug.info(module, msg, static_cast<int>(val))
#define DEBUG_INFO_FLOAT(module, msg, val) Debug.info(module, msg, static_cast<float>(val))
#define DEBUG_INFO_STR(module, msg, val) Debug.info(module, msg, val)

#endif // DEBUG_FRAMEWORK_H 