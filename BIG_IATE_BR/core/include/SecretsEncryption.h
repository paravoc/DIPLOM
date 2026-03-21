//==============================================================================
// BIG IATE - Secrets Encryption
// SecretsEncryption.h
//==============================================================================
// Описание: Класс для шифрования и дешифрования секретов с использованием
//           AES-256-GCM (OpenSSL). Обеспечивает безопасное хранение паролей.
//
// Алгоритм: AES-256-GCM с PBKDF2 для получения ключа из пароля.
// Формат данных: [salt(32)] + [iv(12)] + [tag(16)] + [ciphertext]
//
// Автор: paravoc
// Дата: 21.03.2026
// Версия: 1.0.0
//==============================================================================

#pragma once

#include <string>
#include <expected>
#include <vector>
#include <cstdint>

namespace bigiate::secrets {

    //==============================================================================
    // КЛАСС ШИФРОВАНИЯ AES-256-GCM
    //==============================================================================
    class AESEncryption {
    public:
        // Шифрование данных с мастер-паролем
        // Возвращает hex-строку с зашифрованными данными
        [[nodiscard]] static std::expected<std::string, std::string> encrypt(
            const std::string& data,
            const std::string& password
        );

        // Дешифрование данных с мастер-паролем
        // Возвращает исходную строку
        [[nodiscard]] static std::expected<std::string, std::string> decrypt(
            const std::string& encrypted,
            const std::string& password
        );

    private:
        // Генерация ключа из пароля с помощью PBKDF2
        [[nodiscard]] static std::expected<std::vector<uint8_t>, std::string> deriveKey(
            const std::string& password,
            const std::vector<uint8_t>& salt,
            int iterations = 100000
        );

        // Генерация случайной соли
        [[nodiscard]] static std::expected<std::vector<uint8_t>, std::string> generateSalt(size_t length = 32);

        // Конвертация байтов в hex-строку
        static std::string bytesToHex(const std::vector<uint8_t>& bytes);

        // Конвертация hex-строки в байты
        static std::vector<uint8_t> hexToBytes(const std::string& hex);
    };

} // namespace bigiate::secrets