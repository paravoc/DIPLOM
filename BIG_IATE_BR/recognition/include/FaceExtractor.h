#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <vector>
#include <expected>
#include <string>
#include "RecognitionResult.h"

namespace bigiate::recognition {

    class FaceExtractor {
    public:
        FaceExtractor();
        ~FaceExtractor() = default;

        // Загрузка модели из конфига
        [[nodiscard]] std::expected<void, std::string> load(
            const std::string& modelPath,
            int embeddingSize = 512,
            bool normalize = true
        );

        // Извлечение эмбеддинга из изображения лица
        [[nodiscard]] std::expected<Embedding, std::string> extract(const cv::Mat& faceImage);

        // Проверка, загружена ли модель
        [[nodiscard]] bool isLoaded() const { return !m_net.empty(); }

    private:
        cv::dnn::Net m_net;
        int m_embeddingSize;
        bool m_normalize;

        cv::Mat preprocessFace(const cv::Mat& face);
    };

} // namespace bigiate::recognition