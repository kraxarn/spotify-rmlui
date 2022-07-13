#include "RmlUi/Core.h"
#include "RmlUi_Backend.h"

#ifndef NDEBUG
#include <RmlUi/Debugger/Debugger.h>
#endif

#include <iostream>

auto main(int /*argc*/, char **/*argv*/) -> int
{
	constexpr int window_width = 1280;
	constexpr int window_height = 720;

	const auto initialized = Backend::Initialize("spotify-rmlui",
		window_width, window_height, true);

	if (!initialized)
	{
		std::cerr << "Failed to initialize" << std::endl;
		return -1;
	}

	Rml::SetSystemInterface(Backend::GetSystemInterface());
	Rml::SetRenderInterface(Backend::GetRenderInterface());
	Rml::Initialise();

	auto *context = Rml::CreateContext("main",
		Rml::Vector2i(window_width, window_height));

	// Fonts
	Rml::LoadFontFace("res/font/NotoSans-Regular.ttf", false);
	Rml::LoadFontFace("res/font/NotoEmoji-Regular.ttf", true);

#ifndef NDEBUG
	Rml::Debugger::Initialise(context);
#endif

	// Load document
	auto *document = context->LoadDocument("res/rml/index.rml");
	document->Show();

	// Application loop
	while (Backend::ProcessEvents(context))
	{
		context->Update();
		Backend::BeginFrame();
		context->Render();
		Backend::PresentFrame();
	}

	Rml::Shutdown();
	Backend::Shutdown();
	return 0;
}
