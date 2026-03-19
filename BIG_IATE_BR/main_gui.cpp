
#ifndef USE_CONSOLE
#include <wx/wx.h>
#include "ui/include/MainFrame.h"
#include "configs/include/loader.h"
#include <iostream>

namespace config = bigiate::config;

// Класс приложения
class BigIateApp : public wxApp {
private:
    config::LoadResult m_configResult;

public:
    virtual bool OnInit() override {
        // Настройка локали (для русского языка)
        wxLocale locale;
        locale.Init(wxLANGUAGE_RUSSIAN);

        // Загружаем конфиг
        std::string config_file = "C:\\Users\\smidr\\source\\repos\\BIG_IATE_BR\\x64\\Debug\\test_config.yaml";

        wxPrintf("Загрузка конфига: %s\n", config_file);

        auto result = config::LoadConfig(config_file);

        if (!result.has_value()) {
            wxMessageBox(
                wxString::Format("Ошибка загрузки конфига:\n%s", result.error()),
                "Ошибка",
                wxOK | wxICON_ERROR
            );
            // Можно продолжить с дефолтными настройками
        }
        else {
            m_configResult = *result;
            wxPrintf("Конфиг загружен: версия %s, камер: %zu\n",
                m_configResult.config.version.toString(),
                m_configResult.config.cameras.size());
        }

        // Создаём главное окно и передаём конфиг
        MainFrame* frame = new MainFrame("BIG IATE - Система контроля доступа");
        //frame->SetConfig(m_configResult.config);  // нужно добавить этот метод
        frame->Show(true);

        return true;
    }
};

wxIMPLEMENT_APP(BigIateApp);
#endif