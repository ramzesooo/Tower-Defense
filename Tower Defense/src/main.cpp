#include "common.h"
#include "app.h"

#include "SDL.h"
#include "SDL_ttf.h"

#include <format>
#include <chrono>

// SDL2 2.30.8
// SDL2_image 2.8.2
// SDL2_ttf 2.22.0

static constexpr uint32_t logsCooldown = 1000;

int main(int argc, char** arg)
{
	IF_DEBUG(
		App::s_Logger.AddLog(std::string_view("DEBUG MODE"));
		for (int i = 0; i < argc; i++)
			App::s_Logger.AddLog(std::format("Arg: {}: {}", i, arg[i]));

		{
			SDL_version SDLVersion{};

			SDL_VERSION(&SDLVersion); // compiled version
			App::s_Logger.AddLog(std::format("\nSDL version:\nCompiled: {}.{}.{}", SDLVersion.major, SDLVersion.minor, SDLVersion.patch));

			SDL_GetVersion(&SDLVersion); // linked version
			App::s_Logger.AddLog(std::format("Linked: {}.{}.{}\n", SDLVersion.major, SDLVersion.minor, SDLVersion.patch));
		}
	)

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0 || TTF_Init() != 0)
	{
		App::s_Logger.AddLog(std::string_view(SDL_GetError()));
	}

	IF_DEBUG(App::s_Logger.PrintQueuedLogs();)
	IF_DEBUG(App::s_Logger.ClearLogs();)

	uint32_t logsTime = SDL_GetTicks() + logsCooldown;

	auto tp1 = std::chrono::system_clock::now();

	App app;

	auto tp2 = std::chrono::system_clock::now();
	std::chrono::duration<float> elapsedTime = tp2 - tp1;
	tp1 = tp2;

	IF_DEBUG(uint32_t frames = 0;)

	while (app.IsRunning())
	{
		tp2 = std::chrono::system_clock::now();
		elapsedTime = tp2 - tp1;
		tp1 = tp2;

		App::s_ElapsedTime = elapsedTime.count();

		app.EventHandler();
		app.Update();
		app.Render();

		IF_DEBUG(
			frames++;
			if (SDL_GetTicks() > logsTime)
			{
				SDL_SetWindowTitle(SDL_RenderGetWindow(App::s_Renderer), std::format("Tower Defense (FPS: {})", frames).c_str());
				logsTime = SDL_GetTicks() + logsCooldown;
				frames = 0;
				App::s_Logger.PrintQueuedLogs();
				App::s_Logger.ClearLogs();
			}
		)
	}

	return 0;
}