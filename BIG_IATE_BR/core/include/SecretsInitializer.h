//==============================================================================
// BIG IATE - Secrets Initializer
// SecretsInitializer.h
//==============================================================================
// Описание: Класс для первоначальной настройки секретов.
//           Автоматически определяет первый запуск, создаёт шаблон,
//           запрашивает мастер-пароль и шифрует секреты.
//
// Автор: paravoc
// Дата: 21.03.2026
// Версия: 1.0.0
//==============================================================================

#pragma once

#include <string>
#include <memory>
#include <expected>
#include "../../configs/include/types.h"

namespace bigiate::secrets {

    //==============================================================================
    // КЛАСС ИНИЦИАЛИЗАЦИИ СЕКРЕТОВ
    //==============================================================================
    class SecretsInitializer {
    public:
        // Создать шаблон секретов (YAML)
        static std::expected<void, std::string> createTemplate(const std::string& path);

        // Показать диалог редактирования секретов (wxWidgets)
        static std::expected<std::string, std::string> editSecretsDialog(const std::string& templatePath);

        // Зашифровать секреты и сохранить в файл
        static std::expected<void, std::string> encryptSecrets(
            const std::string& inputPath,
            const std::string& outputPath,
            const std::string& masterPassword
        );

        // Получить мастер-пароль от пользователя (через GUI диалог)
        static std::expected<std::string, std::string> getMasterPasswordFromUser(bool isFirstRun = false);

        // ГЛАВНАЯ ФУНКЦИЯ: автоматически определяет первый запуск,
        // создаёт шаблон, запрашивает пароль, шифрует и загружает секреты
        static std::expected<config::Secrets, std::string> initialize();
    };

} // namespace bigiate::secrets