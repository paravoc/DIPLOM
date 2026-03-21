#pragma once

#include <vector>
#include <string>
#include <optional>
#include <opencv2/opencv.hpp>

namespace bigiate::recognition {

    // Результат детекции одного лица
    struct Detection {
        cv::Rect bbox;              // прямоугольник лица
        float confidence;           // уверенность детекции
        int cameraId;               // ID камеры
        cv::Mat faceROI;            // вырезанное лицо (для отладки)
    };

    // Результат экстракции эмбеддинга
    struct Embedding {
        std::vector<float> vector;  // 512-мерный вектор
        float quality;              // качество (0-1)
        cv::Mat faceImage;          // исходное изображение лица
    };

    // Результат сравнения с БД
    struct Match {
        int personId;               // ID человека
        std::string fullName;       // ФИО
        std::string personType;     // student, teacher, staff, guest
        float similarity;           // схожесть (0-1)
        int encodingId;             // ID эмбеддинга из БД
        bool verificationRequired;  // требуется ли дополнительная верификация
        std::string verificationCode; // код для верификации (для близняшек)
    };

    // Полный результат распознавания
    struct RecognitionResult {
        int cameraId;               // ID камеры
        Detection detection;        // результат детекции
        Embedding embedding;        // результат экстракции
        std::optional<Match> match; // результат поиска (если найден)
        bool accessGranted;         // доступ разрешён
        std::string reason;         // причина решения
    };

} // namespace bigiate::recognition