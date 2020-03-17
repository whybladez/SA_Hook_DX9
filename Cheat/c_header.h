#pragma once
#include <Windows.h>
#include <process.h>

#include "sdk/game/common.h"
#include "sdk/game/CPools.h"
#include "sdk/game/CMenuManager.h"

// ����� ��������� � �������� �������
#define COLOR_ERROR				0xff9a3519		// ������� �����
#define COLOR_INFO				0xffd6cb00		// Ƹ���� �����
#define COLOR_SUCCESS			0xff5cb300		// ������ �����
#define COLOR_DEBUG				0xff1f80c1		// ����� �����

// �������� �������� �������
#define NAME_SVDIR				"cheat"		// ��� �������� ����������
#define NAME_LOG				"log.txt"	// ��� ����� �����

// ������ ���� � �������� �������
#define PATH_SVDIR				"./" NAME_SVDIR
#define PATH_LOG				PATH_SVDIR "/" NAME_LOG


// ����� �������
namespace util {

	// ���������� �������
	static HMODULE h_module;

	// ��������� ������
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

	// ��������� ������
	static inline void free_resource(
		HGLOBAL resource
	) {
		FreeResource(resource);
	}

	// ������ �� �������
	static inline bool is_key_down(
		uint8_t key_id
	) {
		return GetKeyState(key_id) & 0x80;
	}

	// �������� ������ ����
	static inline bool is_active() {
		return *(uint8_t*)(0x8D621C) && !FrontEndMenuManager.m_bMenuActive;
	}

	// �������� ������ ������
	static inline int get_width_screen() {
		return GetSystemMetrics(SM_CXSCREEN);
	}

	// �������� ������ ������
	static inline int get_height_screen() {
		return GetSystemMetrics(SM_CYSCREEN);
	}

}