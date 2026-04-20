//==============================================================================
// BIG IATE - Secrets Initializer Implementation
// SecretsInitializer.cpp
//==============================================================================
// Описание: Реализация первоначальной настройки секретов.
//
// Автор: paravoc
// Дата: 21.03.2026
// Версия: 1.0.0
//==============================================================================

#include "../include/SecretsInitializer.h"
#include "../include/SecretsManager.h"
#include "../include/SecretsEncryption.h"
#include <wx/wx.h>
#include <fstream>
#include <sstream>
#include <iostream>

namespace bigiate::secrets {

    //==============================================================================
    // СОЗДАНИЕ ШАБЛОНА
    //==============================================================================
    std::expected<void, std::string> SecretsInitializer::createTemplate(const std::string& path) {
        std::ofstream file(path);
        if (!file.is_open()) {
            return std::unexpected("Cannot create template: " + path);
        }

        file << "# BIG IATE - Secrets Template\n";
        file << "# Fill in the passwords and click OK\n\n";
        file << "database_password: \"\"\n\n";
        file << "cameras:\n";
        file << "  1:\n";
        file << "    username: \"\"\n";
        file << "    password: \"\"\n";
        file << "  2:\n";
        file << "    username: \"\"\n";
        file << "    password: \"\"\n";
        file << "  3:\n";
        file << "    username: \"\"\n";
        file << "    password: \"\"\n";
        file << "  4:\n";
        file << "    username: \"\"\n";
        file << "    password: \"\"\n\n";
        file << "bastion_password: \"\"\n";
        file.close();

        return {};
    }

    //==============================================================================
    // ДИАЛОГ РЕДАКТИРОВАНИЯ СЕКРЕТОВ
    //==============================================================================
    std::expected<std::string, std::string> SecretsInitializer::editSecretsDialog(const std::string& templatePath) {
        // Читаем шаблон
        std::ifstream in(templatePath);
        if (!in.is_open()) {
            return std::unexpected("Cannot open template: " + templatePath);
        }

        std::stringstream buffer;
        buffer << in.rdbuf();
        std::string content = buffer.str();
        in.close();

        // Показываем диалог редактирования
        wxTextEntryDialog dlg(nullptr,
            wxString::FromUTF8("Редактируйте секреты (пароли):\n\n"
                "database_password: \"your_db_password\"\n\n"
                "cameras:\n"
                "  1:\n"
                "    username: \"admin\"\n"
                "    password: \"camera_pass\"\n\n"
                "bastion_password: \"\""),
            wxString::FromUTF8("Настройка секретов"),
            wxString::FromUTF8(content),
            wxOK | wxCANCEL);

        if (dlg.ShowModal() != wxID_OK) {
            return std::unexpected("User cancelled");
        }

        return dlg.GetValue().ToStdString();
    }

    //==============================================================================
    // ШИФРОВАНИЕ СЕКРЕТОВ
    //==============================================================================
    std::expected<void, std::string> SecretsInitializer::encryptSecrets(
        const std::string& inputPath,
        const std::string& outputPath,
        const std::string& masterPassword) {

        std::ifstream in(inputPath);
        if (!in.is_open()) {
            return std::unexpected("Cannot open input file: " + inputPath);
        }

        std::stringstream buffer;
        buffer << in.rdbuf();
        std::string plaintext = buffer.str();
        in.close();

        if (plaintext.empty()) {
            return std::unexpected("Input file is empty");
        }

        auto encrypted = AESEncryption::encrypt(plaintext, masterPassword);
        if (!encrypted.has_value()) {
            return std::unexpected(encrypted.error());
        }

        std::ofstream out(outputPath, std::ios::binary);
        out.write(encrypted.value().c_str(), static_cast<std::streamsize>(encrypted.value().size()));
        out.close();

        return {};
    }

    //==============================================================================
    // ПОЛУЧЕНИЕ МАСТЕР-ПАРОЛЯ ОТ ПОЛЬЗОВАТЕЛЯ
    //==============================================================================
    std::expected<std::string, std::string> SecretsInitializer::getMasterPasswordFromUser(bool isFirstRun) {
        wxString prompt;
        wxString title;

        if (isFirstRun) {
            prompt = wxString::FromUTF8("Введите мастер-пароль для шифрования секретов:");
            title = wxString::FromUTF8("Установка мастер-пароля");
        }
        else {
            prompt = wxString::FromUTF8("Введите мастер-пароль для расшифровки секретов:");
            title = wxString::FromUTF8("Аутентификация");
        }

        wxTextEntryDialog dlg(nullptr, prompt, title, "", wxOK | wxCANCEL | wxTE_PASSWORD);

        if (dlg.ShowModal() != wxID_OK) {
            return std::unexpected("User cancelled");
        }

        std::string password = dlg.GetValue().ToStdString();

        if (password.empty()) {
            return std::unexpected("Password cannot be empty");
        }

        // Для первого запуска — подтверждение пароля
        if (isFirstRun) {
            wxTextEntryDialog confirmDlg(nullptr,
                wxString::FromUTF8("Подтвердите мастер-пароль:"),
                wxString::FromUTF8("Подтверждение"),
                "",
                wxOK | wxCANCEL | wxTE_PASSWORD);

            if (confirmDlg.ShowModal() != wxID_OK) {
                return std::unexpected("User cancelled");
            }

            std::string confirm = confirmDlg.GetValue().ToStdString();
            if (password != confirm) {
                return std::unexpected("Passwords do not match");
            }
        }

        return password;
    }

    //==============================================================================
    // ГЛАВНАЯ ФУНКЦИЯ ИНИЦИАЛИЗАЦИИ
    //==============================================================================
    std::expected<config::Secrets, std::string> SecretsInitializer::initialize() {
        const std::string secretsEncPath = "secrets.yaml.enc";
        const std::string secretsTempPath = "secrets.yaml";

        // Проверяем, есть ли зашифрованный файл
        std::ifstream checkFile(secretsEncPath);
        bool secretsExist = checkFile.is_open();
        checkFile.close();

        std::string masterPassword;
        std::string secretsContent;

        //==========================================================================
        // ПЕРВЫЙ ЗАПУСК: создаём и шифруем секреты
        //==========================================================================
        if (!secretsExist) {
            std::cout << "🔧 Первый запуск. Настройка секретов..." << std::endl;

            // Создаём шаблон
            auto createResult = createTemplate(secretsTempPath);
            if (!createResult.has_value()) {
                return std::unexpected(createResult.error());
            }

            // Редактируем секреты
            auto editResult = editSecretsDialog(secretsTempPath);
            if (!editResult.has_value()) {
                return std::unexpected(editResult.error());
            }
            secretsContent = editResult.value();

            // Сохраняем отредактированные секреты
            std::ofstream out(secretsTempPath);
            out << secretsContent;
            out.close();

            // Получаем мастер-пароль
            auto pwdResult = getMasterPasswordFromUser(true);
            if (!pwdResult.has_value()) {
                return std::unexpected(pwdResult.error());
            }
            masterPassword = pwdResult.value();

            // Шифруем
            auto encryptResult = encryptSecrets(secretsTempPath, secretsEncPath, masterPassword);
            if (!encryptResult.has_value()) {
                return std::unexpected(encryptResult.error());
            }

            // Удаляем открытый файл
            std::remove(secretsTempPath.c_str());

            std::cout << "✅ Секреты настроены и зашифрованы!" << std::endl;
            wxMessageBox(wxString::FromUTF8("Секреты успешно настроены!\n\nЗапомните мастер-пароль."),
                wxString::FromUTF8("Успех"),
                wxOK | wxICON_INFORMATION);
        }

        //==========================================================================
        // ПОЛУЧАЕМ МАСТЕР-ПАРОЛЬ ДЛЯ РАСШИФРОВКИ
        //==========================================================================
        if (masterPassword.empty()) {
            auto pwdResult = getMasterPasswordFromUser(false);
            if (!pwdResult.has_value()) {
                return std::unexpected(pwdResult.error());
            }
            masterPassword = pwdResult.value();
        }

        //==========================================================================
        // ЗАГРУЗКА И РАСШИФРОВКА СЕКРЕТОВ
        //==========================================================================
        auto secretsResult = SecretsManager::loadFromFile(secretsEncPath, masterPassword);
        if (!secretsResult.has_value()) {
            return std::unexpected(secretsResult.error());
        }

        std::cout << "✅ Секреты загружены!" << std::endl;
        return secretsResult.value();
    }

} // namespace bigiate::secrets