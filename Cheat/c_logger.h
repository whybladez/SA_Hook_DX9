#pragma once

#include <iostream>

#define LogInfo(message, ...) logger::log_info(message, __VA_ARGS__)
#define LogError(module_name, description, ...) logger::log_error(module_name, __func__, description, __VA_ARGS__)
#define LogDebug(message, ...) logger::log_debug(message, __VA_ARGS__)
#define WriteLog(message, ...) logger::log_wo_msg(message, __VA_ARGS__)

namespace logger {

	static FILE* log_file = nullptr;

	// ������� ����������
	template<class... ARGS>
	static inline void log_info(
		const char* message,
		const ARGS... args
	) {

		char buffer[512];

		snprintf(buffer, sizeof(buffer), "[info] : %s\n", message);

		if (log_file) {
			fprintf(log_file, buffer, args...);
			fflush(log_file);
		}

	}

	// ������� ������
	template<class... ARGS>
	static inline void log_error(
		const char* module_name,
		const char* method_name,
		const char* description,
		const ARGS... args
	) {

		char buffer[512];

		snprintf(
			buffer, sizeof(buffer),
			"[error] : [%s] : [%s] : %s\n",
			module_name,
			method_name,
			description
		);

		if (log_file) {
			fprintf(log_file, buffer, args...);
			fflush(log_file);
		}

	}

	// ������� ���������� ���������
	template<class... ARGS>
	static inline void log_debug(
		const char* message,
		const ARGS... args
	) {

		char buffer[512];

		snprintf(buffer, sizeof(buffer), "[debug] : %s\n", message);

		if (log_file) {
			fprintf(log_file, buffer, args...);
			fflush(log_file);
		}

	}

	// �������� ��������� � ���� (��� ������ �� �������)
	template<class... ARGS>
	static inline void log_wo_msg(
		const char* message,
		const ARGS... args
	) {

		if (log_file) {
			char buffer[512];
			snprintf(buffer, sizeof(buffer), "[debug] : %s\n", message);
			fprintf(log_file, buffer, args...);
			fflush(log_file);
		}

	}

	// ���������������� ������
	static inline bool init() {

		if (!log_file) return log_file = fopen(PATH_LOG, "wt");
		else return false;

	}

	// ��������� ������
	static inline void free() {

		if (log_file) {
			fclose(log_file);
			log_file = nullptr;
		}

	}

}