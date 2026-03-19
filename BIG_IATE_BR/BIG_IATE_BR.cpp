#include <iostream>
#include <expected>
#include <yaml-cpp/yaml.h>
#include "configs/include/loader.h"
#include "configs/include/types.h"

// Создаём алиас для удобства
namespace config = bigiate::config;

void PrintLoggingConfig(const config::LoggingConfig& logging) {
    std::cout << "\n=== Logging Configuration ===\n";
    std::cout << "Level: " << logging.level << "\n";
    std::cout << "Format: " << logging.format << "\n";

    std::cout << "\n--- Outputs ---\n";
    for (size_t i = 0; i < logging.outputs.size(); ++i) {
        const auto& out = logging.outputs[i];
        std::cout << "Output " << i + 1 << ":\n";
        std::cout << "  Type: " << out.type << "\n";
        std::cout << "  Enabled: " << (out.enabled ? "true" : "false") << "\n";
        if (out.path.has_value()) {
            std::cout << "  Path: " << out.path.value() << "\n";
        }
        std::cout << "  Rotation: " << out.rotation << "\n";
        std::cout << "  Max size: " << out.max_size_mb << " MB\n";
        std::cout << "  Max files: " << out.max_files << "\n";
    }

    std::cout << "\n--- Metrics ---\n";
    std::cout << "Enabled: " << (logging.metrics.enabled ? "true" : "false") << "\n";
    std::cout << "Interval: " << logging.metrics.interval_seconds << " seconds\n";
}

void PrintDatabaseConfig(const config::DatabaseConfig& db) {
    std::cout << "\n=== Database Configuration ===\n";
    std::cout << "Host: " << db.host << "\n";
    std::cout << "Port: " << db.port << "\n";
    std::cout << "Database name: " << db.name << "\n";
    std::cout << "Schema: " << db.schema << "\n";
    std::cout << "Username: " << db.username << "\n";

    std::cout << "\n--- Pool Settings ---\n";
    std::cout << "Min connections: " << db.pool.min_connections << "\n";
    std::cout << "Max connections: " << db.pool.max_connections << "\n";
    std::cout << "Connection timeout: " << db.pool.connection_timeout_seconds << "s\n";
    std::cout << "Idle timeout: " << db.pool.idle_timeout_seconds << "s\n";

    std::cout << "\n--- Vector Settings ---\n";
    std::cout << "Dimension: " << db.vector.dimension << "\n";
    std::cout << "Similarity threshold: " << db.vector.similarity_threshold << "\n";
    std::cout << "Index type: " << db.vector.index_type << "\n";
}

void PrintCameraConfig(const config::CameraConfig& camera) {
    std::cout << "\n=== Camera Configuration ===\n";
    std::cout << "ID: " << camera.id << "\n";
    std::cout << "Name: " << camera.name << "\n";
    std::cout << "Enabled: " << (camera.enabled ? "true" : "false") << "\n";

    std::cout << "\n--- Connection ---\n";
    std::cout << "Protocol: " << camera.connection.protocol << "\n";
    if (!camera.connection.host.empty()) {
        std::cout << "Host: " << camera.connection.host << "\n";
        std::cout << "Port: " << camera.connection.port << "\n";
        std::cout << "Path: " << camera.connection.path << "\n";
    }
    if (camera.connection.device.has_value()) {
        std::cout << "Device: " << camera.connection.device.value() << "\n";
    }

    std::cout << "\n--- Capture ---\n";
    std::cout << "FPS: " << camera.capture.fps << "\n";
    std::cout << "Resolution: " << camera.capture.width << "x" << camera.capture.height << "\n";
    std::cout << "Rotation: " << camera.capture.rotation << "°\n";
}

void PrintBastionConfig(const config::BastionConfig& bastion) {
    std::cout << "\n=== Bastion Configuration ===\n";
    std::cout << "Enabled: " << (bastion.enabled ? "true" : "false") << "\n";
    std::cout << "Protocol: " << bastion.protocol << "\n";
    std::cout << "Host: " << bastion.host << "\n";
    std::cout << "Port: " << bastion.port << "\n";

    if (bastion.username.has_value()) {
        std::cout << "Username: " << bastion.username.value() << "\n";
    }
    else {
        std::cout << "Username: <not set>\n";
    }

    std::cout << "\n--- Timeouts ---\n";
    std::cout << "Connect timeout: " << bastion.timeout.connect_seconds << "s\n";
    std::cout << "Send timeout: " << bastion.timeout.send_seconds << "s\n";
    std::cout << "Receive timeout: " << bastion.timeout.receive_seconds << "s\n";

    std::cout << "\n--- Retry Settings ---\n";
    std::cout << "Max attempts: " << bastion.retry.max_attempts << "\n";
    std::cout << "Delay: " << bastion.retry.delay_ms << "ms\n";
    std::cout << "Backoff multiplier: " << bastion.retry.backoff_multiplier << "\n";
}

// НОВАЯ ФУНКЦИЯ ДЛЯ ВЫВОДА RECOGNITION CONFIG
void PrintRecognitionConfig(const config::RecognitionConfig& recognition) {
    std::cout << "\n=== Recognition Configuration ===\n";

    // Detector
    std::cout << "\n--- Detector (SSD) ---\n";
    std::cout << "Type: " << recognition.detector.type << "\n";
    std::cout << "Model path: " << recognition.detector.model_path << "\n";
    if (recognition.detector.config_path.has_value()) {
        std::cout << "Config path: " << recognition.detector.config_path.value() << "\n";
    }
    std::cout << "Backend: " << recognition.detector.backend << "\n";
    std::cout << "Confidence threshold: " << recognition.detector.confidence_threshold << "\n";
    std::cout << "Input size: " << recognition.detector.input_width << "x" << recognition.detector.input_height << "\n";
    std::cout << "Use GPU: " << (recognition.detector.use_gpu ? "true" : "false") << "\n";
    if (recognition.detector.gpu_id.has_value()) {
        std::cout << "GPU ID: " << recognition.detector.gpu_id.value() << "\n";
    }
    std::cout << "Batch size: " << recognition.detector.batch_size << "\n";

    // Extractor
    std::cout << "\n--- Extractor (ArcFace) ---\n";
    std::cout << "Type: " << recognition.extractor.type << "\n";
    std::cout << "Model path: " << recognition.extractor.model_path << "\n";
    std::cout << "Backend: " << recognition.extractor.backend << "\n";
    std::cout << "Embedding size: " << recognition.extractor.embedding_size << "\n";
    std::cout << "Normalize: " << (recognition.extractor.normalize ? "true" : "false") << "\n";
    std::cout << "Use GPU: " << (recognition.extractor.use_gpu ? "true" : "false") << "\n";
    if (recognition.extractor.gpu_id.has_value()) {
        std::cout << "GPU ID: " << recognition.extractor.gpu_id.value() << "\n";
    }
    std::cout << "Batch size: " << recognition.extractor.batch_size << "\n";

    // Matching
    std::cout << "\n--- Matching ---\n";
    std::cout << "Threshold: " << recognition.matching.threshold << "\n";
    std::cout << "Max distance: " << recognition.matching.max_distance << "\n";
    std::cout << "Top K: " << recognition.matching.top_k << "\n";
    std::cout << "Use index: " << (recognition.matching.use_index ? "true" : "false") << "\n";

    // Anti-passback
    std::cout << "\n--- Anti-passback ---\n";
    std::cout << "Enabled: " << (recognition.anti_passback.enabled ? "true" : "false") << "\n";
    std::cout << "Cooldown seconds: " << recognition.anti_passback.cooldown_seconds << "\n";
    std::cout << "Strict mode: " << (recognition.anti_passback.strict_mode ? "true" : "false") << "\n";

    // Performance
    std::cout << "\n--- Performance ---\n";
    std::cout << "Skip frames: " << recognition.performance.skip_frames << "\n";
    std::cout << "Max faces per frame: " << recognition.performance.max_faces_per_frame << "\n";
    std::cout << "Parallel detection: " << (recognition.performance.parallel_detection ? "true" : "false") << "\n";
    std::cout << "Queue size: " << recognition.performance.queue_size << "\n";
}

int main(int argc, char* argv[]) {

    try {
        // Определяем путь к конфиг файлу
        std::string config_file = "C:\\Users\\smidr\\source\\repos\\BIG_IATE_BR\\x64\\Debug\\test_config.yaml";
        if (argc > 1) {
            config_file = argv[1];  // можно передать как аргумент
        }

        std::cout << "Loading configuration from: " << config_file << "\n";

        // Загружаем YAML файл
        YAML::Node root = YAML::LoadFile(config_file);

        // Парсим конфигурацию базы данных
        if (root["database"]) {
            std::cout << "\n📦 Parsing database configuration...\n";
            auto db_result = config::ParseDatabase(root["database"]);

            if (db_result.has_value()) {
                std::cout << "✅ Database config parsed successfully!\n";
                PrintDatabaseConfig(db_result.value());
            }
            else {
                std::cout << "❌ Failed to parse database config: " << db_result.error() << "\n";
            }
        }
        else {
            std::cout << "ℹ️ No database configuration found\n";
        }

        // Парсинг бастиона
        if (root["bastion"]) {
            std::cout << "\n🛡️ Parsing bastion configuration...\n";
            auto bastion_result = config::ParseBastion(root["bastion"]);

            if (bastion_result.has_value()) {
                std::cout << "✅ Bastion config parsed successfully!\n";
                PrintBastionConfig(bastion_result.value());
            }
            else {
                std::cout << "❌ Failed to parse bastion config: " << bastion_result.error() << "\n";
            }
        }
        else {
            std::cout << "ℹ️ No bastion configuration found\n";
        }

        // НОВОЕ: Парсинг распознавания
        if (root["recognition"]) {
            std::cout << "\n🔍 Parsing recognition configuration...\n";
            auto recognition_result = config::ParseRecognition(root["recognition"]);

            if (recognition_result.has_value()) {
                std::cout << "✅ Recognition config parsed successfully!\n";
                PrintRecognitionConfig(recognition_result.value());
            }
            else {
                std::cout << "❌ Failed to parse recognition config: " << recognition_result.error() << "\n";
            }
        }
        else {
            std::cout << "ℹ️ No recognition configuration found\n";
        }

        if (root["logging"]) {
            std::cout << "\n📝 Parsing logging configuration...\n";
            auto logging_result = config::ParseLogging(root["logging"]);

            if (logging_result.has_value()) {
                std::cout << "✅ Logging config parsed successfully!\n";
                PrintLoggingConfig(logging_result.value());
            }
            else {
                std::cout << "❌ Failed to parse logging config: " << logging_result.error() << "\n";
            }
        }
        else {
            std::cout << "ℹ️ No logging configuration found\n";
        }

        // Парсим конфигурацию камер
        if (root["cameras"] && root["cameras"].IsSequence()) {
            std::cout << "\n📷 Parsing cameras configuration...\n";

            for (size_t i = 0; i < root["cameras"].size(); ++i) {
                std::cout << "\n--- Camera " << i + 1 << " ---\n";
                auto camera_result = config::ParseCamera(root["cameras"][i]);

                if (camera_result.has_value()) {
                    std::cout << "✅ Camera " << i + 1 << " parsed successfully!\n";
                    PrintCameraConfig(camera_result.value());
                }
                else {
                    std::cout << "❌ Failed to parse camera " << i + 1 << ": " << camera_result.error() << "\n";
                }
            }
        }
        else {
            std::cout << "ℹ️ No cameras configuration found\n";
        }

    }
    catch (const YAML::Exception& e) {
        std::cerr << "\n❌ YAML parsing error: " << e.what() << "\n";
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "\n❌ Error: " << e.what() << "\n";
        return 1;
    }

    std::cout << "\n✨ Done!\n";
    return 0;
}