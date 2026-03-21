#ifndef USE_CONSOLE
#include <wx/wx.h>
#include "core/include/AppCore.h"
#include "core/include/SecretsManager.h"
#include "core/include/SecretsEncryption.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

// ============================================================
// СОЗДАНИЕ ШАБЛОНА SECRETS.YAML
// ============================================================
void createSecretsTemplate(const std::string& path) {
    std::ofstream file(path);
    file << "# BIG IATE - Secrets Template\n";
    file << "# Заполните пароли и нажмите OK для шифрования\n\n";
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
}

// ============================================================
// ДИАЛОГ ДЛЯ РЕДАКТИРОВАНИЯ СЕКРЕТОВ
// ============================================================
bool editSecretsDialog(wxWindow* parent, std::string& content) {
    wxTextEntryDialog dlg(parent,
        wxString::FromUTF8("Редактируйте секреты (пароли):\n\n"
            "database_password: \"\"\n"
            "cameras:\n"
            "  1:\n"
            "    username: \"\"\n"
            "    password: \"\"\n"
            "  2:\n"
            "    username: \"\"\n"
            "    password: \"\"\n\n"
            "bastion_password: \"\""),
        wxString::FromUTF8("Настройка секретов"),
        wxString::FromUTF8(content),
        wxOK | wxCANCEL);

    if (dlg.ShowModal() == wxID_OK) {
        content = dlg.GetValue().ToStdString();
        return true;
    }
    return false;
}

// ============================================================
// РЕЖИМ ШИФРОВАНИЯ (запускается администратором)
// ============================================================
int runEncryptMode(const std::string& inputFile, const std::string& outputFile, const std::string& password) {
    std::cout << "========================================" << std::endl;
    std::cout << "BIG IATE - Secrets Encryptor" << std::endl;
    std::cout << "========================================" << std::endl;

    std::ifstream in(inputFile);
    if (!in.is_open()) {
        std::cerr << "❌ File not found: " << inputFile << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << in.rdbuf();
    std::string plaintext = buffer.str();
    in.close();

    if (plaintext.empty()) {
        std::cerr << "❌ File is empty: " << inputFile << std::endl;
        return 1;
    }

    if (password.empty()) {
        std::cerr << "❌ Password cannot be empty!" << std::endl;
        return 1;
    }

    auto encrypted = bigiate::secrets::AESEncryption::encrypt(plaintext, password);
    if (!encrypted.has_value()) {
        std::cerr << "❌ Encryption failed: " << encrypted.error() << std::endl;
        return 1;
    }

    std::ofstream out(outputFile, std::ios::binary);
    out.write(encrypted.value().c_str(), static_cast<std::streamsize>(encrypted.value().size()));
    out.close();

    std::cout << "✅ Secrets encrypted successfully!" << std::endl;
    std::cout << "📁 Output: " << outputFile << std::endl;
    std::cout << "⚠️  Delete the original file: " << inputFile << std::endl;

    return 0;
}

// ============================================================
// ОБЫЧНЫЙ РЕЖИМ (запускается охранником)
// ============================================================
class BigIateApp : public wxApp {
public:
    virtual bool OnInit() override {
        // Установка русской локали
        wxLocale locale;
        locale.Init(wxLANGUAGE_RUSSIAN);

        // Консоль для отладки
#ifdef _WIN32
        AllocConsole();
        FILE* f;
        freopen_s(&f, "CONOUT$", "w", stdout);
        freopen_s(&f, "CONOUT$", "w", stderr);
        SetConsoleOutputCP(CP_UTF8);
#endif

        std::cout << "========================================" << std::endl;
        std::cout << "=== BIG IATE - СИСТЕМА КОНТРОЛЯ ДОСТУПА ===" << std::endl;
        std::cout << "========================================" << std::endl;

        // Проверяем, существует ли файл секретов
        std::ifstream checkFile("secrets.yaml.enc");
        bool secretsExist = checkFile.is_open();
        checkFile.close();

        std::string masterPassword;

        // ============================================================
        // ПЕРВЫЙ ЗАПУСК: НЕТ ФАЙЛА СЕКРЕТОВ
        // ============================================================
        if (!secretsExist) {
            std::cout << "🔧 Первый запуск. Настройка секретов..." << std::endl;

            // Создаём шаблон
            createSecretsTemplate("secrets.yaml");

            // Показываем диалог для редактирования
            std::string secretsContent;
            std::ifstream in("secrets.yaml");
            std::stringstream buffer;
            buffer << in.rdbuf();
            secretsContent = buffer.str();
            in.close();

            if (!editSecretsDialog(nullptr, secretsContent)) {
                std::cout << "❌ Отмена настройки секретов" << std::endl;
                return false;
            }

            // Сохраняем отредактированные секреты
            std::ofstream out("secrets.yaml");
            out << secretsContent;
            out.close();

            // Запрашиваем мастер-пароль для шифрования
            wxTextEntryDialog pwdDlg(nullptr,
                wxString::FromUTF8("Введите мастер-пароль для шифрования секретов:"),
                wxString::FromUTF8("Установка мастер-пароля"),
                "",
                wxOK | wxCANCEL | wxTE_PASSWORD);

            if (pwdDlg.ShowModal() != wxID_OK) {
                std::cout << "❌ Отмена установки пароля" << std::endl;
                return false;
            }

            masterPassword = pwdDlg.GetValue().ToStdString();

            if (masterPassword.empty()) {
                wxMessageBox(wxString::FromUTF8("Мастер-пароль не может быть пустым!"),
                    wxString::FromUTF8("Ошибка"),
                    wxOK | wxICON_ERROR);
                return false;
            }

            // Подтверждение пароля
            wxTextEntryDialog confirmDlg(nullptr,
                wxString::FromUTF8("Подтвердите мастер-пароль:"),
                wxString::FromUTF8("Подтверждение"),
                "",
                wxOK | wxCANCEL | wxTE_PASSWORD);

            if (confirmDlg.ShowModal() != wxID_OK) {
                return false;
            }

            std::string confirmPassword = confirmDlg.GetValue().ToStdString();

            if (masterPassword != confirmPassword) {
                wxMessageBox(wxString::FromUTF8("Пароли не совпадают!"),
                    wxString::FromUTF8("Ошибка"),
                    wxOK | wxICON_ERROR);
                return false;
            }

            // Шифруем
            auto result = runEncryptMode("secrets.yaml", "secrets.yaml.enc", masterPassword);
            if (result != 0) {
                return false;
            }

            // Удаляем открытый файл
            std::remove("secrets.yaml");

            std::cout << "✅ Секреты настроены и зашифрованы!" << std::endl;
            wxMessageBox(wxString::FromUTF8("Секреты успешно настроены!\n\nЗапомните мастер-пароль."),
                wxString::FromUTF8("Успех"),
                wxOK | wxICON_INFORMATION);
        }

        // ============================================================
        // ПОСЛЕДУЮЩИЕ ЗАПУСКИ: ЗАПРОС ПАРОЛЯ ДЛЯ РАСШИФРОВКИ
        // ============================================================
        if (masterPassword.empty()) {
            wxTextEntryDialog pwdDlg(nullptr,
                wxString::FromUTF8("Введите мастер-пароль для расшифровки секретов:"),
                wxString::FromUTF8("Аутентификация"),
                "",
                wxOK | wxCANCEL | wxTE_PASSWORD);

            if (pwdDlg.ShowModal() != wxID_OK) {
                std::cout << "❌ Отмена ввода пароля" << std::endl;
                return false;
            }

            masterPassword = pwdDlg.GetValue().ToStdString();
        }

        if (masterPassword.empty()) {
            wxMessageBox(wxString::FromUTF8("Мастер-пароль не может быть пустым!"),
                wxString::FromUTF8("Ошибка"),
                wxOK | wxICON_ERROR);
            return false;
        }

        // ============================================================
        // ЗАГРУЗКА И РАСШИФРОВКА СЕКРЕТОВ
        // ============================================================
        std::cout << "📂 Загрузка секретов..." << std::endl;

        auto secretsResult = bigiate::secrets::SecretsManager::loadFromFile(
            "secrets.yaml.enc", masterPassword);

        if (!secretsResult.has_value()) {
            std::cerr << "❌ Ошибка: " << secretsResult.error() << std::endl;
            wxMessageBox(
                wxString::Format("Не удалось загрузить секреты:\n%s\n\nПроверьте мастер-пароль.",
                    secretsResult.error()),
                wxString::FromUTF8("Ошибка"),
                wxOK | wxICON_ERROR
            );
            return false;
        }

        std::cout << "✅ Секреты загружены!" << std::endl;

        // ============================================================
        // ЗАПУСК СИСТЕМЫ
        // ============================================================
        bigiate::core::AppCore::instance().setSecrets(secretsResult.value());

        std::cout << "🚀 Запуск системы..." << std::endl;
        bool success = bigiate::core::AppCore::instance().run("test_config.yaml");

        if (!success) {
            wxMessageBox(wxString::FromUTF8("Не удалось запустить систему. Проверьте консоль."),
                wxString::FromUTF8("Ошибка"),
                wxOK | wxICON_ERROR);
            return false;
        }

        return true;
    }

    virtual int OnExit() override {
        bigiate::core::AppCore::instance().stop();
        return wxApp::OnExit();
    }
};

wxIMPLEMENT_APP(BigIateApp);

// ============================================================
// ТОЧКА ВХОДА
// ============================================================
int main(int argc, char* argv[]) {
    // Поддержка командной строки для администратора
    if (argc >= 2 && std::string(argv[1]) == "--encrypt") {
        std::string inputFile = (argc >= 3) ? argv[2] : "secrets.yaml";
        std::string password = (argc >= 4) ? argv[3] : "";
        std::string outputFile = (argc >= 5) ? argv[4] : "secrets.yaml.enc";

        if (password.empty()) {
            std::cerr << "❌ Требуется пароль!" << std::endl;
            std::cout << "Использование: BIG_IATE_BR.exe --encrypt <input.yaml> <password> [output.enc]" << std::endl;
            return 1;
        }

        return runEncryptMode(inputFile, outputFile, password);
    }

    return wxEntry(argc, argv);
}
#endif