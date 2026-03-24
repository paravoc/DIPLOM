#ifndef USE_CONSOLE
#include <wx/wx.h>
#include "core/include/AppCore.h"
#include "core/include/PeopleManager.h"
#include "core/include/SecretsInitializer.h"
#include "ui/include/StartupDialog.h"
#include "ui/include/AddPersonDialog.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

class BigIateApp : public wxApp {
public:
    virtual bool OnInit() override {
        wxLocale locale;
        locale.Init(wxLANGUAGE_RUSSIAN);

#ifdef _WIN32
        AllocConsole();
        FILE* f;
        freopen_s(&f, "CONOUT$", "w", stdout);
        freopen_s(&f, "CONOUT$", "w", stderr);
        SetConsoleOutputCP(CP_UTF8);
#endif

        std::string configPath = "C:\\Users\\smidr\\source\\repos\\BIG_IATE_BR\\x64\\Debug\\test_config.yaml";

        // Цикл для возврата в меню после операций
        while (true) {
            StartupDialog dlg(nullptr);

            if (dlg.ShowModal() != wxID_OK) {
                return false;  // Выход
            }

            auto action = dlg.GetSelectedAction();

            // ============================================================
            // ОСНОВНОЙ РЕЖИМ
            // ============================================================
            if (action == StartupDialog::ACTION_RUN) {
                std::cout << "\n=== ЗАПУСК ОСНОВНОГО РЕЖИМА ===\n" << std::endl;

                auto secretsResult = bigiate::secrets::SecretsInitializer::initialize();
                if (!secretsResult.has_value()) {
                    wxMessageBox(
                        wxString::FromUTF8("Ошибка: " + secretsResult.error()),
                        wxString::FromUTF8("Ошибка"),
                        wxOK | wxICON_ERROR
                    );
                    return false;
                }

                bigiate::core::AppCore::instance().setSecrets(secretsResult.value());
                bool success = bigiate::core::AppCore::instance().run(configPath);

                if (!success) {
                    wxMessageBox(
                        wxString::FromUTF8("Не удалось запустить систему"),
                        wxString::FromUTF8("Ошибка"),
                        wxOK | wxICON_ERROR
                    );
                    return false;
                }

                return true;  // Основной режим запущен, выходим из цикла
            }

            // ============================================================
            // ДОБАВЛЕНИЕ ЧЕЛОВЕКА
            // ============================================================
            else if (action == StartupDialog::ACTION_ADD_PERSON) {
                AddPersonDialog dlg(nullptr);

                if (dlg.ShowModal() != wxID_OK) {
                    continue;  // Вернуться в меню
                }

                auto data = dlg.GetData();

                bigiate::core::PeopleManager manager;
                auto initResult = manager.init(configPath);

                if (!initResult.has_value()) {
                    wxMessageBox(
                        wxString::FromUTF8("Ошибка: " + initResult.error()),
                        wxString::FromUTF8("Ошибка"),
                        wxOK | wxICON_ERROR
                    );
                    continue;
                }

                auto result = manager.addPersonFromPhoto(
                    data.imagePath, data.fullName, data.personType
                );

                if (result.has_value()) {
                    wxMessageBox(
                        wxString::FromUTF8("✅ Добавлен!\n\nID: " + std::to_string(result.value()) +
                            "\nФИО: " + data.fullName +
                            "\nТип: " + data.personType),
                        wxString::FromUTF8("Успех"),
                        wxOK | wxICON_INFORMATION
                    );
                }
                else {
                    wxMessageBox(
                        wxString::FromUTF8("❌ Ошибка:\n" + result.error()),
                        wxString::FromUTF8("Ошибка"),
                        wxOK | wxICON_ERROR
                    );
                }

                manager.close();
                // continue - вернёмся в меню
            }

            // ============================================================
            // СПИСОК ЛЮДЕЙ
            // ============================================================
            else if (action == StartupDialog::ACTION_LIST_PERSONS) {
                bigiate::core::PeopleManager manager;
                auto initResult = manager.init(configPath);

                if (!initResult.has_value()) {
                    wxMessageBox(
                        wxString::FromUTF8("Ошибка: " + initResult.error()),
                        wxString::FromUTF8("Ошибка"),
                        wxOK | wxICON_ERROR
                    );
                    continue;
                }

                auto result = manager.listAllPersons();

                if (!result.has_value()) {
                    wxMessageBox(
                        wxString::FromUTF8("Ошибка: " + result.error()),
                        wxString::FromUTF8("Ошибка"),
                        wxOK | wxICON_ERROR
                    );
                }
                else {
                    wxMessageBox(
                        wxString::FromUTF8("Список выведен в консоль"),
                        wxString::FromUTF8("Готово"),
                        wxOK | wxICON_INFORMATION
                    );
                }

                manager.close();
                // continue - вернёмся в меню
            }

            // ============================================================
            // ВЫХОД
            // ============================================================
            else if (action == StartupDialog::ACTION_EXIT) {
                return false;
            }
        }

        return false;
    }

    virtual int OnExit() override {
        bigiate::core::AppCore::instance().stop();
        return wxApp::OnExit();
    }
};

wxIMPLEMENT_APP(BigIateApp);

int main(int argc, char* argv[]) {
    // Режим шифрования
    if (argc >= 2 && std::string(argv[1]) == "--encrypt") {
        std::string inputFile = (argc >= 3) ? argv[2] : "secrets.yaml";
        std::string password = (argc >= 4) ? argv[3] : "";
        std::string outputFile = (argc >= 5) ? argv[4] : "secrets.yaml.enc";

        if (password.empty()) {
            std::cerr << "❌ Требуется пароль!" << std::endl;
            return 1;
        }

        auto result = bigiate::secrets::SecretsInitializer::encryptSecrets(
            inputFile, outputFile, password
        );

        if (result.has_value()) {
            std::cout << "✅ Зашифровано: " << outputFile << std::endl;
            return 0;
        }
        else {
            std::cerr << "❌ Ошибка: " << result.error() << std::endl;
            return 1;
        }
    }

    return wxEntry(argc, argv);
}
#endif