//==============================================================================
// FaceTurnstile - Print Configuration Utilities
// print_config_utils.cpp
//==============================================================================
#include "../include/print_config_utils.h"
#include <iomanip>

namespace bigiate::config {

    void PrintDatabaseConfig(const DatabaseConfig& db) {
        std::cout << "\n=== Database Configuration ===\n";
        std::cout << "Host: " << db.host << "\n";
        std::cout << "Port: " << db.port << "\n";
        std::cout << "Database name: " << db.name << "\n";
        std::cout << "Schema: " << db.schema << "\n";
        std::cout << "Username: " << db.username << "\n";

        std::cout << "\n--- Pool Settings ---\n";
        std::cout << "Min connections: " << db.pool.min_connections << "\n";
        std::cout << "Max connections: " << db.pool.max_connections << "\n";
        std::cout << "Connection timeout: " << db.pool.connection_timeout_seconds << "s\n";
        std::cout << "Idle timeout: " << db.pool.idle_timeout_seconds << "s\n";

        std::cout << "\n--- Vector Settings ---\n";
        std::cout << "Dimension: " << db.vector.dimension << "\n";
        std::cout << "Similarity threshold: " << db.vector.similarity_threshold << "\n";
        std::cout << "Index type: " << db.vector.index_type << "\n";
    }

    void PrintCameraConfig(const CameraConfig& camera) {
        std::cout << "\n=== Camera Configuration ===\n";
        std::cout << "ID: " << camera.id << "\n";
        std::cout << "Name: " << camera.name << "\n";
        std::cout << "Enabled: " << (camera.enabled ? "true" : "false") << "\n";

        std::cout << "\n--- Connection ---\n";
        std::cout << "Protocol: " << camera.connection.protocol << "\n";
        if (!camera.connection.host.empty()) {
            std::cout << "Host: " << camera.connection.host << "\n";
            std::cout << "Port: " << camera.connection.port << "\n";
            std::cout << "Path: " << camera.connection.path << "\n";
        }
        if (camera.connection.device.has_value()) {
            std::cout << "Device: " << camera.connection.device.value() << "\n";
        }

        std::cout << "\n--- Capture ---\n";
        std::cout << "FPS: " << camera.capture.fps << "\n";
        std::cout << "Resolution: " << camera.capture.width << "x" << camera.capture.height << "\n";
        std::cout << "Rotation: " << camera.capture.rotation << "°\n";
    }

    void PrintBastionConfig(const BastionConfig& bastion) {
        std::cout << "\n=== Bastion Configuration ===\n";
        std::cout << "Enabled: " << (bastion.enabled ? "true" : "false") << "\n";
        std::cout << "Protocol: " << bastion.protocol << "\n";
        std::cout << "Host: " << bastion.host << "\n";
        std::cout << "Port: " << bastion.port << "\n";

        if (bastion.username.has_value()) {
            std::cout << "Username: " << bastion.username.value() << "\n";
        }
        else {
            std::cout << "Username: <not set>\n";
        }

        std::cout << "\n--- Timeouts ---\n";
        std::cout << "Connect timeout: " << bastion.timeout.connect_seconds << "s\n";
        std::cout << "Send timeout: " << bastion.timeout.send_seconds << "s\n";
        std::cout << "Receive timeout: " << bastion.timeout.receive_seconds << "s\n";

        std::cout << "\n--- Retry Settings ---\n";
        std::cout << "Max attempts: " << bastion.retry.max_attempts << "\n";
        std::cout << "Delay: " << bastion.retry.delay_ms << "ms\n";
        std::cout << "Backoff multiplier: " << bastion.retry.backoff_multiplier << "\n";
    }

    void PrintRecognitionConfig(const RecognitionConfig& recognition) {
        std::cout << "\n=== Recognition Configuration ===\n";

        // Detector
        std::cout << "\n--- Detector (SSD) ---\n";
        std::cout << "Type: " << recognition.detector.type << "\n";
        std::cout << "Model path: " << recognition.detector.model_path << "\n";
        if (recognition.detector.config_path.has_value()) {
            std::cout << "Config path: " << recognition.detector.config_path.value() << "\n";
        }
        std::cout << "Backend: " << recognition.detector.backend << "\n";
        std::cout << "Confidence threshold: " << recognition.detector.confidence_threshold << "\n";
        std::cout << "Input size: " << recognition.detector.input_width << "x" << recognition.detector.input_height << "\n";
        std::cout << "Use GPU: " << (recognition.detector.use_gpu ? "true" : "false") << "\n";
        if (recognition.detector.gpu_id.has_value()) {
            std::cout << "GPU ID: " << recognition.detector.gpu_id.value() << "\n";
        }
        std::cout << "Batch size: " << recognition.detector.batch_size << "\n";

        // Extractor
        std::cout << "\n--- Extractor (ArcFace) ---\n";
        std::cout << "Type: " << recognition.extractor.type << "\n";
        std::cout << "Model path: " << recognition.extractor.model_path << "\n";
        std::cout << "Backend: " << recognition.extractor.backend << "\n";
        std::cout << "Embedding size: " << recognition.extractor.embedding_size << "\n";
        std::cout << "Normalize: " << (recognition.extractor.normalize ? "true" : "false") << "\n";
        std::cout << "Use GPU: " << (recognition.extractor.use_gpu ? "true" : "false") << "\n";
        if (recognition.extractor.gpu_id.has_value()) {
            std::cout << "GPU ID: " << recognition.extractor.gpu_id.value() << "\n";
        }
        std::cout << "Batch size: " << recognition.extractor.batch_size << "\n";

        // Matching
        std::cout << "\n--- Matching ---\n";
        std::cout << "Threshold: " << recognition.matching.threshold << "\n";
        std::cout << "Max distance: " << recognition.matching.max_distance << "\n";
        std::cout << "Top K: " << recognition.matching.top_k << "\n";
        std::cout << "Use index: " << (recognition.matching.use_index ? "true" : "false") << "\n";

        // Anti-passback
        std::cout << "\n--- Anti-passback ---\n";
        std::cout << "Enabled: " << (recognition.anti_passback.enabled ? "true" : "false") << "\n";
        std::cout << "Cooldown seconds: " << recognition.anti_passback.cooldown_seconds << "\n";
        std::cout << "Strict mode: " << (recognition.anti_passback.strict_mode ? "true" : "false") << "\n";

        // Performance
        std::cout << "\n--- Performance ---\n";
        std::cout << "Skip frames: " << recognition.performance.skip_frames << "\n";
        std::cout << "Max faces per frame: " << recognition.performance.max_faces_per_frame << "\n";
        std::cout << "Parallel detection: " << (recognition.performance.parallel_detection ? "true" : "false") << "\n";
        std::cout << "Queue size: " << recognition.performance.queue_size << "\n";
    }

    void PrintLoggingConfig(const LoggingConfig& logging) {
        std::cout << "\n=== Logging Configuration ===\n";
        std::cout << "Level: " << logging.level << "\n";
        std::cout << "Format: " << logging.format << "\n";

        std::cout << "\n--- Outputs ---\n";
        for (size_t i = 0; i < logging.outputs.size(); ++i) {
            const auto& out = logging.outputs[i];
            std::cout << "Output " << i + 1 << ":\n";
            std::cout << "  Type: " << out.type << "\n";
            std::cout << "  Enabled: " << (out.enabled ? "true" : "false") << "\n";
            if (out.path.has_value()) {
                std::cout << "  Path: " << out.path.value() << "\n";
            }
            std::cout << "  Rotation: " << out.rotation << "\n";
            std::cout << "  Max size: " << out.max_size_mb << " MB\n";
            std::cout << "  Max files: " << out.max_files << "\n";
        }

        std::cout << "\n--- Metrics ---\n";
        std::cout << "Enabled: " << (logging.metrics.enabled ? "true" : "false") << "\n";
        std::cout << "Interval: " << logging.metrics.interval_seconds << " seconds\n";
    }

    void PrintSecurityConfig(const SecurityConfig& security) {
        std::cout << "\n=== Security Configuration ===\n";
        std::cout << "Encrypt secrets: " << (security.encrypt_secrets ? "true" : "false") << "\n";
        std::cout << "Secrets file: " << security.secrets_file << "\n";

        std::cout << "Config file mode: " << std::oct << "0" << security.config_file_mode << std::dec << "\n";
        std::cout << "Log file mode: " << std::oct << "0" << security.log_file_mode << std::dec << "\n";

        std::cout << "\n--- Allowed Networks ---\n";
        if (security.allowed_networks.empty()) {
            std::cout << "All networks allowed\n";
        }
        else {
            for (const auto& net : security.allowed_networks) {
                std::cout << "  - " << net << "\n";
            }
        }

        std::cout << "\n--- Encryption Settings ---\n";
        std::cout << "Algorithm: " << security.encryption.algorithm << "\n";
        std::cout << "Key derivation: " << security.encryption.key_derivation << "\n";
        std::cout << "Iterations: " << security.encryption.iterations << "\n";
        std::cout << "Salt length: " << security.encryption.salt_length << " bytes\n";
        std::cout << "Prompt at startup: " << (security.encryption.prompt_at_startup ? "true" : "false") << "\n";

        std::cout << "\nFirst run: " << (security.first_run ? "true" : "false") << "\n";
    }

    void PrintServerConfig(const ServerConfig& config) {
        std::cout << "\n" << std::string(60, '=') << "\n";
        std::cout << "SERVER CONFIGURATION\n";
        std::cout << std::string(60, '=') << "\n";

        std::cout << "\nHostname: " << config.hostname << "\n";
        std::cout << "Thread pool size: " << config.thread_pool_size << "\n";
        std::cout << "Frame queue size: " << config.frame_queue_size << "\n";
        std::cout << "Version: " << config.version.toString() << "\n";

        PrintDatabaseConfig(config.database);
        PrintBastionConfig(config.bastion);
        PrintRecognitionConfig(config.recognition);
        PrintLoggingConfig(config.logging);
        PrintSecurityConfig(config.security);

        std::cout << "\n" << std::string(60, '-') << "\n";
        std::cout << "Cameras: " << config.cameras.size() << "\n";
        for (const auto& cam : config.cameras) {
            PrintCameraConfig(cam);
        }
        std::cout << std::string(60, '=') << "\n";
    }

} // namespace bigiate::config