#ifndef DEBUG_LIBRARY_H
#define DEBUG_LIBRARY_H

#include <Arduino.h>
#include <Stream.h>

// Include the full debug framework definition
#include "DebugFramework.h"

// Debug levels
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

// Library-specific module definitions (can be extended by libraries)
#define DEBUG_MODULE_LIBRARY_BASE 0x0100
#define DEBUG_MODULE_CUSTOM_1     0x0101
#define DEBUG_MODULE_CUSTOM_2     0x0102
#define DEBUG_MODULE_CUSTOM_3     0x0103
#define DEBUG_MODULE_CUSTOM_4     0x0104

// Convenience macros for easier usage in libraries
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

// Raw output macros
#define DEBUG_PRINT(msg) Debug.print(msg)
#define DEBUG_PRINT_VAL(val) Debug.print(val)
#define DEBUG_PRINTLN(msg) Debug.println(msg)
#define DEBUG_PRINTLN_VAL(val) Debug.println(val)

// Utility macros
#define DEBUG_HEX(module, label, data, length) Debug.printHex(module, label, data, length)
#define DEBUG_STATUS(module, status) Debug.printStatus(module, status)

// Type-specific macros for better type handling
#define DEBUG_INFO_INT(module, msg, val) Debug.info(module, msg, static_cast<int>(val))
#define DEBUG_INFO_FLOAT(module, msg, val) Debug.info(module, msg, static_cast<float>(val))
#define DEBUG_INFO_STR(module, msg, val) Debug.info(module, msg, val)

// Conditional debug macros (only compile if DEBUG is defined)
#ifdef DEBUG
    #define DEBUG_IF_ERROR(module, msg) DEBUG_ERROR(module, msg)
    #define DEBUG_IF_WARN(module, msg) DEBUG_WARN(module, msg)
    #define DEBUG_IF_INFO(module, msg) DEBUG_INFO(module, msg)
    #define DEBUG_IF_DEBUG(module, msg) DEBUG_DEBUG(module, msg)
    #define DEBUG_IF_TRACE(module, msg) DEBUG_TRACE(module, msg)
#else
    #define DEBUG_IF_ERROR(module, msg)
    #define DEBUG_IF_WARN(module, msg)
    #define DEBUG_IF_INFO(module, msg)
    #define DEBUG_IF_DEBUG(module, msg)
    #define DEBUG_IF_TRACE(module, msg)
#endif

#endif // DEBUG_LIBRARY_H 