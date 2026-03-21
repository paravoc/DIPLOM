#ifndef USE_CONSOLE
#include <wx/wx.h>
#include "core/include/AppCore.h"
#include "core/include/SecretsInitializer.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

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

        // ============================================================
        // 1. ИНИЦИАЛИЗАЦИЯ СЕКРЕТОВ
        // ============================================================
        std::cout << "\n🔐 Инициализация секретов..." << std::endl;

        auto secretsResult = bigiate::secrets::SecretsInitializer::initialize();

        if (!secretsResult.has_value()) {
            std::cerr << "❌ Ошибка: " << secretsResult.error() << std::endl;
            wxMessageBox(
                wxString::FromUTF8("Не удалось инициализировать секреты:\n" + secretsResult.error()),
                wxString::FromUTF8("Ошибка"),
                wxOK | wxICON_ERROR
            );
            return false;
        }

        std::cout << "✅ Секреты загружены" << std::endl;

        // ============================================================
        // 2. ЗАПУСК СИСТЕМЫ
        // ============================================================
        std::cout << "\n🚀 Запуск системы..." << std::endl;

        // Передаём секреты в AppCore
        bigiate::core::AppCore::instance().setSecrets(secretsResult.value());

        // Запускаем систему с полным путём к конфигу
        std::string configPath = "C:\\Users\\smidr\\source\\repos\\BIG_IATE_BR\\x64\\Debug\\test_config.yaml";

        bool success = bigiate::core::AppCore::instance().run(configPath);

        if (!success) {
            std::cerr << "❌ Не удалось запустить систему" << std::endl;
            wxMessageBox(
                wxString::FromUTF8("Не удалось запустить систему.\nПроверьте конфигурацию и логи."),
                wxString::FromUTF8("Ошибка"),
                wxOK | wxICON_ERROR
            );
            return false;
        }

        std::cout << "\n✅ Система работает" << std::endl;
        return true;
    }

    virtual int OnExit() override {
        std::cout << "\n🛑 Остановка системы..." << std::endl;
        bigiate::core::AppCore::instance().stop();
        return wxApp::OnExit();
    }
};

wxIMPLEMENT_APP(BigIateApp);

// ============================================================
// ТОЧКА ВХОДА (поддержка режима шифрования)
// ============================================================
int main(int argc, char* argv[]) {
    // Режим шифрования для администратора
    if (argc >= 2 && std::string(argv[1]) == "--encrypt") {
        std::string inputFile = (argc >= 3) ? argv[2] : "secrets.yaml";
        std::string password = (argc >= 4) ? argv[3] : "";
        std::string outputFile = (argc >= 5) ? argv[4] : "secrets.yaml.enc";

        if (password.empty()) {
            std::cerr << "❌ Требуется пароль!" << std::endl;
            std::cout << "Использование: BIG_IATE_BR.exe --encrypt <input.yaml> <password> [output.enc]" << std::endl;
            return 1;
        }

        auto result = bigiate::secrets::SecretsInitializer::encryptSecrets(
            inputFile, outputFile, password
        );

        if (result.has_value()) {
            std::cout << "✅ Файл зашифрован: " << outputFile << std::endl;
            return 0;
        }
        else {
            std::cerr << "❌ Ошибка шифрования: " << result.error() << std::endl;
            return 1;
        }
    }

    // Обычный запуск
    return wxEntry(argc, argv);
}
#endif