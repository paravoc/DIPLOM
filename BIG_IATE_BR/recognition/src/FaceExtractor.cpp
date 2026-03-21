#include "../include/FaceExtractor.h"
#include <iostream>

namespace bigiate::recognition {

    FaceExtractor::FaceExtractor()
        : m_embeddingSize(512)
        , m_normalize(true) {
    }

    std::expected<void, std::string> FaceExtractor::load(
        const std::string& modelPath,
        int embeddingSize,
        bool normalize) {

        try {
            m_net = cv::dnn::readNetFromONNX(modelPath);

            if (m_net.empty()) {
                return std::unexpected("Failed to load extractor model: " + modelPath);
            }

            m_embeddingSize = embeddingSize;
            m_normalize = normalize;

            std::cout << "✅ FaceExtractor loaded: " << modelPath << std::endl;
            return {};

        }
        catch (const std::exception& e) {
            return std::unexpected(std::string("Exception loading extractor: ") + e.what());
        }
    }

    cv::Mat FaceExtractor::preprocessFace(const cv::Mat& face) {
        cv::Mat resized;
        cv::resize(face, resized, cv::Size(112, 112));

        cv::Mat normalized;
        if (m_normalize) {
            resized.convertTo(normalized, CV_32FC3, 1.0 / 255.0);
            // Нормализация для ArcFace
            cv::subtract(normalized, cv::Scalar(0.5, 0.5, 0.5), normalized);
            cv::multiply(normalized, cv::Scalar(2.0, 2.0, 2.0), normalized);
        }
        else {
            resized.convertTo(normalized, CV_32FC3);
        }

        return normalized;
    }

    std::expected<Embedding, std::string> FaceExtractor::extract(const cv::Mat& faceImage) {
        if (m_net.empty()) {
            return std::unexpected("FaceExtractor not loaded!");
        }

        if (faceImage.empty()) {
            return std::unexpected("Empty face image");
        }

        try {
            cv::Mat processed = preprocessFace(faceImage);

            // Создаём блоб
            cv::Mat blob = cv::dnn::blobFromImage(processed);
            m_net.setInput(blob);

            // Forward pass
            cv::Mat output = m_net.forward();

            // Преобразуем в вектор
            Embedding emb;
            emb.vector.reserve(output.total());

            float* data = output.ptr<float>();
            for (size_t i = 0; i < output.total(); ++i) {
                emb.vector.push_back(data[i]);
            }

            emb.quality = 1.0f; // TODO: вычислить качество
            emb.faceImage = faceImage.clone();

            return emb;

        }
        catch (const std::exception& e) {
            return std::unexpected(std::string("Extraction failed: ") + e.what());
        }
    }

} // namespace bigiate::recognition