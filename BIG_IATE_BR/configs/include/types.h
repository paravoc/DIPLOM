//==============================================================================
// BIG IATE - Configuration Types
// types.h
//==============================================================================
// Описание: Основные структуры данных для конфигурации системы.
//           Содержит все настройки: камеры, БД, распознавание, логи, безопасность.
//
// Автор: paravoc
// Дата: 21.03.2026
// Версия: 1.0.0
//==============================================================================

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <optional>

namespace bigiate::config {

    //==============================================================================
    // ВЕРСИЯ КОНФИГУРАЦИИ
    //==============================================================================
    struct Version {
        int major = 1;      // Мажорная версия (1.x.x)
        int minor = 0;      // Минорная версия (x.0.x)
        int patch = 0;      // Патч-версия (x.x.0)

        [[nodiscard]] std::string toString() const {
            return std::to_string(major) + "." +
                std::to_string(minor) + "." +
                std::to_string(patch);
        }
    };

    //==============================================================================
    // НАСТРОЙКИ КАМЕРЫ
    //==============================================================================
    struct CameraConfig {
        int id = 0;                     // Уникальный идентификатор камеры
        std::string name;               // Название камеры (для отображения в GUI)
        bool enabled = true;            // Включена ли камера

        // Настройки подключения
        struct Connection {
            std::string protocol = "rtsp";          // rtsp, usb, http
            std::string host;                       // IP-адрес для сетевых камер
            int port = 554;                         // Порт (554 для RTSP)
            std::string path = "/";                 // Путь к потоку
            std::optional<std::string> device;      // ID устройства для USB камер (0, 1, 2...)
        } connection;

        // Настройки захвата видео
        struct Capture {
            int fps = 15;               // Кадров в секунду
            int width = 1280;           // Ширина кадра
            int height = 720;           // Высота кадра
            int rotation = 0;           // Поворот изображения (0, 90, 180, 270)
        } capture;
    };

    //==============================================================================
    // НАСТРОЙКИ БАЗЫ ДАННЫХ (PostgreSQL + pgvector)
    //==============================================================================
    struct DatabaseConfig {
        std::string host = "localhost";         // Хост БД
        int port = 5432;                        // Порт PostgreSQL
        std::string name = "face_db";           // Имя базы данных
        std::string schema = "public";          // Схема
        std::string username;                   // Имя пользователя

        // Настройки пула соединений
        struct Pool {
            int min_connections = 2;            // Минимум соединений в пуле
            int max_connections = 10;           // Максимум соединений
            int connection_timeout_seconds = 5; // Таймаут подключения (сек)
            int idle_timeout_seconds = 60;      // Таймаут простоя (сек)
        } pool;

        // Настройки векторного поиска (pgvector)
        struct Vector {
            int dimension = 512;                // Размерность эмбеддинга
            double similarity_threshold = 0.75; // Порог схожести
            std::string index_type = "ivfflat"; // Тип индекса (ivfflat, hnsw)
        } vector;
    };

    //==============================================================================
    // НАСТРОЙКИ БАСТИОНА (шлюз для удалённого доступа)
    //==============================================================================
    struct BastionConfig {
        bool enabled = true;                            // Включён ли бастион
        std::string protocol = "tcp";                   // Протокол (tcp, udp)
        std::string host = "192.168.1.200";             // Хост бастиона
        int port = 9000;                                // Порт
        std::optional<std::string> username;            // Имя пользователя (если требуется)

        // Таймауты
        struct Timeouts {
            int connect_seconds = 3;                    // Таймаут подключения
            int send_seconds = 2;                       // Таймаут отправки
            int receive_seconds = 3;                    // Таймаут получения
        } timeout;

        // Настройки повторных попыток
        struct Retry {
            int max_attempts = 3;                       // Максимум попыток
            int delay_ms = 100;                         // Начальная задержка (мс)
            int backoff_multiplier = 2;                 // Множитель задержки
        } retry;
    };

    //==============================================================================
    // НАСТРОЙКИ РАСПОЗНАВАНИЯ ЛИЦ
    //==============================================================================
    struct RecognitionConfig {
        // Детектор лиц (SSD / YOLO / OpenCV)
        struct Detector {
            std::string type = "ssd";                   // Тип детектора
            std::string model_path;                     // Путь к модели
            std::optional<std::string> config_path;     // Конфиг для Caffe моделей
            std::string backend = "opencv";             // Бэкенд (opencv, onnxruntime)
            float confidence_threshold = 0.5f;          // Порог уверенности (0-1)
            int input_width = 300;                      // Ширина входного изображения
            int input_height = 300;                     // Высота входного изображения
            bool use_gpu = false;                       // Использовать GPU
            int batch_size = 1;                         // Размер батча
            std::optional<int> gpu_id;                  // ID GPU (если несколько)
        } detector;

        // Экстрактор эмбеддингов (ArcFace)
        struct Extractor {
            std::string type = "arcface";               // Тип экстрактора
            std::string model_path;                     // Путь к модели
            std::string backend = "opencv";             // Бэкенд
            int embedding_size = 512;                   // Размер эмбеддинга
            bool normalize = true;                      // Нормализовать эмбеддинг
            bool use_gpu = false;                       // Использовать GPU
            int batch_size = 1;                         // Размер батча
            std::optional<int> gpu_id;                  // ID GPU
        } extractor;

        // Настройки сравнения и поиска в БД
        struct Matching {
            float threshold = 0.6f;                     // Порог схожести
            float max_distance = 1.5f;                  // Максимальное расстояние
            int top_k = 1;                              // Количество возвращаемых результатов
            bool use_index = true;                      // Использовать индекс pgvector
        } matching;

        // Защита от повторного прохода
        struct AntiPassback {
            bool enabled = true;                        // Включена защита
            int cooldown_seconds = 30;                  // Время блокировки после прохода
            bool strict_mode = false;                   // Строгий режим
        } anti_passback;

        // Настройки производительности
        struct Performance {
            int skip_frames = 2;                        // Пропускать каждый N-й кадр
            int max_faces_per_frame = 10;               // Максимум лиц на кадр
            bool parallel_detection = false;            // Параллельная детекция
            int queue_size = 100;                       // Размер очереди кадров
            int min_face_size = 100;                    // Минимальный размер лица (px)
        } performance;
    };

    //==============================================================================
    // НАСТРОЙКИ ЛОГИРОВАНИЯ
    //==============================================================================
    struct LoggingConfig {
        std::string level = "info";                     // debug, info, warn, error
        std::string format = "json";                    // json, text

        // Выходные потоки логирования
        struct Output {
            std::string type;                           // console, file
            bool enabled;                               // Включён ли вывод
            std::optional<std::string> path;            // Путь для файлового вывода
            std::string rotation = "daily";             // daily, hourly, size
            int max_size_mb = 100;                      // Максимальный размер файла (МБ)
            int max_files = 7;                          // Максимум файлов
        };
        std::vector<Output> outputs;

        // Настройки метрик
        struct Metrics {
            bool enabled = true;                        // Включён сбор метрик
            int interval_seconds = 60;                  // Интервал сбора (сек)
        } metrics;
    };

    //==============================================================================
    // НАСТРОЙКИ БЕЗОПАСНОСТИ
    //==============================================================================
    struct SecurityConfig {
        bool encrypt_secrets = true;                    // Шифровать секреты
        std::string secrets_file = "secrets.yaml.enc";  // Файл с зашифрованными секретами
        int config_file_mode = 0600;                    // Права на конфиг (Linux)
        int log_file_mode = 0640;                       // Права на логи (Linux)
        std::vector<std::string> allowed_networks;      // Разрешённые сети

        // Настройки шифрования
        struct Encryption {
            std::string algorithm = "AES-256-GCM";      // Алгоритм шифрования
            std::string key_derivation = "PBKDF2";      // Метод получения ключа
            int iterations = 100000;                    // Количество итераций
            int salt_length = 32;                       // Длина соли (байт)
            bool prompt_at_startup = true;              // Запрашивать пароль при запуске
        } encryption;

        bool first_run = false;                         // Первый запуск
    };

    //==============================================================================
    // ГЛАВНАЯ СТРУКТУРА КОНФИГУРАЦИИ
    //==============================================================================
    struct ServerConfig {
        // Основные настройки сервера
        std::string hostname = "bigiate-server";        // Имя сервера
        int thread_pool_size = 4;                       // Размер пула потоков
        size_t frame_queue_size = 100;                  // Размер очереди кадров

        // Версия конфига
        Version version;

        // Секции конфигурации
        DatabaseConfig database;                        // Настройки БД
        BastionConfig bastion;                          // Настройки бастиона
        RecognitionConfig recognition;                  // Настройки распознавания
        LoggingConfig logging;                          // Настройки логирования
        SecurityConfig security;                        // Настройки безопасности

        // Камеры (динамический список)
        std::vector<CameraConfig> cameras;
    };

    //==============================================================================
    // СЕКРЕТЫ (пароли, ключи)
    //==============================================================================
    struct Secrets {
        // Секреты для камер
        struct CameraSecrets {
            std::optional<std::string> username;        // Логин для камеры
            std::optional<std::string> password;        // Пароль для камеры
        };
        std::unordered_map<int, CameraSecrets> cameras; // ID камеры -> секреты

        std::string database_password;                  // Пароль БД (обязательный)
        std::optional<std::string> bastion_password;    // Пароль бастиона
    };

    //==============================================================================
    // РЕЗУЛЬТАТ РАСПОЗНАВАНИЯ
    //==============================================================================
    struct MatchResult {
        std::optional<int> user_id;                     // ID пользователя (null если не найден)
        float distance;                                 // Расстояние до найденного
        std::chrono::system_clock::time_point timestamp; // Время распознавания
        int camera_id;                                  // ID камеры
        float detection_confidence;                     // Уверенность детекции
    };

    //==============================================================================
    // РЕЗУЛЬТАТ ЗАГРУЗКИ КОНФИГУРАЦИИ
    //==============================================================================
    struct LoadResult {
        ServerConfig config;                            // Загруженная конфигурация
        Secrets secrets;                                // Загруженные секреты
        bool success = false;                           // Успешна ли загрузка
        std::optional<std::string> error_message;       // Сообщение об ошибке
    };

} // namespace bigiate::config