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

// Для обязательных полей
#define PARSE_REQUIRED(node, field, member, type) \
    do { \
        if (!node[field] || !node[field].IsScalar()) { \
            return std::unexpected("Database missing required field: '" field "'"); \
        } \
        db.member = node[field].as<type>(); \
    } while(0)

// Для опциональных полей с дефолтом
#define PARSE_OPTIONAL(obj, node, field, member, type, default_value) \
    do { \
        if (node[#field] && node[#field].IsScalar()) { \
            obj.member = node[#field].as<type>(); \
        } else { \
            obj.member = default_value; \
        } \
    } while(0)

// Для опциональных полей без дефолта (оставляем как есть)
#define PARSE_OPTIONAL_NODEFAULT(node, field, member, type) \
    do { \
        if (node[#field] && node[#field].IsScalar()) { \
            db.member = node[#field].as<type>(); \
        } \
    } while(0)


// Для обязательных секций в LoadConfig
#define PARSE_REQUIRED_SECTION(section_name, parse_func, target_field) \
    do { \
        if (root[#section_name]) { \
            auto parse_result = parse_func(root[#section_name]); \
            if (!parse_result) { \
                return std::unexpected(#section_name " error: " + parse_result.error()); \
            } \
            result.config.target_field = *parse_result; \
        } else { \
            return std::unexpected("Missing required section: '" #section_name "'"); \
        } \
    } while(0)

// Для опциональных секций в LoadConfig
#define PARSE_OPTIONAL_SECTION(section_name, parse_func, target_field) \
    do { \
        if (root[#section_name]) { \
            auto parse_result = parse_func(root[#section_name]); \
            if (!parse_result) { \
                return std::unexpected(#section_name " error: " + parse_result.error()); \
            } \
            result.config.target_field = *parse_result; \
        } \
    } while(0)

// Для парсинга простых скалярных полей
#define PARSE_SCALAR_FIELD(field_name, target_field, type) \
    do { \
        if (root[#field_name] && root[#field_name].IsScalar()) { \
            result.config.target_field = root[#field_name].as<type>(); \
        } \
    } while(0)

// Для парсинга массива камер
#define PARSE_CAMERAS() \
    do { \
        if (root["cameras"] && root["cameras"].IsSequence()) { \
            for (size_t i = 0; i < root["cameras"].size(); ++i) { \
                auto camera_result = ParseCamera(root["cameras"][i]); \
                if (!camera_result) { \
                    return std::unexpected("Camera " + std::to_string(i + 1) + \
                                          " error: " + camera_result.error()); \
                } \
                result.config.cameras.push_back(*camera_result); \
            } \
            if (result.config.cameras.empty()) { \
                return std::unexpected("At least one camera must be configured"); \
            } \
        } else { \
            return std::unexpected("Missing required section: 'cameras' (must be a sequence)"); \
        } \
    } while(0)

#define CHECK_FILE(path, msg) \
    do { \
        if (!std::filesystem::exists(path)) { \
            return std::unexpected(msg); \
        } \
    } while(0)


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