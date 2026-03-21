// src/config/include/config/types.h
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <optional>

namespace bigiate::config {

    // Версия конфига
    struct Version {
        int major = 1;
        int minor = 0;
        int patch = 0;

        [[nodiscard]] std::string toString() const {
            return std::to_string(major) + "." +
                std::to_string(minor) + "." +
                std::to_string(patch);
        }
    };

    // Настройки камеры (без секретов)
    struct CameraConfig {
        int id = 0;
        std::string name;
        bool enabled = true;

        struct Connection {
            std::string protocol = "rtsp";
            std::string host;
            int port = 554;
            std::string path = "/";
            std::optional<std::string> device;  // только для USB камер
        } connection;

        struct Capture {
            int fps = 15;
            int width = 1280;
            int height = 720;
            int rotation = 0;
        } capture;
    };

    // Настройки БД
    struct DatabaseConfig {
        std::string host = "localhost";
        int port = 5432;
        std::string name = "face_db";
        std::string schema = "public";
        std::string username;

        struct Pool {
            int min_connections = 2;
            int max_connections = 10;
            int connection_timeout_seconds = 5;
            int idle_timeout_seconds = 60;
        } pool;

        struct Vector {
            int dimension = 512;
            double similarity_threshold = 0.75;
            std::string index_type = "ivfflat";
        } vector;
    };

    // Настройки бастиона
    struct BastionConfig {
        bool enabled = true;
        std::string protocol = "tcp";
        std::string host = "192.168.1.200";
        int port = 9000;
        std::optional<std::string> username;  // если требуется авторизация

        struct Timeouts {
            int connect_seconds = 3;
            int send_seconds = 2;
            int receive_seconds = 3;
        } timeout;

        struct Retry {
            int max_attempts = 3;
            int delay_ms = 100;
            int backoff_multiplier = 2;
        } retry;
    };

    // Настройки распознавания
    // Настройки распознавания
    struct RecognitionConfig {
        // SSD детектор лиц
        struct Detector {
            std::string type = "ssd";  // ssd, yolov5, etc.
            std::string model_path;
            std::optional<std::string> config_path;  // для Caffe моделей (.prototxt)
            std::string backend = "opencv";  // opencv, onnxruntime, tensorrt
            float confidence_threshold = 0.5f;
            int input_width = 300;
            int input_height = 300;
            bool use_gpu = false;
            int batch_size = 1;
            std::optional<int> gpu_id;
        } detector;

        // ArcFace экстрактор эмбеддингов
        struct Extractor {
            std::string type = "arcface";
            std::string model_path;
            std::string backend = "opencv";
            int embedding_size = 512;
            bool normalize = true;
            bool use_gpu = false;
            int batch_size = 1;
            std::optional<int> gpu_id;
        } extractor;

        // Настройки сравнения и поиска в БД
        struct Matching {
            float threshold = 0.6f;
            float max_distance = 1.5f;
            int top_k = 1;
            bool use_index = true;
        } matching;

        // Anti-passback (защита от повторного прохода)
        struct AntiPassback {
            bool enabled = true;
            int cooldown_seconds = 30;
            bool strict_mode = false;
        } anti_passback;

        // Настройки производительности
        struct Performance {
            int skip_frames = 2;        // каждый N-й кадр
            int max_faces_per_frame = 10;
            bool parallel_detection = false;
            int queue_size = 100;
            int min_face_size = 100;

        } performance;
    };

    // Настройки логирования
    struct LoggingConfig {
        std::string level = "info";
        std::string format = "json";

        struct Output {
            std::string type;  // "console" или "file"
            bool enabled;
            std::optional<std::string> path;  // для file обязательно, для console опционально
            std::string rotation = "daily";
            int max_size_mb = 100;
            int max_files = 7;
        };
        std::vector<Output> outputs;

        struct Metrics {
            bool enabled = true;
            int interval_seconds = 60;
        } metrics;
    };

    // Настройки безопасности
    struct SecurityConfig {
        bool encrypt_secrets = true;
        std::string secrets_file = "secrets.yaml.enc";
        int config_file_mode = 0600;      // для Linux
        int log_file_mode = 0640;         // для Linux
        std::vector<std::string> allowed_networks;

        struct Encryption {
            std::string algorithm = "AES-256-GCM";
            std::string key_derivation = "PBKDF2";
            int iterations = 100000;
            int salt_length = 32;
            bool prompt_at_startup = true;
        } encryption;

        bool first_run = false;
    };

    struct ServerConfig {
        // Основные настройки сервера
        std::string hostname = "bigiate-server";
        int thread_pool_size = 4;
        size_t frame_queue_size = 100;

        // Версия конфига
        Version version;

        // Основные секции
        DatabaseConfig database;
        BastionConfig bastion;
        RecognitionConfig recognition;
        LoggingConfig logging;
        SecurityConfig security;

        // Камеры (обычно после основных секций)
        std::vector<CameraConfig> cameras;
    };

    // Секреты (пароли)
    struct Secrets {
        struct CameraSecrets {
            std::optional<std::string> username;  // для камер без авторизации
            std::optional<std::string> password;
        };
        std::unordered_map<int, CameraSecrets> cameras;
        std::string database_password;  // обязательное
        std::optional<std::string> bastion_password;  // если бастион требует пароль
    };

    // Результат распознавания (для других модулей)
    struct MatchResult {
        std::optional<int> user_id;  // null если не найден
        float distance;
        std::chrono::system_clock::time_point timestamp;
        int camera_id;
        float detection_confidence;
    };

    // Результат загрузки
    struct LoadResult {
        ServerConfig config;
        Secrets secrets;
        bool success = false;
        std::optional<std::string> error_message;  // есть только если success=false
    };

} // namespace bigiate::config