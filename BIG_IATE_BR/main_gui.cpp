#ifndef USE_CONSOLE
#include <wx/wx.h>
#include "../../core/include/AppCore.h"
#include <iostream>


#ifdef _WIN32
#include <windows.h>
#endif

// Класс приложения
class BigIateApp : public wxApp {
public:
    virtual bool OnInit() override {
        // Включаем консоль для отладки
#ifdef _WIN32
        AllocConsole();
        FILE* f;
        freopen_s(&f, "CONOUT$", "w", stdout);
        freopen_s(&f, "CONOUT$", "w", stderr);
        SetConsoleOutputCP(CP_UTF8);
#endif

        std::cout << "=== ЗАПУСК BIG IATE ===" << std::endl;

        // ЕДИНСТВЕННАЯ ФУНКЦИЯ — запускает всё!
        bool success = bigiate::core::AppCore::instance().run(
            "C:\\Users\\smidr\\source\\repos\\BIG_IATE_BR\\x64\\Debug\\test_config.yaml"
        );

        return success;
    }

    virtual int OnExit() override {
        bigiate::core::AppCore::instance().stop();
        return wxApp::OnExit();
    }
};

wxIMPLEMENT_APP(BigIateApp);
#endif