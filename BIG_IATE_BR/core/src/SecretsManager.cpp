//==============================================================================
// BIG IATE - Secrets Manager Implementation
// SecretsManager.cpp
//==============================================================================
// Описание: Реализация загрузки и парсинга зашифрованных секретов.
//
// Автор: paravoc
// Дата: 21.03.2026
// Версия: 1.0.0
//==============================================================================

#include "../include/SecretsManager.h"
#include "../include/SecretsEncryption.h"
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <sstream>
#include <iostream>

namespace bigiate::secrets {

    //==============================================================================
    // ПАРСИНГ YAML В СТРУКТУРУ SECRETS
    //==============================================================================
    std::expected<config::Secrets, std::string> SecretsManager::parseYaml(const std::string& yaml) {
        try {
            YAML::Node root = YAML::Load(yaml);
            config::Secrets secrets;

            // Пароль базы данных (обязательный)
            if (root["database_password"] && root["database_password"].IsScalar()) {
                secrets.database_password = root["database_password"].as<std::string>();
            }
            else {
                return std::unexpected("Missing database_password in secrets");
            }

            // Пароли для камер
            if (root["cameras"] && root["cameras"].IsMap()) {
                for (const auto& cam : root["cameras"]) {
                    int camId = cam.first.as<int>();
                    const auto& camNode = cam.second;

                    config::Secrets::CameraSecrets camSecrets;
                    if (camNode["username"] && camNode["username"].IsScalar()) {
                        camSecrets.username = camNode["username"].as<std::string>();
                    }
                    if (camNode["password"] && camNode["password"].IsScalar()) {
                        camSecrets.password = camNode["password"].as<std::string>();
                    }
                    secrets.cameras[camId] = camSecrets;
                }
            }

            // Пароль бастиона (опциональный)
            if (root["bastion_password"] && root["bastion_password"].IsScalar()) {
                secrets.bastion_password = root["bastion_password"].as<std::string>();
            }

            return secrets;

        }
        catch (const YAML::Exception& e) {
            return std::unexpected(std::string("YAML parse error: ") + e.what());
        }
    }

    //==============================================================================
    // ЗАГРУЗКА СЕКРЕТОВ ИЗ ЗАШИФРОВАННОГО ФАЙЛА
    //==============================================================================
    std::expected<config::Secrets, std::string> SecretsManager::loadFromFile(
        const std::string& path,
        const std::string& masterPassword) {

        // Читаем зашифрованный файл
        std::ifstream in(path, std::ios::binary);
        if (!in.is_open()) {
            return std::unexpected("Cannot open secrets file: " + path);
        }

        std::stringstream buffer;
        buffer << in.rdbuf();
        std::string encrypted = buffer.str();
        in.close();

        // Расшифровываем
        auto decrypted = AESEncryption::decrypt(encrypted, masterPassword);
        if (!decrypted.has_value()) {
            return std::unexpected(decrypted.error());
        }

        // Парсим YAML
        return parseYaml(decrypted.value());
    }

    //==============================================================================
    // СОЗДАНИЕ ШАБЛОНА СЕКРЕТОВ
    //==============================================================================
    std::expected<void, std::string> SecretsManager::createTemplate(const std::string& path) {
        std::ofstream file(path);
        if (!file.is_open()) {
            return std::unexpected("Cannot create template: " + path);
        }

        file << "# BIG IATE - Secrets Template\n";
        file << "# Fill in the passwords and run with --encrypt flag\n\n";
        file << "database_password: \"\"\n\n";
        file << "cameras:\n";
        file << "  1:\n";
        file << "    username: \"\"\n";
        file << "    password: \"\"\n";
        file << "  2:\n";
        file << "    username: \"\"\n";
        file << "    password: \"\"\n\n";
        file << "bastion_password: \"\"\n";
        file.close();

        return {};
    }

} // namespace bigiate::secrets