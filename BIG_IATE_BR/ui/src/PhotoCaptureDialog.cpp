#include "../include/PhotoCaptureDialog.h"
#include "../../recognition/include/FaceDetector.h"
#include "../../recognition/include/FaceExtractor.h"
#include <wx/statline.h>
#include <wx/msgdlg.h>

#define _T(str) wxString::FromUTF8(str)

enum {
    ID_TIMER = 1000,
    ID_CAPTURE,
    ID_CLOSE,
    ID_SELECT_CAMERA
};

wxBEGIN_EVENT_TABLE(PhotoCaptureDialog, wxDialog)
EVT_TIMER(ID_TIMER, PhotoCaptureDialog::OnTimer)
EVT_BUTTON(ID_CAPTURE, PhotoCaptureDialog::OnCapture)
EVT_BUTTON(ID_CLOSE, PhotoCaptureDialog::OnClose)
EVT_CHOICE(ID_SELECT_CAMERA, PhotoCaptureDialog::OnSelectCamera)
wxEND_EVENT_TABLE()

PhotoCaptureDialog::PhotoCaptureDialog(wxWindow* parent,
    std::shared_ptr<bigiate::recognition::FaceRecognizer> recognizer,
    const std::vector<bigiate::config::CameraConfig>& cameras)
    : wxDialog(parent, wxID_ANY, _T("Съёмка лица"),
        wxDefaultPosition, wxSize(900, 700)),
    m_recognizer(recognizer),
    m_cameras(cameras),
    m_selectedCameraId(-1),
    m_faceDetected(false),
    m_faceConfidence(0.0f) {

    SetBackgroundColour(wxColour(35, 35, 45));

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Заголовок
    wxStaticText* title = new wxStaticText(this, wxID_ANY, _T("СЪЁМКА ЛИЦА"));
    title->SetFont(wxFont(18, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    title->SetForegroundColour(wxColour(0, 180, 255));
    mainSizer->Add(title, 0, wxALIGN_CENTER | wxTOP, 15);

    // ========== ВЕРХНЯЯ ПАНЕЛЬ С КНОПКАМИ ==========
    wxPanel* topPanel = new wxPanel(this, wxID_ANY);
    topPanel->SetBackgroundColour(wxColour(45, 45, 55));

    wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);

    // Выбор камеры
    topSizer->Add(new wxStaticText(topPanel, wxID_ANY, _T("Камера:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);

    m_cameraChoice = new wxChoice(topPanel, ID_SELECT_CAMERA, wxDefaultPosition, wxSize(200, 28));

    // Добавляем камеры из конфига
    for (const auto& cam : m_cameras) {
        if (cam.enabled) {
            wxString name = wxString::Format(_T("%s (ID: %d)"),
                wxString::FromUTF8(cam.name), cam.id);
            m_cameraChoice->Append(name);
        }
    }

    // Добавляем USB камеры по умолчанию
    for (int i = 0; i < 5; i++) {
        cv::VideoCapture test(i);
        if (test.isOpened()) {
            bool exists = false;
            for (const auto& cam : m_cameras) {
                if (cam.connection.device.has_value() &&
                    std::stoi(cam.connection.device.value()) == i) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                m_cameraChoice->Append(wxString::Format(_T("USB камера %d"), i));
            }
            test.release();
        }
    }

    if (m_cameraChoice->GetCount() == 0) {
        m_cameraChoice->Append(_T("Нет доступных камер"));
    }
    m_cameraChoice->SetSelection(0);
    topSizer->Add(m_cameraChoice, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 20);

    // Кнопка "Сфотографировать"
    m_captureBtn = new wxButton(topPanel, ID_CAPTURE, _T("📷 СФОТОГРАФИРОВАТЬ"), wxDefaultPosition, wxSize(180, 38));
    m_captureBtn->SetBackgroundColour(wxColour(0, 140, 0));
    m_captureBtn->SetForegroundColour(*wxWHITE);
    m_captureBtn->SetFont(wxFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    m_captureBtn->Enable(false);
    topSizer->Add(m_captureBtn, 0, wxRIGHT, 15);

    // Кнопка "Готово"
    wxButton* closeBtn = new wxButton(topPanel, ID_CLOSE, _T("ГОТОВО"), wxDefaultPosition, wxSize(100, 38));
    closeBtn->SetBackgroundColour(wxColour(140, 60, 60));
    closeBtn->SetForegroundColour(*wxWHITE);
    closeBtn->SetFont(wxFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    topSizer->Add(closeBtn, 0);

    topSizer->AddStretchSpacer();
    topPanel->SetSizer(topSizer);
    mainSizer->Add(topPanel, 0, wxEXPAND | wxALL, 10);

    // Видео область
    m_videoBitmap = new wxStaticBitmap(this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize(800, 560));
    m_videoBitmap->SetBackgroundColour(wxColour(20, 20, 30));
    m_videoBitmap->SetMinSize(wxSize(800, 560));
    mainSizer->Add(m_videoBitmap, 1, wxALIGN_CENTER | wxALL, 10);

    // Статус
    m_statusText = new wxStaticText(this, wxID_ANY, _T("Ожидание запуска камеры..."));
    m_statusText->SetForegroundColour(wxColour(200, 200, 200));
    m_statusText->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    mainSizer->Add(m_statusText, 0, wxALIGN_CENTER | wxBOTTOM, 10);

    // Информация
    wxStaticText* infoText = new wxStaticText(this, wxID_ANY,
        _T("💡 Совет: сделайте 2-3 фото с разных ракурсов для лучшего распознавания"));
    infoText->SetForegroundColour(wxColour(150, 150, 170));
    infoText->SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    mainSizer->Add(infoText, 0, wxALIGN_CENTER | wxBOTTOM, 10);

    SetSizer(mainSizer);

    // Запускаем таймер
    m_timer = new wxTimer(this, ID_TIMER);
    m_timer->Start(33);  // ~30 FPS

    // Открываем первую камеру
    wxCommandEvent dummy;
    OnSelectCamera(dummy);

    Centre();
}

PhotoCaptureDialog::~PhotoCaptureDialog() {
    if (m_timer) {
        m_timer->Stop();
        delete m_timer;
    }
    if (m_cap.isOpened()) {
        m_cap.release();
    }
}

void PhotoCaptureDialog::OnSelectCamera(wxCommandEvent& event) {
    if (m_cap.isOpened()) {
        m_cap.release();
    }

    int selection = m_cameraChoice->GetSelection();
    if (selection < 0) {
        m_statusText->SetLabel(_T("❌ Нет доступных камер"));
        return;
    }

    wxString selected = m_cameraChoice->GetString(selection);

    // Пытаемся найти камеру в конфиге
    int cameraId = -1;
    int usbIndex = -1;

    // Сначала ищем по имени в конфиге
    for (size_t i = 0; i < m_cameras.size(); i++) {
        wxString camName = wxString::Format(_T("%s (ID: %d)"),
            wxString::FromUTF8(m_cameras[i].name),
            m_cameras[i].id);
        if (camName == selected) {
            cameraId = m_cameras[i].id;
            if (m_cameras[i].connection.device.has_value()) {
                usbIndex = std::stoi(m_cameras[i].connection.device.value());
            }
            break;
        }
    }

    // Если не нашли, пробуем USB камеру
    if (cameraId == -1 && selected.StartsWith("USB камера")) {
        usbIndex = wxAtoi(selected.substr(10));
    }

    // Открываем камеру
    if (usbIndex >= 0) {
        m_cap.open(usbIndex);
        m_selectedCameraId = usbIndex;
    }
    else if (cameraId >= 0) {
        // Для RTSP камеры нужно формировать строку подключения
        for (const auto& cam : m_cameras) {
            if (cam.id == cameraId) {
                std::string url = cam.connection.protocol + "://" +
                    cam.connection.host + ":" +
                    std::to_string(cam.connection.port) +
                    cam.connection.path;
                m_cap.open(url);
                m_selectedCameraId = cam.id;
                break;
            }
        }
    }

    if (!m_cap.isOpened()) {
        m_statusText->SetLabel(_T("❌ Не удалось открыть камеру"));
        m_captureBtn->Enable(false);
        return;
    }

    m_cap.set(cv::CAP_PROP_FRAME_WIDTH, 800);
    m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, 560);

    m_statusText->SetLabel(_T("✅ Камера готова. Поднесите лицо к камере"));
}

void PhotoCaptureDialog::UpdateFrame() {
    if (!m_cap.isOpened()) return;

    m_cap >> m_currentFrame;
    if (m_currentFrame.empty()) return;

    // Пропускаем кадры для плавности
    static int frameCounter = 0;
    frameCounter++;

    if (frameCounter >= 3) {
        frameCounter = 0;

        m_faceDetected = false;
        m_faceRect = cv::Rect();
        m_currentEmbedding.clear();

        if (m_recognizer) {
            auto results = m_recognizer->recognize(m_currentFrame, 0, 0.5f);

            if (!results.empty() && results[0].detection.bbox.width > 0) {
                m_faceDetected = true;
                m_faceRect = results[0].detection.bbox;
                m_faceConfidence = results[0].detection.confidence;
                m_currentFace = results[0].detection.faceROI.clone();
                m_currentEmbedding = results[0].embedding.vector;

                m_captureBtn->Enable(true);
                m_statusText->SetLabel(wxString::Format(_T("✅ Лицо обнаружено! (уверенность: %.0f%%) Сделано фото: %d"),
                    m_faceConfidence * 100,
                    (int)m_capturedPhotos.size()));
            }
            else {
                m_captureBtn->Enable(false);
                m_statusText->SetLabel(wxString::Format(_T("❌ Лицо не обнаружено (%d фото сделано)"),
                    (int)m_capturedPhotos.size()));
            }
        }
    }

    // Рисуем прямоугольник
    cv::Mat displayFrame = m_currentFrame.clone();
    if (m_faceDetected && m_faceRect.width > 0) {
        cv::rectangle(displayFrame, m_faceRect, cv::Scalar(0, 255, 0), 2);
    }

    // Отображаем
    cv::Mat rgb;
    cv::cvtColor(displayFrame, rgb, cv::COLOR_BGR2RGB);
    wxImage image(rgb.cols, rgb.rows, rgb.data, true);

    wxSize size = m_videoBitmap->GetSize();
    if (size.GetWidth() > 0 && size.GetHeight() > 0) {
        wxImage scaled = image.Scale(size.GetWidth(), size.GetHeight(), wxIMAGE_QUALITY_HIGH);
        m_videoBitmap->SetBitmap(wxBitmap(scaled));
    }
    else {
        m_videoBitmap->SetBitmap(wxBitmap(image));
    }
}

void PhotoCaptureDialog::OnTimer(wxTimerEvent& event) {
    UpdateFrame();
}

void PhotoCaptureDialog::OnCapture(wxCommandEvent& event) {
    if (!m_faceDetected || m_currentFace.empty()) {
        wxMessageBox(_T("Лицо не обнаружено. Поднесите лицо к камере и попробуйте снова."),
            _T("Ошибка"), wxOK | wxICON_ERROR);
        return;
    }

    PhotoData data;
    data.faceImage = m_currentFace.clone();
    data.fullFrame = m_currentFrame.clone();
    data.embedding = m_currentEmbedding;
    data.faceRect = m_faceRect;
    data.confidence = m_faceConfidence;

    m_capturedPhotos.push_back(data);

    m_statusText->SetLabel(wxString::Format(_T("✅ Фото сохранено! Сделано фото: %d"),
        (int)m_capturedPhotos.size()));

    wxBell();
}

void PhotoCaptureDialog::OnClose(wxCommandEvent& event) {
    if (m_cap.isOpened()) {
        m_cap.release();
    }
    EndModal(wxID_OK);
}