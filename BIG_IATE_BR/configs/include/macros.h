//==============================================================================
// BIG IATE - Configuration Macros
// macros.h
//==============================================================================
// Описание: Макросы для упрощения парсинга YAML, проверок и работы с памятью.
//           Все макросы возвращают std::unexpected при ошибке.
//
// Автор: paravoc
// Дата: 21.03.2026
// Версия: 1.0.0
//==============================================================================

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <filesystem>
#include <expected>
#include <string>
#include <yaml-cpp/yaml.h>

//==============================================================================
// ПАРСИНГ YAML
//==============================================================================

// Обязательное поле (если отсутствует или не скаляр — ошибка)
#define PARSE_REQUIRED(node, field, member, type) \
    do { \
        if (!node[field] || !node[field].IsScalar()) { \
            return std::unexpected("Missing required field: '" field "'"); \
        } \
        db.member = node[field].as<type>(); \
    } while(0)

// Опциональное поле с значением по умолчанию
#define PARSE_OPTIONAL(obj, node, field, member, type, default_value) \
    do { \
        if (node[#field] && node[#field].IsScalar()) { \
            obj.member = node[#field].as<type>(); \
        } else { \
            obj.member = default_value; \
        } \
    } while(0)

// Обязательная секция в LoadConfig
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

// Опциональная секция в LoadConfig
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

// Скалярное поле (опционально)
#define PARSE_SCALAR_FIELD(field_name, target_field, type) \
    do { \
        if (root[#field_name] && root[#field_name].IsScalar()) { \
            result.config.target_field = root[#field_name].as<type>(); \
        } \
    } while(0)

// Массив камер
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

//==============================================================================
// ПРОВЕРКИ
//==============================================================================

// Проверка существования файла
#define CHECK_FILE(path, msg) \
    do { \
        if (!std::filesystem::exists(path)) { \
            return std::unexpected(msg); \
        } \
    } while(0)

// Проверка на пустую строку
#define CHECK_EMPTY(str, msg) \
    do { \
        if ((str).empty()) { \
            return std::unexpected(msg); \
        } \
    } while(0)

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

//==============================================================================
// ЛОГИРОВАНИЕ (через spdlog)
//==============================================================================

#define CONFIG_INIT_LOGGER() \
    do { \
        auto console = spdlog::stdout_color_mt("config"); \
        console->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v"); \
        spdlog::set_default_logger(console); \
    } while(0)

#define CONFIG_LOG_DEBUG(msg) spdlog::debug(msg)
#define CONFIG_LOG_INFO(msg)  spdlog::info(msg)
#define CONFIG_LOG_WARN(msg)  spdlog::warn(msg)
#define CONFIG_LOG_ERROR(msg) spdlog::error(msg)

//==============================================================================
// YAML ВСПОМОГАТЕЛЬНЫЕ МАКРОСЫ
//==============================================================================

// Безопасное получение поля YAML
#define CONFIG_GET_YAML(node, field, msg) \
    [&]() -> std::expected<YAML::Node, std::string> { \
        if (!node[field]) { \
            CONFIG_LOG_ERROR(msg); \
            return std::unexpected(msg); \
        } \
        return node[field]; \
    }()

//==============================================================================
// БЕЗОПАСНОСТЬ ПАМЯТИ
//==============================================================================

// Затирание паролей в памяти
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

//==============================================================================
// ОТЛАДКА
//==============================================================================

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

//==============================================================================
// ВЕРСИОНИРОВАНИЕ
//==============================================================================

#define CONFIG_API_VERSION_MAJOR 1
#define CONFIG_API_VERSION_MINOR 0
#define CONFIG_API_VERSION_PATCH 0

static_assert(CONFIG_API_VERSION_MAJOR == 1, "Macros version mismatch");