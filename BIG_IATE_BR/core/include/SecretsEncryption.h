// core/include/SecretsEncryption.h
#pragma once

#include <string>
#include <expected>
#include <vector>
#include <cstdint>

namespace bigiate::secrets {

    // Класс для шифрования/дешифрования с использованием OpenSSL AES-256-GCM
    class AESEncryption {
    public:
        // Шифрование данных с мастер-паролем
        [[nodiscard]] static std::expected<std::string, std::string> encrypt(
            const std::string& data,
            const std::string& password
        );

        // Дешифрование данных с мастер-паролем
        [[nodiscard]] static std::expected<std::string, std::string> decrypt(
            const std::string& encrypted,
            const std::string& password
        );

    private:
        // Генерация ключа из пароля (PBKDF2)
        [[nodiscard]] static std::expected<std::vector<uint8_t>, std::string> deriveKey(
            const std::string& password,
            const std::vector<uint8_t>& salt,
            int iterations = 100000
        );

        // Генерация случайной соли
        [[nodiscard]] static std::expected<std::vector<uint8_t>, std::string> generateSalt(size_t length = 32);

        // Вспомогательные функции для работы с hex
        static std::string bytesToHex(const std::vector<uint8_t>& bytes);
        static std::vector<uint8_t> hexToBytes(const std::string& hex);
    };

} // namespace bigiate::secrets