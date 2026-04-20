#include "../include/FaceRecognizer.h"
#include <iostream>

namespace bigiate::recognition {

    FaceRecognizer::FaceRecognizer()
        : m_detector(std::make_unique<FaceDetector>())
        , m_extractor(std::make_unique<FaceExtractor>()) {
    }

    std::expected<void, std::string> FaceRecognizer::init(
        const config::RecognitionConfig& cfg,
        std::shared_ptr<db::DBQueries> dbQueries) {

        m_config = cfg;
        m_dbQueries = dbQueries;

        // Загружаем детектор
        auto detResult = m_detector->load(
            cfg.detector.model_path,
            cfg.detector.config_path,
            cfg.detector.confidence_threshold,
            cfg.detector.input_width,
            cfg.detector.input_height
        );

        if (!detResult.has_value()) {
            return std::unexpected("Detector init failed: " + detResult.error());
        }

        // Загружаем экстрактор
        auto extResult = m_extractor->load(
            cfg.extractor.model_path,
            cfg.extractor.embedding_size,
            cfg.extractor.normalize
        );

        if (!extResult.has_value()) {
            return std::unexpected("Extractor init failed: " + extResult.error());
        }

        m_initialized = true;
        std::cout << "✅ FaceRecognizer initialized" << std::endl;
        return {};
    }

    std::vector<RecognitionResult> FaceRecognizer::recognize(
        const cv::Mat& frame,
        int cameraId,
        float threshold) {

        std::vector<RecognitionResult> results;

        if (!m_initialized) {
            std::cerr << "FaceRecognizer not initialized!" << std::endl;
            return results;
        }

        // 1. Детекция лиц
        auto detections = m_detector->detect(frame, cameraId);

        if (detections.empty()) {
            return results;
        }

        // 2. Для каждого лица извлекаем эмбеддинг и ищем в БД
        for (const auto& detection : detections) {
            RecognitionResult result;
            result.cameraId = cameraId;
            result.detection = detection;

            // Извлекаем эмбеддинг
            auto embResult = m_extractor->extract(detection.faceROI);

            if (!embResult.has_value()) {
                result.accessGranted = false;
                result.reason = "Extraction failed: " + embResult.error();
                results.push_back(result);
                continue;
            }

            result.embedding = embResult.value();

            // Ищем в БД
            if (m_dbQueries) {
                auto matches = m_dbQueries->findPersonByEmbedding(
                    result.embedding.vector,
                    threshold,
                    m_config.matching.top_k
                );

                if (matches.has_value() && !matches->empty()) {
                    if (matches.has_value() && !matches->empty()) {
                        std::cout << "🔍 [DEBUG] Найдено совпадений: " << matches->size()
                            << ", лучшее: " << matches->front().similarity * 100 << "%"
                            << " (имя: " << matches->front().fullName << ")"
                            << std::endl;
                        result.bestMatchSimilarity = matches->front().similarity;
                    }
                    else {
                        std::cout << "🔍 [DEBUG] Совпадений не найдено" << std::endl;
                        result.bestMatchSimilarity = 0.0f;
                    }
                    const auto& dbMatch = matches->front();

                    Match match;
                    match.personId = dbMatch.personId;
                    match.fullName = dbMatch.fullName;
                    match.personType = dbMatch.personType;
                    match.similarity = dbMatch.similarity;
                    match.encodingId = dbMatch.encodingId;
                    match.verificationRequired = !dbMatch.verificationCode.empty();
                    match.verificationCode = dbMatch.verificationCode;

                    result.match = match;

                    // Проверяем права доступа
                    result.accessGranted = checkAccess(match, cameraId);
                    result.reason = result.accessGranted ? "Access granted" : "Access denied";
                }
                else {
                    result.accessGranted = false;
                    result.reason = "Person not recognized";
                }
            }
            else {
                result.accessGranted = false;
                result.reason = "Database not available";
            }

            results.push_back(result);
        }

        return results;
    }

    bool FaceRecognizer::checkAccess(const Match& match, int cameraId) {
        // TODO: Реализовать проверку прав доступа
        // - Время доступа
        // - Дни недели
        // - Зоны доступа
        // - Anti-passback
        return match.similarity >= m_config.matching.threshold;
    }

} // namespace bigiate::recognition