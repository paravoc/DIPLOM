#include <iostream>
#include <expected>
#include <yaml-cpp/yaml.h>
#include "configs/include/loader.h"
#include "configs/include/types.h"

// Создаём алиас для удобства
namespace config = bigiate::config;


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