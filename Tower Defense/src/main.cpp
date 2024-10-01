#include "app.h"

#include "SDL.h"
#include "SDL_ttf.h"

#include <chrono>

constexpr uint32_t logsCooldown = 500;

int main(int argc, char** arg)
{
	for (int i = 0; i < argc; i++)
	{
		printf("Arg %d: %s\n", i, arg[i]);
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		App::s_Logger.AddLog(SDL_GetError());
	}

	if (TTF_Init() != 0)
	{
		App::s_Logger.AddLog(TTF_GetError());
	}

	App::s_Logger.PrintQueuedLogs();
	App::s_Logger.ClearLogs();

	uint32_t logsTime = SDL_GetTicks() + logsCooldown;

	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();

	App app;

	uint64_t start = SDL_GetPerformanceCounter(), end = SDL_GetPerformanceCounter();
	float elapsed = (end - start) / (float)SDL_GetPerformanceCounter() * 1000;;

	while (app.IsRunning())
	{
		start = SDL_GetPerformanceCounter();

		if (SDL_TICKS_PASSED(SDL_GetTicks(), logsTime))
		{
			App::s_Logger.AddLog("FPS: " + std::to_string(std::llroundf(1.0f / elapsed)));
			App::s_Logger.PrintQueuedLogs();
			App::s_Logger.ClearLogs();
			logsTime = SDL_GetTicks() + logsCooldown;
		}

		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;

		App::s_ElapsedTime = elapsedTime.count();

		app.EventHandler();
		app.Update();
		app.Render();

		end = SDL_GetPerformanceCounter();
		elapsed = (end - start) / (float)SDL_GetPerformanceCounter() * 1000;
	}

	return 0;
}