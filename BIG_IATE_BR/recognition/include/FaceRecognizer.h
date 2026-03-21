#pragma once

#include <memory>
#include <vector>
#include <expected>
#include "FaceDetector.h"
#include "FaceExtractor.h"
#include "RecognitionResult.h"
#include "../../configs/include/types.h"
#include "../../database/include/DBQueries.h"

namespace bigiate::recognition {

    class FaceRecognizer {
    public:
        FaceRecognizer();
        ~FaceRecognizer() = default;

        // Инициализация из конфига
        [[nodiscard]] std::expected<void, std::string> init(
            const config::RecognitionConfig& cfg,
            std::shared_ptr<db::DBQueries> dbQueries
        );

        // Распознавание на кадре
        [[nodiscard]] std::vector<RecognitionResult> recognize(
            const cv::Mat& frame,
            int cameraId,
            float threshold = 0.75f
        );

        // Проверка прав доступа
        bool checkAccess(const Match& match, int cameraId);

    private:
        std::unique_ptr<FaceDetector> m_detector;
        std::unique_ptr<FaceExtractor> m_extractor;
        std::shared_ptr<db::DBQueries> m_dbQueries;
        config::RecognitionConfig m_config;
        bool m_initialized{ false };
    };

} // namespace bigiate::recognition