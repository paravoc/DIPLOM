#include "../include/FaceDetector.h"
#include <iostream>

namespace bigiate::recognition {

    FaceDetector::FaceDetector()
        : m_confidenceThreshold(0.5f)
        , m_inputWidth(300)
        , m_inputHeight(300) {
    }

    std::expected<void, std::string> FaceDetector::load(
        const std::string& modelPath,
        const std::optional<std::string>& configPath,
        float confidenceThreshold,
        int inputWidth,
        int inputHeight) {

        try {
            if (configPath.has_value()) {
                // Caffe модель (SSD)
                m_net = cv::dnn::readNetFromCaffe(configPath.value(), modelPath);
            }
            else {
                // ONNX модель (YOLO, ArcFace детектор)
                m_net = cv::dnn::readNetFromONNX(modelPath);
            }

            if (m_net.empty()) {
                return std::unexpected("Failed to load detector model: " + modelPath);
            }

            m_confidenceThreshold = confidenceThreshold;
            m_inputWidth = inputWidth;
            m_inputHeight = inputHeight;

            std::cout << "✅ FaceDetector loaded: " << modelPath << std::endl;
            return {};

        }
        catch (const std::exception& e) {
            return std::unexpected(std::string("Exception loading detector: ") + e.what());
        }
    }

    std::vector<Detection> FaceDetector::detect(const cv::Mat& frame, int cameraId) {
        std::vector<Detection> results;

        if (m_net.empty()) {
            std::cerr << "FaceDetector not loaded!" << std::endl;
            return results;
        }

        // Подготовка блоба
        cv::Mat blob = cv::dnn::blobFromImage(
            frame, 1.0, cv::Size(m_inputWidth, m_inputHeight),
            cv::Scalar(104.0, 177.0, 123.0), false, false);

        m_net.setInput(blob);
        cv::Mat output = m_net.forward();

        cv::Mat detectionMat(output.size[2], output.size[3], CV_32F, output.ptr<float>());

        int frameHeight = frame.rows;
        int frameWidth = frame.cols;

        for (int i = 0; i < detectionMat.rows; ++i) {
            float confidence = detectionMat.at<float>(i, 2);

            if (confidence > m_confidenceThreshold) {
                int x1 = static_cast<int>(detectionMat.at<float>(i, 3) * frameWidth);
                int y1 = static_cast<int>(detectionMat.at<float>(i, 4) * frameHeight);
                int x2 = static_cast<int>(detectionMat.at<float>(i, 5) * frameWidth);
                int y2 = static_cast<int>(detectionMat.at<float>(i, 6) * frameHeight);

                // Ограничиваем координаты
                x1 = std::max(0, x1);
                y1 = std::max(0, y1);
                x2 = std::min(frameWidth, x2);
                y2 = std::min(frameHeight, y2);

                if (x1 < x2 && y1 < y2) {
                    Detection det;
                    det.bbox = cv::Rect(x1, y1, x2 - x1, y2 - y1);
                    det.confidence = confidence;
                    det.cameraId = cameraId;
                    det.faceROI = frame(det.bbox).clone();
                    results.push_back(det);
                }
            }
        }

        return results;
    }

} // namespace bigiate::recognition