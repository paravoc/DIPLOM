#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <vector>
#include <expected>
#include <string>
#include "RecognitionResult.h"

namespace bigiate::recognition {

    class FaceDetector {
    public:
        FaceDetector();
        ~FaceDetector() = default;

        // Загрузка модели из конфига
        [[nodiscard]] std::expected<void, std::string> load(
            const std::string& modelPath,
            const std::optional<std::string>& configPath,
            float confidenceThreshold = 0.5f,
            int inputWidth = 300,
            int inputHeight = 300
        );

        // Детекция лиц на кадре
        [[nodiscard]] std::vector<Detection> detect(const cv::Mat& frame, int cameraId = 0);

        // Проверка, загружена ли модель
        [[nodiscard]] bool isLoaded() const { return !m_net.empty(); }

    private:
        cv::dnn::Net m_net;
        float m_confidenceThreshold;
        int m_inputWidth;
        int m_inputHeight;
    };

} // namespace bigiate::recognition