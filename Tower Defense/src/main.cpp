#include "app.h"

#include "SDL.h"
#include "SDL_ttf.h"

//#ifdef _DEBUG
//#include <iostream>
//#endif

#include <chrono>

// SDL2 2.30.8
// SDL2_image 2.8.2
// SDL2_ttf 2.22.0

constexpr uint32_t logsCooldown = 1000;
uint32_t frames = 1;

int main(int argc, char** arg)
{
#ifdef DEBUG
	App::s_Logger.AddLog("DEBUG MODE (" + std::to_string(DEBUG) + ")");
	for (int i = 0; i < argc; i++)
	{
		App::s_Logger.AddLog("Arg: " + std::to_string(i) + ": " + arg[i]);
	}
#endif

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0 || TTF_Init() != 0)
	{
		App::s_Logger.AddLog(SDL_GetError());
	}

#ifdef DEBUG
	App::s_Logger.PrintQueuedLogs();
	App::s_Logger.ClearLogs();
#endif

	uint32_t logsTime = SDL_GetTicks() + logsCooldown;

	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();
	//std::chrono::system_clock::time_point

	App app;

	tp2 = std::chrono::system_clock::now();
	std::chrono::duration<float> elapsedTime = tp2 - tp1;
	tp1 = tp2;

	while (app.IsRunning())
	{
		++frames;

#ifdef DEBUG
		if (SDL_TICKS_PASSED(SDL_GetTicks(), logsTime))
		{
			SDL_SetWindowTitle(SDL_RenderGetWindow(App::s_Renderer), std::string("Tower Defense (FPS: " + std::to_string(frames) + ")").c_str());
			App::s_Logger.PrintQueuedLogs();
			App::s_Logger.ClearLogs();
			logsTime = SDL_GetTicks() + logsCooldown;
			frames = 0;
		}
#else
		if (SDL_TICKS_PASSED(SDL_GetTicks(), logsTime))
		{
			SDL_SetWindowTitle(SDL_RenderGetWindow(App::s_Renderer), std::string("Tower Defense (FPS: " + std::to_string(frames) + ")").c_str());
			logsTime = SDL_GetTicks() + logsCooldown;
			frames = 0;
		}
#endif

		tp2 = std::chrono::system_clock::now();
		elapsedTime = tp2 - tp1;
		tp1 = tp2;

		App::s_ElapsedTime = elapsedTime.count();

		app.EventHandler();
		app.Update();
		app.Render();
	}

	return 0;
}