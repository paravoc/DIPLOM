//==============================================================================
// FaceTurnstile - Configuration Module
// macros.h
//==============================================================================
//
// Core macros for configuration processing:
// - Error checking and validation (CHECK, VALIDATE)
// - YAML node access helpers (GET_YAML, GET_YAML_OR)
// - Memory security (SECURE_ZERO, SECURE_STRING)
// - Compiler hints and portability (LIKELY, DEPRECATED)
// - Debug utilities (DUMP, ASSERT)
//
// All validation macros return std::unexpected on failure,
// enabling clean error propagation without exceptions.
//
// Author:  paravoc
// Created: 19.03.2026
// Version: 1.0.0
//==============================================================================
#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <filesystem>
#include <expected>
#include <string>

// Инициализация логгера (вызвать один раз в main)
#define CONFIG_INIT_LOGGER() \
    do { \
        auto console = spdlog::stdout_color_mt("config"); \
        console->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v"); \
        spdlog::set_default_logger(console); \
    } while(0)

// Версионирование API модуля
#define CONFIG_API_VERSION_MAJOR 1
#define CONFIG_API_VERSION_MINOR 0
#define CONFIG_API_VERSION_PATCH 0

// ====================================================
// Макросы для логирования
// ====================================================
#define CONFIG_LOG_DEBUG(msg) spdlog::debug(msg)
#define CONFIG_LOG_INFO(msg)  spdlog::info(msg)
#define CONFIG_LOG_WARN(msg)  spdlog::warn(msg)
#define CONFIG_LOG_ERROR(msg) spdlog::error(msg)

// ====================================================
// Макросы для проверок и валидации
// ====================================================

// Проверка указателя с возвратом ошибки
#define CONFIG_CHECK(ptr, msg) \
    do { \
        if (!(ptr)) { \
            CONFIG_LOG_ERROR(msg); \
            return std::unexpected(msg); \
        } \
    } while(0)

// Проверка условия с возвратом ошибки
#define CONFIG_VALIDATE(cond, msg) \
    do { \
        if (!(cond)) { \
            CONFIG_LOG_ERROR(msg); \
            return std::unexpected(msg); \
        } \
    } while(0)

// Проверка на null с возвратом альтернативы
#define CONFIG_CHECK_OR(ptr, fallback) \
    ((ptr) ? (ptr) : (fallback))

// Проверка на пустую строку
#define CONFIG_CHECK_EMPTY(str, msg) \
    do { \
        if ((str).empty()) { \
            CONFIG_LOG_ERROR(msg); \
            return std::unexpected(msg); \
        } \
    } while(0)

// Проверка существования файла
#define CONFIG_CHECK_FILE(path, msg) \
    do { \
        if (!std::filesystem::exists(path)) { \
            CONFIG_LOG_ERROR(msg); \
            return std::unexpected(msg); \
        } \
    } while(0)

// ====================================================
// Макросы для работы с YAML
// ====================================================

// Безопасное получение поля YAML с проверкой
#define CONFIG_GET_YAML(node, field, msg) \
    [&]() -> std::expected<YAML::Node, std::string> { \
        if (!node[field]) { \
            CONFIG_LOG_ERROR(msg); \
            return std::unexpected(msg); \
        } \
        return node[field]; \
    }()

// Получение поля с дефолтным значением
#define CONFIG_GET_YAML_OR(node, field, def) \
    (node[field] ? node[field] : def)

// ====================================================
// Макросы для работы с памятью
// ====================================================

// Затирание паролей в памяти (безопасность)
#define CONFIG_SECURE_ZERO(ptr, size) \
    do { \
        if (ptr) { \
            volatile char* p = (volatile char*)(ptr); \
            for (size_t i = 0; i < (size); ++i) { \
                p[i] = 0; \
            } \
        } \
    } while(0)

// Затирание std::string
#define CONFIG_SECURE_STRING(str) \
    do { \
        if (!(str).empty()) { \
            CONFIG_SECURE_ZERO(&(str)[0], (str).size()); \
            (str).clear(); \
        } \
    } while(0)

// ====================================================
// Макросы для версионирования и совместимости
// ====================================================

#ifdef __GNUC__
#define CONFIG_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define CONFIG_DEPRECATED __declspec(deprecated)
#else
#define CONFIG_DEPRECATED
#endif

#ifdef _WIN32
#ifdef CONFIG_BUILD_DLL
#define CONFIG_API __declspec(dllexport)
#else
#define CONFIG_API __declspec(dllimport)
#endif
#else
#define CONFIG_API __attribute__((visibility("default")))
#endif

// ====================================================
// Макросы для отладки
// ====================================================

#define CONFIG_DUMP(var) \
    spdlog::info("{} = {} [{}:{}]", #var, (var), __FILE__, __LINE__)

#ifdef _DEBUG
#define CONFIG_ASSERT(cond) \
        do { \
            if (!(cond)) { \
                CONFIG_LOG_ERROR("Assertion failed: " #cond); \
                std::abort(); \
            } \
        } while(0)
#else
#define CONFIG_ASSERT(cond) ((void)0)
#endif

// ====================================================
// Макросы для производительности
// ====================================================

#ifdef __GNUC__
#define CONFIG_LIKELY(expr)   __builtin_expect(!!(expr), 1)
#define CONFIG_UNLIKELY(expr) __builtin_expect(!!(expr), 0)
#else
#define CONFIG_LIKELY(expr)   (expr)
#define CONFIG_UNLIKELY(expr) (expr)
#endif

#define CONFIG_NON_COPYABLE(ClassName) \
    ClassName(const ClassName&) = delete; \
    ClassName& operator=(const ClassName&) = delete

#define CONFIG_NON_MOVABLE(ClassName) \
    ClassName(ClassName&&) = delete; \
    ClassName& operator=(ClassName&&) = delete

static_assert(CONFIG_API_VERSION_MAJOR == 1, "Macros version mismatch");