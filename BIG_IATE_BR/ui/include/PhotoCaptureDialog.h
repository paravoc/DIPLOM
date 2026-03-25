#pragma once

#include <wx/wx.h>
#include <wx/timer.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include <memory>
#include "../../configs/include/types.h"
#include "../../recognition/include/FaceRecognizer.h"

class PhotoCaptureDialog : public wxDialog {
public:
    PhotoCaptureDialog(wxWindow* parent,
        std::shared_ptr<bigiate::recognition::FaceRecognizer> recognizer,
        const std::vector<bigiate::config::CameraConfig>& cameras);
    ~PhotoCaptureDialog();

    struct PhotoData {
        cv::Mat faceImage;
        cv::Mat fullFrame;
        std::vector<float> embedding;
        cv::Rect faceRect;
        float confidence;
    };

    std::vector<PhotoData> GetCapturedPhotos() const { return m_capturedPhotos; }
    bool HasPhotos() const { return !m_capturedPhotos.empty(); }

private:
    void OnTimer(wxTimerEvent& event);
    void OnCapture(wxCommandEvent& event);
    void OnClose(wxCommandEvent& event);
    void OnSelectCamera(wxCommandEvent& event);
    void UpdateFrame();


    wxStaticBitmap* m_videoBitmap;
    wxButton* m_captureBtn;
    wxButton* m_closeBtn;
    wxChoice* m_cameraChoice;
    wxStaticText* m_statusText;
    wxTimer* m_timer;

    cv::VideoCapture m_cap;
    cv::Mat m_currentFrame;
    cv::Mat m_currentFace;
    std::vector<float> m_currentEmbedding;
    cv::Rect m_faceRect;
    float m_faceConfidence;
    bool m_faceDetected;

    std::vector<PhotoData> m_capturedPhotos;
    int m_selectedCameraId;

    std::shared_ptr<bigiate::recognition::FaceRecognizer> m_recognizer;
    std::vector<bigiate::config::CameraConfig> m_cameras;

    wxDECLARE_EVENT_TABLE();
};