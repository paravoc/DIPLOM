// core/include/SecretsInitializer.h
#pragma once

#include <string>
#include <memory>
#include <expected>
#include "../../configs/include/types.h"

namespace bigiate::secrets {

    // Класс для инициализации секретов (первый запуск)
    class SecretsInitializer {
    public:
        // Создать шаблон секретов
        static std::expected<void, std::string> createTemplate(const std::string& path);

        // Показать диалог редактирования секретов
        static std::expected<std::string, std::string> editSecretsDialog(const std::string& templatePath);

        // Зашифровать секреты
        static std::expected<void, std::string> encryptSecrets(
            const std::string& inputPath,
            const std::string& outputPath,
            const std::string& masterPassword
        );

        // Получить мастер-пароль от пользователя (диалог)
        static std::expected<std::string, std::string> getMasterPasswordFromUser(bool isFirstRun = false);

        // Инициализация секретов (автоматически определяет первый запуск)
        static std::expected<config::Secrets, std::string> initialize();
    };

} // namespace bigiate::secrets