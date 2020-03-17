#include <thread>
#include <atomic>
#include <forward_list>

#include "c_header.h"
#include "c_addresses.h"
#include "c_render.h"
#include "c_samp_core.h"

namespace core {
	static volatile bool afk_status = false;		// Статус афк
	static volatile bool plugin_status = false;		// Статус загрузки плагина
	static volatile bool post_status = false;
	static UINT_PTR timer_post;

	
	// Обработчик цикла отрисовки
	void handler_render(IDirect3DDevice9* device,CONST RECT* source_rect,HWND hwnd) 
	{

		if (util::is_active()) 
		{
			if (plugin_status && util::is_active() && !afk_status)
			{
				
			}
		}
	}

	// Обработчик инициализации устройства отрисовки
	void handler_init(IDirect3DDevice9* device,D3DPRESENT_PARAMETERS* parameters) 
	{

	}

	// Обработчик сброса устройства отрисовки
	void handler_reset() 
	{

	}

	void CALLBACK handler_post_timer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
	{
		post_status = false;
		KillTimer(hwnd, idEvent);
	}

	static void CALLBACK handler_exit() 
	{
		handler_reset();
		logger::free();
		plugin_status = false;
	}

	// Главный поток логики
	static DWORD WINAPI mainThread(
		LPVOID parameters
	) {

		while (plugin_status) 
		{
			// Проверка режима афк
			if (!util::is_active() && !afk_status) {
				if (post_status) KillTimer(render::get_hwnd(), timer_post);
				else post_status = true; afk_status = true;
			}
			else if (util::is_active() && afk_status) {
				timer_post = SetTimer(render::get_hwnd(), NULL, 25, handler_post_timer);
				afk_status = false;
			}

			if (util::is_active()) {

			}

		}

		return 0;
	}

	static void start() {

		plugin_status = true;
		CreateThread(0, 0, mainThread, 0, 0, 0);

	}
}

static volatile uint8_t status_samp = 2ui8;

static DWORD WINAPI SampInit(LPVOID samp_init) 
{
	status_samp = samp::init(core::handler_exit);
	return 0;
}

static DWORD WINAPI initThread(LPVOID parameters)
{
	if (!CreateDirectory(PATH_SVDIR, NULL)) 
	{
		FILE* output = fopen("log.txt", "wt");
		fprintf(output, "Could not create log directory. (code:%u)\n", GetLastError());
		fclose(output);
	}

	std::cout << "Create Directory" << std::endl;

	if (!logger::init()) {
		//std::cout << "Logger not init" << std::endl;
	}

	addresses::init();

	std::cout << "Addresses init" << std::endl;

	if (!render::core::init(core::handler_render, core::handler_init, core::handler_reset)) {
		LogError("main", "Could not initialize render::core::init");
		std::cout << "Could not initialize render::core::init" << std::endl;
		logger::free(); return 0;
	}

	if (!CreateThread(0, 0, SampInit, 0, 0, 0)) {
		LogError("main", "Could not initialize SAMPInit");
		std::cout << "Could not initialize SAMPinit" << std::endl;
		render::core::free(); logger::free(); return 0;
	}

	std::cout << "Before STATUS_SAMP" << std::endl;

	while (status_samp == 2) Sleep(10);

	if (!status_samp) {
		LogError("main", "could not initialize samp");
		std::cout << "could not initialize samp" << std::endl;
		render::core::free(); logger::free(); return 0;
	}

	samp::add_message_to_chat(0xFFA9C4E4, "Cheat loaded.");
	core::start();
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,DWORD dwReasonForCall,LPVOID lpReserved) 
{
	switch (dwReasonForCall) 
	{
		case DLL_PROCESS_ATTACH:
		{
			AllocConsole();
			freopen("CONOUT$", "w", stdout);
			return (BOOL)(CreateThread(0, 0, initThread, 0, 0, 0));
		}
	} return TRUE;
}