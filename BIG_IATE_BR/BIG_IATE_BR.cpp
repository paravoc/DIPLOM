#include <iostream>
#include "configs/include/loader.h"
#include "configs/include/print_config_utils.h"

namespace config = bigiate::config;

int main(int argc, char* argv[]) {
    std::string config_file = "C:\\Users\\smidr\\source\\repos\\BIG_IATE_BR\\x64\\Debug\\test_config.yaml";
    if (argc > 1) {
        config_file = argv[1];
    }

    auto result = config::LoadConfig(config_file);

    if (!result.has_value()) {
        std::cerr << "❌ " << result.error() << "\n";
        return 1;
    }

    std::cout << "✅ Config loaded successfully!\n";

    // Одной строкой выводим всё!
    config::PrintServerConfig(result->config);

    return 0;
}