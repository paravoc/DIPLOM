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
        // ИНИЦИАЛИЗАЦИЯ СЕКРЕТОВ (автоматически определяет первый запуск)
        // ============================================================
        auto secretsResult = bigiate::secrets::SecretsInitializer::initialize();

        if (!secretsResult.has_value()) {
            std::cerr << "❌ " << secretsResult.error() << std::endl;
            wxMessageBox(wxString::FromUTF8(secretsResult.error()),
                wxString::FromUTF8("Ошибка"),
                wxOK | wxICON_ERROR);
            return false;
        }

        // ============================================================
        // ЗАПУСК СИСТЕМЫ
        // ============================================================
        bigiate::core::AppCore::instance().setSecrets(secretsResult.value());

        std::cout << "🚀 Запуск системы..." << std::endl;
        bool success = bigiate::core::AppCore::instance().run(
            "C:\\Users\\smidr\\source\\repos\\BIG_IATE_BR\\x64\\Debug\\test_config.yaml"
        );

        if (!success) {
            wxMessageBox(wxString::FromUTF8("Не удалось запустить систему."),
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
// ТОЧКА ВХОДА (поддержка командной строки)
// ============================================================
int main(int argc, char* argv[]) {
    // Режим шифрования для администратора (опционально)
    if (argc >= 2 && std::string(argv[1]) == "--encrypt") {
        std::string inputFile = (argc >= 3) ? argv[2] : "secrets.yaml";
        std::string password = (argc >= 4) ? argv[3] : "";
        std::string outputFile = (argc >= 5) ? argv[4] : "secrets.yaml.enc";

        if (password.empty()) {
            std::cerr << "❌ Password required!" << std::endl;
            std::cout << "Usage: BIG_IATE_BR.exe --encrypt <input.yaml> <password> [output.enc]" << std::endl;
            return 1;
        }

        return bigiate::secrets::SecretsInitializer::encryptSecrets(inputFile, outputFile, password).has_value() ? 0 : 1;
    }

    return wxEntry(argc, argv);
}
#endif