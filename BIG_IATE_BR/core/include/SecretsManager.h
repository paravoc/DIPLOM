//==============================================================================
// BIG IATE - Secrets Manager
// SecretsManager.h
//==============================================================================
// Описание: Класс для загрузки и парсинга зашифрованных секретов.
//           Расшифровывает файл и преобразует YAML в структуру config::Secrets.
//
// Автор: paravoc
// Дата: 21.03.2026
// Версия: 1.0.0
//==============================================================================

#pragma once

#include <string>
#include <expected>
#include "../../configs/include/types.h"

namespace bigiate::secrets {

    //==============================================================================
    // КЛАСС УПРАВЛЕНИЯ СЕКРЕТАМИ
    //==============================================================================
    class SecretsManager {
    public:
        // Загрузить секреты из зашифрованного файла
        // @param path          путь к файлу secrets.yaml.enc
        // @param masterPassword мастер-пароль для расшифровки
        [[nodiscard]] static std::expected<config::Secrets, std::string> loadFromFile(
            const std::string& path,
            const std::string& masterPassword
        );

        // Создать шаблон секретов (для первого запуска)
        [[nodiscard]] static std::expected<void, std::string> createTemplate(
            const std::string& path
        );

    private:
        // Парсинг YAML-строки в структуру Secrets
        [[nodiscard]] static std::expected<config::Secrets, std::string> parseYaml(
            const std::string& yaml
        );
    };

} // namespace bigiate::secrets