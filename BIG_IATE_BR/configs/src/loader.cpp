#include "../include/loader.h"
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <filesystem>

namespace bigiate::config {


    // Проверка существования файла
    [[nodiscard]] bool ConfigFileExists(const std::string& config_path) {
        bool res = std::filesystem::exists(std::filesystem::path(config_path));
        return res; 
    }

    // ====================================================
    // ШАГ 2: Загрузка только версии (для теста)
    // ====================================================

    [[nodiscard]] std::expected<Version, std::string>
        LoadVersionOnly(const std::string& config_path) {

        // 1. Проверить что файл есть
        CONFIG_CHECK_FILE(config_path, "Config file not found: " + config_path);

        try {
            // 2. Загрузить YAML
            YAML::Node root = YAML::LoadFile(config_path);

            // 3. Найти поле "version"
            auto version_node = CONFIG_GET_YAML(root, "version", "Version field not found in config");
            if (!version_node) {
                return std::unexpected(version_node.error());
            }

            // 4. Прочитать как строку
            std::string version_str;
            try {
                version_str = version_node->as<std::string>();
            }
            catch (const YAML::Exception& e) {
                CONFIG_LOG_ERROR("Version must be a string");
                return std::unexpected("Version must be a string");
            }

            // 5. Проверить что не пустая
            CONFIG_CHECK_EMPTY(version_str, "Version string is empty");

            // 6. Разобрать "1.0.0" на major.minor.patch
            Version version;
            std::istringstream ss(version_str);
            std::string token;

            std::vector<int> parts;
            while (std::getline(ss, token, '.')) {
                try {
                    parts.push_back(std::stoi(token));
                }
                catch (...) {
                    CONFIG_LOG_ERROR("Version components must be numbers");
                    return std::unexpected("Version components must be numbers");
                }
            }

            if (parts.size() != 3) {
                CONFIG_LOG_ERROR("Invalid version format. Expected major.minor.patch");
                return std::unexpected("Invalid version format. Expected major.minor.patch");
            }

            version.major = parts[0];
            version.minor = parts[1];
            version.patch = parts[2];


            // 7. Успех - возвращаем версию
            CONFIG_LOG_INFO("Loaded config version: " + version.toString());
            return version;

        }
        catch (const YAML::Exception& e) {
            CONFIG_LOG_ERROR(std::string("YAML parse error: ") + e.what());
            return std::unexpected(std::string("YAML parse error: ") + e.what());
        }
        catch (const std::exception& e) {
            CONFIG_LOG_ERROR(std::string("Unexpected error: ") + e.what());
            return std::unexpected(std::string("Unexpected error: ") + e.what());
        }
    }

    // ====================================================
    // ШАГ 3: Парсинг отдельных секций
    // ====================================================

    // Парсинг одной камеры из YAML узла
    [[nodiscard]] std::expected<CameraConfig, std::string>
        ParseCamera(const YAML::Node& node) {
        CameraConfig camera;

        // 1. Прочитать id (обязательное)
        if (!node["id"] || !node["id"].IsScalar()) {
            return std::unexpected("Camera missing required field: 'id'");
        }
        camera.id = node["id"].as<int>();  // id это int

        // 2. Прочитать name (обязательное)
        if (!node["name"] || !node["name"].IsScalar()) {
            return std::unexpected("Camera missing required field: 'name'");
        }
        camera.name = node["name"].as<std::string>();

        // 3. Прочитать enabled (опционально, дефолт true)
        if (node["enabled"] && node["enabled"].IsScalar()) {
            camera.enabled = node["enabled"].as<bool>();
        }
        // иначе оставляем true (уже установлено в структуре)

        // 4. Прочитать connection (опционально, но если есть - парсим)
        if (node["connection"] && node["connection"].IsMap()) {
            const auto& conn_node = node["connection"];

            // protocol (опционально)
            if (conn_node["protocol"] && conn_node["protocol"].IsScalar()) {
                camera.connection.protocol = conn_node["protocol"].as<std::string>();
            }

            // host (обязательное для network камер)
            if (conn_node["host"] && conn_node["host"].IsScalar()) {
                camera.connection.host = conn_node["host"].as<std::string>();
            }
            else if (camera.connection.protocol != "usb") {
                // Если не USB, то host обязателен
                return std::unexpected("Camera " + std::to_string(camera.id) +
                    ": 'host' required for protocol '" +
                    camera.connection.protocol + "'");
            }

            // port (опционально)
            if (conn_node["port"] && conn_node["port"].IsScalar()) {
                camera.connection.port = conn_node["port"].as<int>();
            }

            // path (опционально)
            if (conn_node["path"] && conn_node["path"].IsScalar()) {
                camera.connection.path = conn_node["path"].as<std::string>();
            }

            // device (опционально, только для USB)
            if (conn_node["device"] && conn_node["device"].IsScalar()) {
                camera.connection.device = conn_node["device"].as<std::string>();
            }
        }

        // 5. Прочитать capture (опционально, но если есть - парсим)
        if (node["capture"] && node["capture"].IsMap()) {
            const auto& cap_node = node["capture"];

            // fps (опционально)
            if (cap_node["fps"] && cap_node["fps"].IsScalar()) {
                camera.capture.fps = cap_node["fps"].as<int>();
            }

            // width (опционально)
            if (cap_node["width"] && cap_node["width"].IsScalar()) {
                camera.capture.width = cap_node["width"].as<int>();
            }

            // height (опционально)
            if (cap_node["height"] && cap_node["height"].IsScalar()) {
                camera.capture.height = cap_node["height"].as<int>();
            }

            // rotation (опционально)
            if (cap_node["rotation"] && cap_node["rotation"].IsScalar()) {
                camera.capture.rotation = cap_node["rotation"].as<int>();
            }
        }

        return camera;
    }

    // Парсинг DatabaseConfig для PostgreSQL
    [[nodiscard]] std::expected<DatabaseConfig, std::string>
        ParseDatabase(const YAML::Node& node) {
        DatabaseConfig db;

        if (!node.IsMap()) {
            return std::unexpected("Database configuration must be a YAML map");
        }

        // Обязательные поля
        PARSE_REQUIRED(node, "username", username, std::string);

        // Опциональные поля с дефолтами
        PARSE_OPTIONAL(node, "host", host, std::string, "localhost");
        PARSE_OPTIONAL(node, "port", port, int, 5432);
        PARSE_OPTIONAL(node, "name", name, std::string, "face_db");
        PARSE_OPTIONAL(node, "schema", schema, std::string, "public");

        // Парсинг pool (опционально)
        if (node["pool"] && node["pool"].IsMap()) {
            const auto& pool_node = node["pool"];

            PARSE_OPTIONAL(pool_node, "min_connections", pool.min_connections, int, 2);
            PARSE_OPTIONAL(pool_node, "max_connections", pool.max_connections, int, 10);
            PARSE_OPTIONAL(pool_node, "connection_timeout_seconds", pool.connection_timeout_seconds, int, 5);
            PARSE_OPTIONAL(pool_node, "idle_timeout_seconds", pool.idle_timeout_seconds, int, 60);
        }

        // Парсинг vector (опционально)
        if (node["vector"] && node["vector"].IsMap()) {
            const auto& vector_node = node["vector"];

            PARSE_OPTIONAL(vector_node, "dimension", vector.dimension, int, 512);
            PARSE_OPTIONAL(vector_node, "similarity_threshold", vector.similarity_threshold, double, 0.75);
            PARSE_OPTIONAL(vector_node, "index_type", vector.index_type, std::string, "ivfflat");
        }

        return db;
    }

    // Парсинг BastionConfig
    [[nodiscard]] std::expected<BastionConfig, std::string>
        ParseBastion(const YAML::Node& node) {
        BastionConfig bastion;
        // Заполнить все поля
        return bastion;
    }

    // Парсинг RecognitionConfig
    [[nodiscard]] std::expected<RecognitionConfig, std::string>
        ParseRecognition(const YAML::Node& node) {
        RecognitionConfig recognition;
        // Заполнить все поля (SSD, ArcFace, matching, anti_passback)
        return recognition;
    }

    // Парсинг LoggingConfig
    [[nodiscard]] std::expected<LoggingConfig, std::string>
        ParseLogging(const YAML::Node& node) {
        LoggingConfig logging;
        // Заполнить все поля
        return logging;
    }

    // Парсинг SecurityConfig
    [[nodiscard]] std::expected<SecurityConfig, std::string>
        ParseSecurity(const YAML::Node& node) {
        SecurityConfig security;
        // Заполнить все поля
        return security;
    }

    // ====================================================
    // ШАГ 4: Валидация (после загрузки)
    // ====================================================

    [[nodiscard]] std::expected<void, std::string>
        ValidateConfig(const ServerConfig& config) {
        // 1. Проверить что есть хотя бы одна камера
        // 2. Проверить уникальность id камер
        // 3. Проверить что порты в диапазоне
        // 4. Проверить что пути к моделям указаны
        return {};
    }

    // ====================================================
    // ШАГ 5: Главная функция (собирает всё вместе)
    // ====================================================

    [[nodiscard]] std::expected<LoadResult, std::string>
        LoadConfig(const std::string& config_path) {
        LoadResult result;

        // 1. Проверить файл
        if (!ConfigFileExists(config_path)) {
            return std::unexpected("Config file not found: " + config_path);
        }

        try {
            // 2. Загрузить YAML
            YAML::Node root = YAML::LoadFile(config_path);

            // 3. Загрузить версию
            auto version_result = LoadVersionOnly(config_path);
            if (!version_result) {
                return std::unexpected(version_result.error());
            }
            result.config.version = *version_result;

            // 4. Парсить все секции по очереди
            // - cameras (цикл по массиву)
            // - database
            // - bastion
            // - recognition
            // - logging
            // - security
            // - web

            // 5. Валидация
            auto validation = ValidateConfig(result.config);
            if (!validation) {
                return std::unexpected(validation.error());
            }

            // 6. TODO: Загрузка secrets (потом)
            result.secrets = Secrets{};
            result.success = true;

            return result;

        }
        catch (const YAML::Exception& e) {
            return std::unexpected(std::string("YAML parse error: ") + e.what());
        }
        catch (const std::exception& e) {
            return std::unexpected(std::string("Error: ") + e.what());
        }
    }

    // ====================================================
    // ШАГ 6: Отладочная печать
    // ====================================================

    void DumpConfig(const ServerConfig& config) {
        // Распечатать основные поля конфига
        // - версию
        // - количество камер
        // - хост БД
        // - и т.д.
    }

} // namespace bigiate::config