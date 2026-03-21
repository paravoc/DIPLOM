// core/src/SecretsManager.cpp
#include "../include/SecretsManager.h"
#include "../include/SecretsEncryption.h"
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <sstream>
#include <iostream>

namespace bigiate::secrets {

    std::expected<config::Secrets, std::string> SecretsManager::parseYaml(const std::string& yaml) {
        try {
            YAML::Node root = YAML::Load(yaml);
            config::Secrets secrets;

            if (root["database_password"] && root["database_password"].IsScalar()) {
                secrets.database_password = root["database_password"].as<std::string>();
            }
            else {
                return std::unexpected("Missing database_password in secrets");
            }

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

            if (root["bastion_password"] && root["bastion_password"].IsScalar()) {
                secrets.bastion_password = root["bastion_password"].as<std::string>();
            }

            return secrets;

        }
        catch (const YAML::Exception& e) {
            return std::unexpected(std::string("YAML parse error: ") + e.what());
        }
    }

    std::expected<config::Secrets, std::string> SecretsManager::loadFromFile(
        const std::string& path,
        const std::string& masterPassword) {

        // Читаем файл
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