#include "app.h"

#include "SDL.h"
#include "SDL_ttf.h"

int main(int argc, char** arg)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		App::s_Logger->AddLog(SDL_GetError());
	}

	if (TTF_Init() != 0)
	{
		App::s_Logger->AddLog(TTF_GetError());
	}

	App::s_Logger->PrintQueuedLogs();
	App::s_Logger->ClearLogs();

	constexpr uint32_t logsCooldown = 500;
	uint32_t logsTime = SDL_GetTicks() + logsCooldown;

	App app;

	while (app.IsRunning())
	{
		if (SDL_TICKS_PASSED(SDL_GetTicks(), logsTime))
		{
			App::s_Logger->PrintQueuedLogs();
			App::s_Logger->ClearLogs();
			logsTime = SDL_GetTicks() + logsCooldown;
		}

		app.EventHandler();
		app.Update();
		app.Render();
	}

	return 0;
}