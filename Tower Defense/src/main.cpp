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
#ifdef DEBUG
	App::s_Logger.AddLog("DEBUG MODE");
	for (int i = 0; i < argc; i++)
	{
		App::s_Logger.AddLog("Arg: " + std::to_string(i) + ": " + arg[i]);
	}

	{
		SDL_version SDLVersion{};

		SDL_VERSION(&SDLVersion); // compiled version
		App::s_Logger.AddLog(std::format("\nSDL version:\nCompiled: {}.{}.{}", SDLVersion.major, SDLVersion.minor, SDLVersion.patch));

		SDL_GetVersion(&SDLVersion); // linked version
		App::s_Logger.AddLog(std::format("Linked: {}.{}.{}\n", SDLVersion.major, SDLVersion.minor, SDLVersion.patch));
	}
#endif

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0 || TTF_Init() != 0)
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

#ifdef DEBUG
	uint32_t frames = 0;
#endif

	while (app.IsRunning())
	{
		tp2 = std::chrono::system_clock::now();
		elapsedTime = tp2 - tp1;
		tp1 = tp2;

		App::s_ElapsedTime = elapsedTime.count();

		app.EventHandler();
		app.Update();
		app.Render();

#ifdef DEBUG
		frames++;
		if (SDL_GetTicks() > logsTime)
		{
			SDL_SetWindowTitle(SDL_RenderGetWindow(App::s_Renderer), std::string("Tower Defense (FPS: " + std::to_string(frames) + ")").c_str());
			logsTime = SDL_GetTicks() + logsCooldown;
			frames = 0;
			App::s_Logger.PrintQueuedLogs();
			App::s_Logger.ClearLogs();
		}
#endif
	}

	return 0;
}