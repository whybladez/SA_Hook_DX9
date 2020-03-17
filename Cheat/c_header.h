#pragma once
#include <Windows.h>
#include <process.h>

#include "sdk/game/common.h"
#include "sdk/game/CPools.h"
#include "sdk/game/CMenuManager.h"

// Цвета сообщений в цифровом формате
#define COLOR_ERROR				0xff9a3519		// Красный шрифт
#define COLOR_INFO				0xffd6cb00		// Жёлтый шрифт
#define COLOR_SUCCESS			0xff5cb300		// Зелёный шрифт
#define COLOR_DEBUG				0xff1f80c1		// Синий шрифт

// Названия объектов плагина
#define NAME_SVDIR				"cheat"		// Имя основной директории
#define NAME_LOG				"log.txt"	// Имя файла логов

// Полные пути к объектам плагина
#define PATH_SVDIR				"./" NAME_SVDIR
#define PATH_LOG				PATH_SVDIR "/" NAME_LOG


// Общие функции
namespace util {

	// Дескриптор плагина
	static HMODULE h_module;

	// Загрузить ресурс
	static bool load_resource(
		const uint32_t rId,		// [in]
		const char* rType,		// [in]
		HGLOBAL* pResource,		// [out]
		void** ppAddress,		// [out]
		uint32_t* pSize			// [out]
	) {
		HRSRC res;
		if ((res = FindResource(h_module, MAKEINTRESOURCE(rId), rType)) &&
			(*pResource = LoadResource(h_module, res)) &&
			(*ppAddress = LockResource(*pResource)) &&
			(*pSize = SizeofResource(h_module, res))
			) return true;
		return false;
	}

	// Выгрузить ресурс
	static inline void free_resource(
		HGLOBAL resource
	) {
		FreeResource(resource);
	}

	// Нажата ли клавиша
	static inline bool is_key_down(
		uint8_t key_id
	) {
		return GetKeyState(key_id) & 0x80;
	}

	// Получить статус игры
	static inline bool is_active() {
		return *(uint8_t*)(0x8D621C) && !FrontEndMenuManager.m_bMenuActive;
	}

	// Получить ширину экрана
	static inline int get_width_screen() {
		return GetSystemMetrics(SM_CXSCREEN);
	}

	// Получить высоту экрана
	static inline int get_height_screen() {
		return GetSystemMetrics(SM_CYSCREEN);
	}

}