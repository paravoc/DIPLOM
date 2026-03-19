//==============================================================================
// FaceTurnstile - Print Configuration Utilities
// print_config_utils.h
//==============================================================================
#pragma once

#include "types.h"
#include <iostream>

namespace bigiate::config {

	// Основные функции печати
	void PrintDatabaseConfig(const DatabaseConfig& db);
	void PrintCameraConfig(const CameraConfig& camera);
	void PrintBastionConfig(const BastionConfig& bastion);
	void PrintRecognitionConfig(const RecognitionConfig& recognition);
	void PrintLoggingConfig(const LoggingConfig& logging);
	void PrintSecurityConfig(const SecurityConfig& security);

	// Функция для печати всего конфига
	void PrintServerConfig(const ServerConfig& config);

} // namespace bigiate::config