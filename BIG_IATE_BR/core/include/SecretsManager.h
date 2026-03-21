// core/include/SecretsManager.h
#pragma once

#include <string>
#include <expected>
#include "../../configs/include/types.h"

namespace bigiate::secrets {

    class SecretsManager {
    public:
        // Загрузить секреты из зашифрованного файла
        [[nodiscard]] static std::expected<config::Secrets, std::string> loadFromFile(
            const std::string& path,
            const std::string& masterPassword
        );

        // Создать шаблон секретов
        [[nodiscard]] static std::expected<void, std::string> createTemplate(
            const std::string& path
        );

    private:
        [[nodiscard]] static std::expected<config::Secrets, std::string> parseYaml(
            const std::string& yaml
        );
    };

} // namespace bigiate::secrets