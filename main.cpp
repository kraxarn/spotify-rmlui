#include "RmlUi/Core.h"

#ifndef NDEBUG
#include <RmlUi/Debugger/Debugger.h>
#endif

#include <iostream>

#include "shell/sdl/RenderInterface.hpp"
#include "shell/sdl/SystemInterface.hpp"

void sdl_init(const Rml::Vector2i &window_size, SDL_Renderer **sdl_renderer,
	SDL_Window **sdl_window, SDL_GLContext *sdl_gl_context)
{
#ifdef RMLUI_PLATFORM_WIN32
	AllocConsole();
#endif

	SDL_Init(SDL_INIT_VIDEO);
	*sdl_window = SDL_CreateWindow("spotify-qt-rmlui",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		window_size.x, window_size.y,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	*sdl_gl_context = SDL_GL_CreateContext(*sdl_window);

	auto ogl_idx = -1;
	auto n_rd = SDL_GetNumRenderDrivers();

	for (int i = 0; i < n_rd; i++)
	{
		SDL_RendererInfo info;
		if (SDL_GetRenderDriverInfo(i, &info) == 0)
		{
			if (strcmp(info.name, "opengl") == 0)
			{
				ogl_idx = i;
			}
		}
	}

	*sdl_renderer = SDL_CreateRenderer(*sdl_window, ogl_idx,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	auto err = glewInit();
	if (err != GLEW_OK)
	{
		std::cerr << "GLEW Error: " << glewGetErrorString(err) << std::endl;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	glMatrixMode(GL_PROJECTION | GL_MODELVIEW);
	glLoadIdentity();
	glOrtho(0, window_size.x, window_size.y, 0, 0, 1);
}

auto main(int /*argc*/, char **/*argv*/) -> int
{
	constexpr int window_width = 1280;
	constexpr int window_height = 720;
	Rml::Vector2i window_size(window_width, window_height);

	SDL_Renderer *sdl_renderer = nullptr;
	SDL_Window *sdl_window = nullptr;
	SDL_GLContext sdl_gl_context{};
	sdl_init(window_size, &sdl_renderer, &sdl_window, &sdl_gl_context);

	Sdl::RenderInterface render_interface(sdl_renderer, sdl_window);
	Sdl::SystemInterface system_interface;

	Rml::SetRenderInterface(&render_interface);
	Rml::SetSystemInterface(&system_interface);
	Rml::Initialise();

	auto *context = Rml::CreateContext("main", window_size);

	// Fonts
	Rml::LoadFontFace("res/font/NotoSans-Regular.ttf", false);
	Rml::LoadFontFace("res/font/NotoEmoji-Regular.ttf", true);

	// Data bindings
	struct ApplicationData
	{
		bool show_text = true;
		Rml::String animal = "dog";
	} my_data;

	auto data_model = context->CreateDataModel("animals");
	if (data_model)
	{
		data_model.Bind("show_text", &my_data.show_text);
		data_model.Bind("animal", &my_data.animal);
	}

#ifndef NDEBUG
	Rml::Debugger::Initialise(context);
#endif

	// Load document
	auto *document = context->LoadDocument("res/rml/index.rml");
	document->Show();

	// Test replacing some text
	auto *element = document->GetElementById("world");
	element->SetInnerRML(reinterpret_cast<const char *>(u8"🌍"));
	element->SetProperty("font-size", "1.5em");

	// Application loop
	auto running = true;
	while (running)
	{
		SDL_Event event;

		SDL_SetRenderDrawColor(sdl_renderer, 0x0, 0x0, 0x0, 0xff);
		SDL_RenderClear(sdl_renderer);

		context->Render();
		SDL_RenderPresent(sdl_renderer);

		while (SDL_PollEvent(&event) != 0)
		{
			switch (event.type)
			{
				case SDL_QUIT:
					running = false;
					break;

				case SDL_MOUSEMOTION:
					context->ProcessMouseMove(event.motion.x, event.motion.y,
						Sdl::SystemInterface::GetKeyModifiers());
					break;

				case SDL_MOUSEBUTTONDOWN:
					context->ProcessMouseButtonDown(Sdl::SystemInterface
						::TranslateMouseButton(event.button.button),
						Sdl::SystemInterface::GetKeyModifiers());
					break;

				case SDL_MOUSEBUTTONUP:
					context->ProcessMouseButtonUp(Sdl::SystemInterface
						::TranslateMouseButton(event.button.button),
						Sdl::SystemInterface::GetKeyModifiers());
					break;

				case SDL_MOUSEWHEEL:
					context->ProcessMouseWheel(float(event.wheel.y),
						Sdl::SystemInterface::GetKeyModifiers());
					break;

				case SDL_KEYDOWN:
				{
#ifndef NDEBUG
					// Intercept F8 key stroke to toggle RmlUi visual debugger tool
					if (event.key.keysym.sym == SDLK_F8)
					{
						Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());
						break;
					}
#endif
					context->ProcessKeyDown(Sdl::SystemInterface
						::TranslateKey(static_cast<SDL_KeyCode>(event.key.keysym.sym)),
						Sdl::SystemInterface::GetKeyModifiers());
					break;
				}

				default:
					break;
			}
		}

		context->Update();
	}

	Rml::Shutdown();
	SDL_DestroyRenderer(sdl_renderer);
	SDL_GL_DeleteContext(sdl_gl_context);
	SDL_DestroyWindow(sdl_window);
	SDL_Quit();

	return 0;
}
