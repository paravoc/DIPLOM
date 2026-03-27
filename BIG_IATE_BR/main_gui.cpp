#ifndef USE_CONSOLE
#include <wx/wx.h>
#include "core/include/AppCore.h"
#include "core/include/PeopleManager.h"
#include "core/include/SecretsInitializer.h"
#include "ui/include/StartupDialog.h"
#include "ui/include/AddPersonDialog.h"
#include "ui/include/DeleteEncodingDialog.h"
#include <iostream>
#include <map>

#ifdef _WIN32
#include <windows.h>
#endif

static std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

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

        while (true) {
            StartupDialog dlg(nullptr);

            if (dlg.ShowModal() != wxID_OK) {
                return false;
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

                return true;
            }

            // ============================================================
            // ДОБАВЛЕНИЕ ЧЕЛОВЕКА
            // ============================================================
// ============================================================
// ДОБАВЛЕНИЕ ЧЕЛОВЕКА (1 человек = много фото)
// ============================================================
            else if (action == StartupDialog::ACTION_ADD_PERSON) {
                std::cout << "\n=== ДОБАВЛЕНИЕ ЧЕЛОВЕКА ===\n" << std::endl;

                bigiate::core::PeopleManager manager;
                auto initResult = manager.init(configPath);

                if (!initResult.has_value()) {
                    wxMessageBox(
                        wxString::FromUTF8("Ошибка инициализации:\n" + initResult.error()),
                        wxString::FromUTF8("Ошибка"),
                        wxOK | wxICON_ERROR
                    );
                    continue;
                }

                AddPersonDialog addDlg(nullptr, manager.getRecognizer(), manager.getCameras());

                if (addDlg.ShowModal() != wxID_OK) {
                    manager.close();
                    continue;
                }

                auto data = addDlg.GetData();

                if (data.imagePaths.empty()) {
                    manager.close();
                    wxMessageBox(_T("Нет фото для добавления"), _T("Ошибка"), wxOK | wxICON_ERROR);
                    continue;
                }

                if (data.fullName.empty()) {
                    manager.close();
                    wxMessageBox(_T("Введите ФИО"), _T("Ошибка"), wxOK | wxICON_ERROR);
                    continue;
                }

                // ========== 1. СОЗДАЁМ ЧЕЛОВЕКА (ОДИН РАЗ) ==========
                bigiate::db::Person person;
                person.fullName = data.fullName;
                person.personType = data.personType;
                person.birthDate = data.birthDate;
                person.gender = data.gender;
                person.phone = data.phone;
                person.email = data.email;
                person.address = data.address;
                person.isActive = true;
                person.isBlocked = false;

                auto personIdResult = manager.addPersonOnly(person);

                if (!personIdResult.has_value()) {
                    wxMessageBox(
                        wxString::FromUTF8("❌ Ошибка при добавлении человека:\n" + personIdResult.error()),
                        wxString::FromUTF8("Ошибка"),
                        wxOK | wxICON_ERROR
                    );
                    manager.close();
                    continue;
                }

                int personId = personIdResult.value();
                std::cout << "✅ Добавлен человек с ID: " << personId << std::endl;

                // ========== 2. ДОБАВЛЯЕМ ВСЕ ФОТО К ЭТОМУ ЧЕЛОВЕКУ ==========
                int successCount = 0;

                for (size_t i = 0; i < data.imagePaths.size(); i++) {
                    const auto& path = data.imagePaths[i];
                    std::cout << "📸 Обработка фото " << (i + 1) << ": " << path << std::endl;

                    bigiate::db::FaceEncoding encoding;
                    encoding.personId = personId;
                    encoding.sourceImagePath = path;
                    encoding.captureDate = getCurrentTimestamp();
                    encoding.isPrimary = (i == 0);  // Первое фото — основное
                    encoding.isCurrent = true;
                    encoding.qualityScore = 1.0f;
                    encoding.faceSize = 0;

                    // Если есть готовый эмбеддинг из камеры
                    if (i < data.embeddings.size() && !data.embeddings[i].empty()) {
                        encoding.embedding = data.embeddings[i];
                        auto encResult = manager.addEncodingToPerson(personId, encoding);
                        if (encResult.has_value()) {
                            successCount++;
                            std::cout << "✅ Фото " << (i + 1) << " добавлено (из камеры)" << std::endl;
                        }
                        else {
                            std::cerr << "❌ Ошибка: " << encResult.error() << std::endl;
                        }
                    }
                    // Иначе загружаем из файла
                    else {
                        auto imageResult = manager.loadImageFromPath(path);
                        if (!imageResult.has_value()) {
                            std::cerr << "❌ Не удалось загрузить фото: " << imageResult.error() << std::endl;
                            continue;
                        }

                        auto embeddingResult = manager.extractEmbedding(imageResult.value());
                        if (!embeddingResult.has_value()) {
                            std::cerr << "❌ Не удалось извлечь эмбеддинг: " << embeddingResult.error() << std::endl;
                            continue;
                        }

                        encoding.embedding = embeddingResult.value();
                        auto encResult = manager.addEncodingToPerson(personId, encoding);
                        if (encResult.has_value()) {
                            successCount++;
                            std::cout << "✅ Фото " << (i + 1) << " добавлено (из файла)" << std::endl;
                        }
                        else {
                            std::cerr << "❌ Ошибка: " << encResult.error() << std::endl;
                        }
                    }
                }

                if (successCount > 0) {
                    wxString message = wxString::Format(
                        _T("✅ Человек успешно добавлен!\n\nID: %d\nФИО: %s\nТип: %s\nДобавлено фото: %d"),
                        personId,
                        wxString::FromUTF8(data.fullName),
                        wxString::FromUTF8(data.personType),
                        successCount
                    );
                    wxMessageBox(message, _T("Успех"), wxOK | wxICON_INFORMATION);
                }
                else {
                    wxMessageBox(
                        wxString::FromUTF8("❌ Не удалось добавить ни одного фото.\nЧеловек не был добавлен."),
                        _T("Ошибка"),
                        wxOK | wxICON_ERROR
                    );
                    manager.deletePerson(personId);
                }

                manager.close();
            }
            // ============================================================
            // СПИСОК ЛЮДЕЙ
            // ============================================================
            else if (action == StartupDialog::ACTION_LIST_PERSONS) {
                std::cout << "\n=== СПИСОК ЛЮДЕЙ ===\n" << std::endl;

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
            }

            // ============================================================
            // УДАЛЕНИЕ ФОТО
            // ============================================================
            else if (action == StartupDialog::ACTION_DELETE_ENCODING) {
                std::cout << "\n=== УДАЛЕНИЕ ФОТО ===\n" << std::endl;

                bigiate::core::PeopleManager manager;
                auto initResult = manager.init(configPath);

                if (!initResult.has_value()) {
                    wxMessageBox(
                        wxString::FromUTF8("Ошибка инициализации:\n" + initResult.error()),
                        wxString::FromUTF8("Ошибка"),
                        wxOK | wxICON_ERROR
                    );
                    continue;
                }

                auto personsResult = manager.getAllPersonsInfo(false);
                if (!personsResult.has_value()) {
                    wxMessageBox(
                        wxString::FromUTF8("Ошибка получения списка людей:\n" + personsResult.error()),
                        wxString::FromUTF8("Ошибка"),
                        wxOK | wxICON_ERROR
                    );
                    manager.close();
                    continue;
                }

                const auto& persons = personsResult.value();
                if (persons.empty()) {
                    wxMessageBox(_T("Нет зарегистрированных людей"), _T("Информация"), wxOK | wxICON_INFORMATION);
                    manager.close();
                    continue;
                }
                
                std::map<int, std::vector<bigiate::db::FaceEncoding>> encodingsMap;
                for (const auto& p : persons) {
                    auto encResult = manager.getFaceEncodingsByPerson(p.id);
                    if (encResult.has_value() && !encResult.value().empty()) {
                        encodingsMap[p.id] = encResult.value();
                    }
                }

                bool hasPhotos = false;
                for (const auto& item : encodingsMap) {
                    if (!item.second.empty()) {
                        hasPhotos = true;
                        break;
                    }
                }

                if (!hasPhotos) {
                    wxMessageBox(_T("У выбранных людей нет фото для удаления"), _T("Информация"), wxOK | wxICON_INFORMATION);
                    manager.close();
                    continue;
                }

                DeleteEncodingDialog deleteDlg(nullptr, persons, encodingsMap);

                if (deleteDlg.ShowModal() == wxID_OK) {
                    auto selected = deleteDlg.GetSelected();

                    if (selected.encodingId != -1) {
                        auto result = manager.deleteEncoding(selected.encodingId);

                        if (result.has_value()) {
                            wxMessageBox(
                                wxString::Format(_T("✅ Фото успешно удалено!\n\nЧеловек: %s\nID фото: %d"),
                                    wxString::FromUTF8(selected.personName),
                                    selected.encodingId),
                                _T("Успех"),
                                wxOK | wxICON_INFORMATION
                            );
                        }
                        else {
                            wxMessageBox(
                                wxString::FromUTF8("❌ Ошибка удаления:\n" + result.error()),
                                _T("Ошибка"),
                                wxOK | wxICON_ERROR
                            );
                        }
                    }
                }

                manager.close();
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