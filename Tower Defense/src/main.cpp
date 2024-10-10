#include "app.h"

#include "SDL.h"
#include "SDL_ttf.h"

#include <chrono>

constexpr uint32_t logsCooldown = 1000;
uint16_t frames = 1;

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
	//std::chrono::system_clock::time_point

	App app;

	tp2 = std::chrono::system_clock::now();
	std::chrono::duration<float> elapsedTime = tp2 - tp1;
	tp1 = tp2;

	while (app.IsRunning())
	{
		++frames;
		if (SDL_TICKS_PASSED(SDL_GetTicks(), logsTime))
		{
			SDL_SetWindowTitle(SDL_RenderGetWindow(App::s_Renderer), std::string("Tower Defense (FPS: " + std::to_string(frames) + ")").c_str());
			App::s_Logger.PrintQueuedLogs();
			App::s_Logger.ClearLogs();
			logsTime = SDL_GetTicks() + logsCooldown;
			frames = 0;
		}

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