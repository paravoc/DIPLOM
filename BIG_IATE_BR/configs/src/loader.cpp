//==============================================================================
// BIG IATE - Configuration Loader Implementation
// loader.cpp
//==============================================================================
// Описание: Реализация парсинга YAML-конфигурации.
//
// Автор: paravoc
// Дата: 21.03.2026
// Версия: 1.0.0
//==============================================================================

#include "../include/loader.h"
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <filesystem>
#include <unordered_set>
#include <sstream>

namespace bigiate::config {

    //==============================================================================
    // ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
    //==============================================================================

    [[nodiscard]] bool ConfigFileExists(const std::string& config_path) {
        return std::filesystem::exists(std::filesystem::path(config_path));
    }

    //==============================================================================
    // ЗАГРУЗКА ТОЛЬКО ВЕРСИИ
    //==============================================================================

    [[nodiscard]] std::expected<Version, std::string>
        LoadVersionOnly(const std::string& config_path) {
        CHECK_FILE(config_path, "Config file not found: " + config_path);

        try {
            YAML::Node root = YAML::LoadFile(config_path);

            auto version_node = CONFIG_GET_YAML(root, "version", "Version field not found in config");
            if (!version_node) {
                return std::unexpected(version_node.error());
            }

            std::string version_str;
            try {
                version_str = version_node->as<std::string>();
            }
            catch (const YAML::Exception&) {
                CONFIG_LOG_ERROR("Version must be a string");
                return std::unexpected("Version must be a string");
            }

            CHECK_EMPTY(version_str, "Version string is empty");

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

    //==============================================================================
    // ПАРСИНГ КАМЕРЫ
    //==============================================================================

    [[nodiscard]] std::expected<CameraConfig, std::string>
        ParseCamera(const YAML::Node& node) {
        CameraConfig camera;

        // ID — обязательное поле
        if (!node["id"] || !node["id"].IsScalar()) {
            return std::unexpected("Camera missing required field: 'id'");
        }
        camera.id = node["id"].as<int>();

        // Имя — обязательное поле
        if (!node["name"] || !node["name"].IsScalar()) {
            return std::unexpected("Camera missing required field: 'name'");
        }
        camera.name = node["name"].as<std::string>();

        // Enabled — опционально, по умолчанию true
        PARSE_OPTIONAL(camera, node, enabled, enabled, bool, true);

        // Connection — опционально
        if (node["connection"] && node["connection"].IsMap()) {
            const auto& conn_node = node["connection"];

            PARSE_OPTIONAL(camera.connection, conn_node, protocol, protocol, std::string, "rtsp");

            // Host — обязателен для сетевых камер
            if (conn_node["host"] && conn_node["host"].IsScalar()) {
                camera.connection.host = conn_node["host"].as<std::string>();
            }
            else if (camera.connection.protocol != "usb") {
                return std::unexpected("Camera " + std::to_string(camera.id) +
                    ": 'host' required for protocol '" + camera.connection.protocol + "'");
            }

            PARSE_OPTIONAL(camera.connection, conn_node, port, port, int, 554);
            PARSE_OPTIONAL(camera.connection, conn_node, path, path, std::string, "/");

            // Device — только для USB
            if (conn_node["device"] && conn_node["device"].IsScalar()) {
                camera.connection.device = conn_node["device"].as<std::string>();
            }
        }

        // Capture — опционально
        if (node["capture"] && node["capture"].IsMap()) {
            const auto& cap_node = node["capture"];

            PARSE_OPTIONAL(camera.capture, cap_node, fps, fps, int, 15);
            PARSE_OPTIONAL(camera.capture, cap_node, width, width, int, 1280);
            PARSE_OPTIONAL(camera.capture, cap_node, height, height, int, 720);
            PARSE_OPTIONAL(camera.capture, cap_node, rotation, rotation, int, 0);
        }

        return camera;
    }

    //==============================================================================
    // ПАРСИНГ БАЗЫ ДАННЫХ
    //==============================================================================

    [[nodiscard]] std::expected<DatabaseConfig, std::string>
        ParseDatabase(const YAML::Node& node) {
        DatabaseConfig db;

        if (!node.IsMap()) {
            return std::unexpected("Database configuration must be a YAML map");
        }

        // Обязательные поля
        PARSE_REQUIRED(node, "username", username, std::string);

        // Опциональные поля
        PARSE_OPTIONAL(db, node, host, host, std::string, "localhost");
        PARSE_OPTIONAL(db, node, port, port, int, 5432);
        PARSE_OPTIONAL(db, node, name, name, std::string, "face_db");
        PARSE_OPTIONAL(db, node, schema, schema, std::string, "public");

        // Пул соединений
        if (node["pool"] && node["pool"].IsMap()) {
            const auto& pool_node = node["pool"];

            PARSE_OPTIONAL(db.pool, pool_node, min_connections, min_connections, int, 2);
            PARSE_OPTIONAL(db.pool, pool_node, max_connections, max_connections, int, 10);
            PARSE_OPTIONAL(db.pool, pool_node, connection_timeout_seconds, connection_timeout_seconds, int, 5);
            PARSE_OPTIONAL(db.pool, pool_node, idle_timeout_seconds, idle_timeout_seconds, int, 60);
        }

        // Векторный поиск
        if (node["vector"] && node["vector"].IsMap()) {
            const auto& vector_node = node["vector"];

            PARSE_OPTIONAL(db.vector, vector_node, dimension, dimension, int, 512);
            PARSE_OPTIONAL(db.vector, vector_node, similarity_threshold, similarity_threshold, double, 0.75);
            PARSE_OPTIONAL(db.vector, vector_node, index_type, index_type, std::string, "ivfflat");
        }

        return db;
    }

    //==============================================================================
    // ПАРСИНГ БАСТИОНА
    //==============================================================================

    [[nodiscard]] std::expected<BastionConfig, std::string>
        ParseBastion(const YAML::Node& node) {
        BastionConfig bastion;

        if (!node.IsMap()) {
            return std::unexpected("Bastion configuration must be a YAML map");
        }

        PARSE_OPTIONAL(bastion, node, enabled, enabled, bool, true);
        PARSE_OPTIONAL(bastion, node, protocol, protocol, std::string, "tcp");
        PARSE_OPTIONAL(bastion, node, host, host, std::string, "192.168.1.200");
        PARSE_OPTIONAL(bastion, node, port, port, int, 9000);

        if (node["username"] && node["username"].IsScalar()) {
            bastion.username = node["username"].as<std::string>();
        }

        if (node["timeout"] && node["timeout"].IsMap()) {
            const auto& timeout_node = node["timeout"];
            PARSE_OPTIONAL(bastion.timeout, timeout_node, connect_seconds, connect_seconds, int, 3);
            PARSE_OPTIONAL(bastion.timeout, timeout_node, send_seconds, send_seconds, int, 2);
            PARSE_OPTIONAL(bastion.timeout, timeout_node, receive_seconds, receive_seconds, int, 3);
        }

        if (node["retry"] && node["retry"].IsMap()) {
            const auto& retry_node = node["retry"];
            PARSE_OPTIONAL(bastion.retry, retry_node, max_attempts, max_attempts, int, 3);
            PARSE_OPTIONAL(bastion.retry, retry_node, delay_ms, delay_ms, int, 100);
            PARSE_OPTIONAL(bastion.retry, retry_node, backoff_multiplier, backoff_multiplier, int, 2);
        }

        return bastion;
    }

    //==============================================================================
    // ПАРСИНГ РАСПОЗНАВАНИЯ
    //==============================================================================

    [[nodiscard]] std::expected<RecognitionConfig, std::string>
        ParseRecognition(const YAML::Node& node) {
        RecognitionConfig recognition;

        if (!node.IsMap()) {
            return std::unexpected("Recognition configuration must be a YAML map");
        }

        // Детектор (обязательный)
        if (node["detector"] && node["detector"].IsMap()) {
            const auto& det_node = node["detector"];

            PARSE_OPTIONAL(recognition.detector, det_node, type, type, std::string, "ssd");
            PARSE_OPTIONAL(recognition.detector, det_node, model_path, model_path, std::string, "");
            PARSE_OPTIONAL(recognition.detector, det_node, backend, backend, std::string, "opencv");
            PARSE_OPTIONAL(recognition.detector, det_node, confidence_threshold, confidence_threshold, float, 0.5f);
            PARSE_OPTIONAL(recognition.detector, det_node, input_width, input_width, int, 300);
            PARSE_OPTIONAL(recognition.detector, det_node, input_height, input_height, int, 300);
            PARSE_OPTIONAL(recognition.detector, det_node, use_gpu, use_gpu, bool, false);
            PARSE_OPTIONAL(recognition.detector, det_node, batch_size, batch_size, int, 1);

            if (det_node["config_path"] && det_node["config_path"].IsScalar()) {
                recognition.detector.config_path = det_node["config_path"].as<std::string>();
            }
            else if (recognition.detector.type == "ssd" &&
                recognition.detector.model_path.find(".caffemodel") != std::string::npos) {
                return std::unexpected("SSD Caffe model requires 'config_path' field with .prototxt file");
            }

            if (det_node["gpu_id"] && det_node["gpu_id"].IsScalar()) {
                recognition.detector.gpu_id = det_node["gpu_id"].as<int>();
            }

            if (recognition.detector.model_path.empty()) {
                return std::unexpected("Detector 'model_path' is required");
            }
        }
        else {
            return std::unexpected("Recognition config missing required section: 'detector'");
        }

        // Экстрактор (обязательный)
        if (node["extractor"] && node["extractor"].IsMap()) {
            const auto& ext_node = node["extractor"];

            PARSE_OPTIONAL(recognition.extractor, ext_node, type, type, std::string, "arcface");
            PARSE_OPTIONAL(recognition.extractor, ext_node, model_path, model_path, std::string, "");
            PARSE_OPTIONAL(recognition.extractor, ext_node, backend, backend, std::string, "opencv");
            PARSE_OPTIONAL(recognition.extractor, ext_node, embedding_size, embedding_size, int, 512);
            PARSE_OPTIONAL(recognition.extractor, ext_node, normalize, normalize, bool, true);
            PARSE_OPTIONAL(recognition.extractor, ext_node, use_gpu, use_gpu, bool, false);
            PARSE_OPTIONAL(recognition.extractor, ext_node, batch_size, batch_size, int, 1);

            if (ext_node["gpu_id"] && ext_node["gpu_id"].IsScalar()) {
                recognition.extractor.gpu_id = ext_node["gpu_id"].as<int>();
            }

            if (recognition.extractor.model_path.empty()) {
                return std::unexpected("Extractor 'model_path' is required");
            }
        }
        else {
            return std::unexpected("Recognition config missing required section: 'extractor'");
        }

        // Сравнение
        if (node["matching"] && node["matching"].IsMap()) {
            const auto& match_node = node["matching"];
            PARSE_OPTIONAL(recognition.matching, match_node, threshold, threshold, float, 0.6f);
            PARSE_OPTIONAL(recognition.matching, match_node, max_distance, max_distance, float, 1.5f);
            PARSE_OPTIONAL(recognition.matching, match_node, top_k, top_k, int, 1);
            PARSE_OPTIONAL(recognition.matching, match_node, use_index, use_index, bool, true);
        }

        // Anti-passback
        if (node["anti_passback"] && node["anti_passback"].IsMap()) {
            const auto& apb_node = node["anti_passback"];
            PARSE_OPTIONAL(recognition.anti_passback, apb_node, enabled, enabled, bool, true);
            PARSE_OPTIONAL(recognition.anti_passback, apb_node, cooldown_seconds, cooldown_seconds, int, 30);
            PARSE_OPTIONAL(recognition.anti_passback, apb_node, strict_mode, strict_mode, bool, false);
        }

        // Производительность
        if (node["performance"] && node["performance"].IsMap()) {
            const auto& perf_node = node["performance"];
            PARSE_OPTIONAL(recognition.performance, perf_node, skip_frames, skip_frames, int, 2);
            PARSE_OPTIONAL(recognition.performance, perf_node, max_faces_per_frame, max_faces_per_frame, int, 10);
            PARSE_OPTIONAL(recognition.performance, perf_node, parallel_detection, parallel_detection, bool, false);
            PARSE_OPTIONAL(recognition.performance, perf_node, queue_size, queue_size, int, 100);
            PARSE_OPTIONAL(recognition.performance, perf_node, min_face_size, min_face_size, int, 100);
        }

        return recognition;
    }

    //==============================================================================
    // ПАРСИНГ ЛОГИРОВАНИЯ
    //==============================================================================

    [[nodiscard]] std::expected<LoggingConfig, std::string>
        ParseLogging(const YAML::Node& node) {
        LoggingConfig logging;

        if (!node.IsMap()) {
            return std::unexpected("Logging configuration must be a YAML map");
        }

        PARSE_OPTIONAL(logging, node, level, level, std::string, "info");
        PARSE_OPTIONAL(logging, node, format, format, std::string, "json");

        // Парсинг outputs (массив)
        if (node["outputs"] && node["outputs"].IsSequence()) {
            for (size_t i = 0; i < node["outputs"].size(); ++i) {
                const auto& out_node = node["outputs"][i];

                if (!out_node.IsMap()) {
                    return std::unexpected("Logging output " + std::to_string(i + 1) + " must be a map");
                }

                LoggingConfig::Output output;

                if (!out_node["type"] || !out_node["type"].IsScalar()) {
                    return std::unexpected("Logging output " + std::to_string(i + 1) +
                        " missing required field: 'type'");
                }
                output.type = out_node["type"].as<std::string>();

                if (!out_node["enabled"] || !out_node["enabled"].IsScalar()) {
                    return std::unexpected("Logging output " + std::to_string(i + 1) +
                        " missing required field: 'enabled'");
                }
                output.enabled = out_node["enabled"].as<bool>();

                if (out_node["path"] && out_node["path"].IsScalar()) {
                    output.path = out_node["path"].as<std::string>();
                }
                else if (output.type == "file") {
                    return std::unexpected("File logging output " + std::to_string(i + 1) +
                        " requires 'path' field");
                }

                PARSE_OPTIONAL(output, out_node, rotation, rotation, std::string, "daily");
                PARSE_OPTIONAL(output, out_node, max_size_mb, max_size_mb, int, 100);
                PARSE_OPTIONAL(output, out_node, max_files, max_files, int, 7);

                logging.outputs.push_back(output);
            }
        }
        else {
            // Дефолтный console output
            LoggingConfig::Output default_output;
            default_output.type = "console";
            default_output.enabled = true;
            logging.outputs.push_back(default_output);
        }

        if (node["metrics"] && node["metrics"].IsMap()) {
            const auto& metrics_node = node["metrics"];
            PARSE_OPTIONAL(logging.metrics, metrics_node, enabled, enabled, bool, true);
            PARSE_OPTIONAL(logging.metrics, metrics_node, interval_seconds, interval_seconds, int, 60);
        }

        return logging;
    }

    //==============================================================================
    // ПАРСИНГ БЕЗОПАСНОСТИ
    //==============================================================================

    [[nodiscard]] std::expected<SecurityConfig, std::string>
        ParseSecurity(const YAML::Node& node) {
        SecurityConfig security;

        if (!node.IsMap()) {
            return std::unexpected("Security configuration must be a YAML map");
        }

        PARSE_OPTIONAL(security, node, encrypt_secrets, encrypt_secrets, bool, true);
        PARSE_OPTIONAL(security, node, secrets_file, secrets_file, std::string, "secrets.yaml.enc");
        PARSE_OPTIONAL(security, node, config_file_mode, config_file_mode, int, 0600);
        PARSE_OPTIONAL(security, node, log_file_mode, log_file_mode, int, 0640);
        PARSE_OPTIONAL(security, node, first_run, first_run, bool, false);

        if (node["allowed_networks"] && node["allowed_networks"].IsSequence()) {
            for (const auto& net_node : node["allowed_networks"]) {
                if (!net_node.IsScalar()) {
                    return std::unexpected("allowed_networks must contain only strings");
                }
                security.allowed_networks.push_back(net_node.as<std::string>());
            }
        }

        if (node["encryption"] && node["encryption"].IsMap()) {
            const auto& enc_node = node["encryption"];
            PARSE_OPTIONAL(security.encryption, enc_node, algorithm, algorithm, std::string, "AES-256-GCM");
            PARSE_OPTIONAL(security.encryption, enc_node, key_derivation, key_derivation, std::string, "PBKDF2");
            PARSE_OPTIONAL(security.encryption, enc_node, iterations, iterations, int, 100000);
            PARSE_OPTIONAL(security.encryption, enc_node, salt_length, salt_length, int, 32);
            PARSE_OPTIONAL(security.encryption, enc_node, prompt_at_startup, prompt_at_startup, bool, true);
        }

        return security;
    }

    //==============================================================================
    // ВАЛИДАЦИЯ КОНФИГУРАЦИИ
    //==============================================================================

    [[nodiscard]] std::expected<void, std::string>
        ValidateConfig(const ServerConfig& config) {
        // 1. Проверка наличия камер
        if (config.cameras.empty()) {
            return std::unexpected("At least one camera must be configured");
        }

        // 2. Уникальность ID камер
        std::unordered_set<int> camera_ids;
        for (const auto& camera : config.cameras) {
            if (camera_ids.find(camera.id) != camera_ids.end()) {
                return std::unexpected("Duplicate camera ID: " + std::to_string(camera.id));
            }
            camera_ids.insert(camera.id);
        }

        // 3. Проверка портов
        if (config.database.port <= 0 || config.database.port > 65535) {
            return std::unexpected("Database port must be between 1 and 65535");
        }

        if (config.bastion.enabled) {
            if (config.bastion.port <= 0 || config.bastion.port > 65535) {
                return std::unexpected("Bastion port must be between 1 and 65535");
            }
        }

        for (const auto& camera : config.cameras) {
            if (camera.connection.protocol != "usb") {
                if (camera.connection.port <= 0 || camera.connection.port > 65535) {
                    return std::unexpected("Camera " + std::to_string(camera.id) +
                        ": port must be between 1 and 65535");
                }
            }
        }

        // 4. Проверка путей к моделям
        if (config.recognition.detector.model_path.empty()) {
            return std::unexpected("Recognition detector model path is required");
        }

        if (config.recognition.detector.type == "ssd" &&
            config.recognition.detector.model_path.find(".caffemodel") != std::string::npos) {
            if (!config.recognition.detector.config_path.has_value()) {
                return std::unexpected("SSD Caffe model requires config_path (.prototxt file)");
            }
        }

        if (config.recognition.extractor.model_path.empty()) {
            return std::unexpected("Recognition extractor model path is required");
        }

        // 5. Проверка пула соединений БД
        if (config.database.pool.min_connections <= 0) {
            return std::unexpected("Database min_connections must be positive");
        }
        if (config.database.pool.max_connections < config.database.pool.min_connections) {
            return std::unexpected("Database max_connections must be >= min_connections");
        }
        if (config.database.pool.connection_timeout_seconds <= 0) {
            return std::unexpected("Database connection_timeout_seconds must be positive");
        }

        // 6. Проверка настроек векторного поиска
        if (config.database.vector.dimension <= 0) {
            return std::unexpected("Vector dimension must be positive");
        }
        if (config.database.vector.similarity_threshold < 0.0f ||
            config.database.vector.similarity_threshold > 1.0f) {
            return std::unexpected("Similarity threshold must be between 0.0 and 1.0");
        }

        // 7. Проверка настроек камер
        for (const auto& camera : config.cameras) {
            if (camera.capture.fps <= 0 || camera.capture.fps > 120) {
                return std::unexpected("Camera " + std::to_string(camera.id) + ": FPS must be between 1 and 120");
            }

            if (camera.capture.width <= 0 || camera.capture.height <= 0) {
                return std::unexpected("Camera " + std::to_string(camera.id) + ": Resolution must be positive");
            }

            if (camera.capture.rotation % 90 != 0 ||
                camera.capture.rotation < 0 ||
                camera.capture.rotation > 270) {
                return std::unexpected("Camera " + std::to_string(camera.id) + ": Rotation must be 0, 90, 180, or 270");
            }

            if (camera.connection.protocol == "usb" && !camera.connection.device.has_value()) {
                return std::unexpected("USB camera " + std::to_string(camera.id) + ": device path is required");
            }
        }

        // 8. Проверка бастиона
        if (config.bastion.enabled) {
            if (config.bastion.host.empty()) {
                return std::unexpected("Bastion host is required when enabled");
            }
            if (config.bastion.timeout.connect_seconds <= 0) {
                return std::unexpected("Bastion connect timeout must be positive");
            }
            if (config.bastion.retry.max_attempts <= 0) {
                return std::unexpected("Bastion max_attempts must be positive");
            }
        }

        // 9. Проверка безопасности
        if (config.security.encrypt_secrets && config.security.secrets_file.empty()) {
            return std::unexpected("Secrets file path is required when encryption is enabled");
        }
        if (config.security.encryption.iterations < 10000) {
            return std::unexpected("Encryption iterations should be at least 10000 for security");
        }

        // 10. Проверка логирования
        bool has_enabled_output = false;
        for (const auto& output : config.logging.outputs) {
            if (output.enabled) {
                has_enabled_output = true;
                if (output.type == "file" && !output.path.has_value()) {
                    return std::unexpected("File logging output requires path");
                }
            }
        }
        if (!has_enabled_output) {
            return std::unexpected("At least one logging output must be enabled");
        }
        if (config.logging.metrics.interval_seconds <= 0) {
            return std::unexpected("Metrics interval must be positive");
        }

        // 11. Проверка распознавания
        if (config.recognition.detector.confidence_threshold < 0.0f ||
            config.recognition.detector.confidence_threshold > 1.0f) {
            return std::unexpected("Detector confidence threshold must be between 0.0 and 1.0");
        }
        if (config.recognition.matching.threshold < 0.0f ||
            config.recognition.matching.threshold > 1.0f) {
            return std::unexpected("Matching threshold must be between 0.0 and 1.0");
        }
        if (config.recognition.performance.skip_frames < 0) {
            return std::unexpected("Skip frames must be non-negative");
        }
        if (config.recognition.performance.max_faces_per_frame <= 0) {
            return std::unexpected("Max faces per frame must be positive");
        }

        return {};
    }

    //==============================================================================
    // ГЛАВНАЯ ФУНКЦИЯ ЗАГРУЗКИ КОНФИГА
    //==============================================================================

    [[nodiscard]] std::expected<LoadResult, std::string>
        LoadConfig(const std::string& config_path) {
        LoadResult result;

        CHECK_FILE(config_path, "Config file not found: " + config_path);

        try {
            YAML::Node root = YAML::LoadFile(config_path);

            // Загрузка версии
            auto version_result = LoadVersionOnly(config_path);
            if (!version_result) {
                return std::unexpected(version_result.error());
            }
            result.config.version = *version_result;

            // Основные поля сервера
            PARSE_SCALAR_FIELD(hostname, hostname, std::string);
            PARSE_SCALAR_FIELD(thread_pool_size, thread_pool_size, int);
            PARSE_SCALAR_FIELD(frame_queue_size, frame_queue_size, size_t);

            // Обязательные секции
            PARSE_REQUIRED_SECTION(database, ParseDatabase, database);
            PARSE_REQUIRED_SECTION(recognition, ParseRecognition, recognition);

            // Опциональные секции
            PARSE_OPTIONAL_SECTION(bastion, ParseBastion, bastion);
            PARSE_OPTIONAL_SECTION(logging, ParseLogging, logging);
            PARSE_OPTIONAL_SECTION(security, ParseSecurity, security);

            // Камеры
            PARSE_CAMERAS();

            // Валидация
            auto validation = ValidateConfig(result.config);
            if (!validation) {
                return std::unexpected("Validation error: " + validation.error());
            }

            // Заглушка для секретов (будут загружаться отдельно)
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

} // namespace bigiate::config