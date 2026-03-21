// core/src/SecretsEncryption.cpp
#include "../include/SecretsEncryption.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cstring>

namespace bigiate::secrets {

    // ============================================================
    // ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
    // ============================================================

    static std::string getOpenSSLError() {
        char buf[256];
        ERR_error_string_n(ERR_get_error(), buf, sizeof(buf));
        return std::string(buf);
    }

    std::string AESEncryption::bytesToHex(const std::vector<uint8_t>& bytes) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (uint8_t byte : bytes) {
            ss << std::setw(2) << static_cast<int>(byte);
        }
        return ss.str();
    }

    std::vector<uint8_t> AESEncryption::hexToBytes(const std::string& hex) {
        std::vector<uint8_t> bytes;
        for (size_t i = 0; i < hex.length(); i += 2) {
            std::string byteString = hex.substr(i, 2);
            uint8_t byte = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
            bytes.push_back(byte);
        }
        return bytes;
    }

    // ============================================================
    // ГЕНЕРАЦИЯ СОЛИ И КЛЮЧА
    // ============================================================

    std::expected<std::vector<uint8_t>, std::string> AESEncryption::generateSalt(size_t length) {
        std::vector<uint8_t> salt(length);
        if (RAND_bytes(salt.data(), static_cast<int>(length)) != 1) {
            return std::unexpected("Failed to generate random salt: " + getOpenSSLError());
        }
        return salt;
    }

    std::expected<std::vector<uint8_t>, std::string> AESEncryption::deriveKey(
        const std::string& password,
        const std::vector<uint8_t>& salt,
        int iterations) {

        std::vector<uint8_t> key(32); // AES-256 = 32 байта

        if (PKCS5_PBKDF2_HMAC(
            password.c_str(),
            static_cast<int>(password.length()),
            salt.data(),
            static_cast<int>(salt.size()),
            iterations,
            EVP_sha256(),
            32,
            key.data()) != 1) {
            return std::unexpected("PBKDF2 key derivation failed: " + getOpenSSLError());
        }

        return key;
    }

    // ============================================================
    // ШИФРОВАНИЕ AES-256-GCM
    // ============================================================

    std::expected<std::string, std::string> AESEncryption::encrypt(
        const std::string& data,
        const std::string& password) {

        try {
            // 1. Генерируем случайную соль
            auto saltResult = generateSalt(32);
            if (!saltResult.has_value()) {
                return std::unexpected(saltResult.error());
            }
            std::vector<uint8_t> salt = std::move(saltResult.value());

            // 2. Генерируем ключ из пароля и соли
            auto keyResult = deriveKey(password, salt);
            if (!keyResult.has_value()) {
                return std::unexpected(keyResult.error());
            }
            std::vector<uint8_t> key = std::move(keyResult.value());

            // 3. Генерируем случайный IV (12 байт для GCM)
            std::vector<uint8_t> iv(12);
            if (RAND_bytes(iv.data(), static_cast<int>(iv.size())) != 1) {
                return std::unexpected("Failed to generate IV: " + getOpenSSLError());
            }

            // 4. Шифруем
            EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
            if (!ctx) {
                return std::unexpected("Failed to create cipher context");
            }

            if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
                EVP_CIPHER_CTX_free(ctx);
                return std::unexpected("Failed to init encryption: " + getOpenSSLError());
            }

            if (EVP_EncryptInit_ex(ctx, nullptr, nullptr, key.data(), iv.data()) != 1) {
                EVP_CIPHER_CTX_free(ctx);
                return std::unexpected("Failed to set key/IV: " + getOpenSSLError());
            }

            std::vector<uint8_t> ciphertext(data.size());
            int len = 0;

            if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len,
                reinterpret_cast<const unsigned char*>(data.data()),
                static_cast<int>(data.size())) != 1) {
                EVP_CIPHER_CTX_free(ctx);
                return std::unexpected("Encryption update failed: " + getOpenSSLError());
            }

            int ciphertext_len = len;

            if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
                EVP_CIPHER_CTX_free(ctx);
                return std::unexpected("Encryption final failed: " + getOpenSSLError());
            }
            ciphertext_len += len;
            ciphertext.resize(ciphertext_len);

            // 5. Получаем тег аутентификации (16 байт)
            std::vector<uint8_t> tag(16);
            if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, static_cast<int>(tag.size()), tag.data()) != 1) {
                EVP_CIPHER_CTX_free(ctx);
                return std::unexpected("Failed to get GCM tag: " + getOpenSSLError());
            }

            EVP_CIPHER_CTX_free(ctx);

            // 6. Формируем результат: salt + iv + tag + ciphertext
            std::vector<uint8_t> result;
            result.insert(result.end(), salt.begin(), salt.end());
            result.insert(result.end(), iv.begin(), iv.end());
            result.insert(result.end(), tag.begin(), tag.end());
            result.insert(result.end(), ciphertext.begin(), ciphertext.end());

            // Возвращаем в hex для удобного хранения
            return bytesToHex(result);

        }
        catch (const std::exception& e) {
            return std::unexpected(std::string("Encryption exception: ") + e.what());
        }
    }

    // ============================================================
    // ДЕШИФРОВАНИЕ AES-256-GCM
    // ============================================================

    std::expected<std::string, std::string> AESEncryption::decrypt(
        const std::string& encrypted,
        const std::string& password) {

        try {
            // 1. Преобразуем hex в байты
            std::vector<uint8_t> data = hexToBytes(encrypted);

            // 2. Извлекаем компоненты
            // Формат: salt(32) + iv(12) + tag(16) + ciphertext
            const size_t SALT_SIZE = 32;
            const size_t IV_SIZE = 12;
            const size_t TAG_SIZE = 16;

            if (data.size() < SALT_SIZE + IV_SIZE + TAG_SIZE) {
                return std::unexpected("Invalid encrypted data: too short");
            }

            std::vector<uint8_t> salt(data.begin(), data.begin() + SALT_SIZE);
            std::vector<uint8_t> iv(data.begin() + SALT_SIZE, data.begin() + SALT_SIZE + IV_SIZE);
            std::vector<uint8_t> tag(data.begin() + SALT_SIZE + IV_SIZE, data.begin() + SALT_SIZE + IV_SIZE + TAG_SIZE);
            std::vector<uint8_t> ciphertext(data.begin() + SALT_SIZE + IV_SIZE + TAG_SIZE, data.end());

            // 3. Генерируем ключ из пароля и соли
            auto keyResult = deriveKey(password, salt);
            if (!keyResult.has_value()) {
                return std::unexpected(keyResult.error());
            }
            std::vector<uint8_t> key = std::move(keyResult.value());

            // 4. Дешифруем
            EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
            if (!ctx) {
                return std::unexpected("Failed to create cipher context");
            }

            if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
                EVP_CIPHER_CTX_free(ctx);
                return std::unexpected("Failed to init decryption: " + getOpenSSLError());
            }

            if (EVP_DecryptInit_ex(ctx, nullptr, nullptr, key.data(), iv.data()) != 1) {
                EVP_CIPHER_CTX_free(ctx);
                return std::unexpected("Failed to set key/IV: " + getOpenSSLError());
            }

            std::vector<uint8_t> plaintext(ciphertext.size());
            int len = 0;

            if (EVP_DecryptUpdate(ctx, plaintext.data(), &len,
                ciphertext.data(),
                static_cast<int>(ciphertext.size())) != 1) {
                EVP_CIPHER_CTX_free(ctx);
                return std::unexpected("Decryption update failed: " + getOpenSSLError());
            }

            int plaintext_len = len;

            // Устанавливаем тег для проверки аутентичности
            if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, static_cast<int>(tag.size()), tag.data()) != 1) {
                EVP_CIPHER_CTX_free(ctx);
                return std::unexpected("Failed to set GCM tag: " + getOpenSSLError());
            }

            if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
                EVP_CIPHER_CTX_free(ctx);
                return std::unexpected("Decryption failed: invalid password or corrupted data");
            }
            plaintext_len += len;
            plaintext.resize(plaintext_len);

            EVP_CIPHER_CTX_free(ctx);

            return std::string(plaintext.begin(), plaintext.end());

        }
        catch (const std::exception& e) {
            return std::unexpected(std::string("Decryption exception: ") + e.what());
        }
    }

} // namespace bigiate::secrets