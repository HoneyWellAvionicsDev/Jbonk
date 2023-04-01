#pragma once

#include "Jbonk/Core/Core.h"
#include "Jbonk/Core/Application.h"
#include "Jbonk/Core/utest.h"

#define RUN_UNIT_TEST 0
#if RUN_UNIT_TEST
	UTEST_MAIN();
#else

#ifdef HZ_PLATFORM_WINDOWS //WIN MAIN

extern Jbonk::Application* Jbonk::CreateApplication(ApplicationCommandLineArgs args);      

UTEST_STATE();

int main(int argc, char** argv)
{
	Jbonk::Log::Init();

	PROFILE_BEGIN_SESSION("Startup", "JbonkProfile-Startup.json");
	auto app = Jbonk::CreateApplication({ argc, argv });              
	PROFILE_END_SESSION();

	PROFILE_BEGIN_SESSION("Runtime", "JbonkProfile-Runtime.json");
	app->Run();
	PROFILE_END_SESSION();

	PROFILE_BEGIN_SESSION("Shutdown", "JbonkProfile-Shutdown.json");
	delete app;
	PROFILE_END_SESSION();

	return 0;
}
#elif HZ_PLATFORM_LINUX 

int main(int argc, char** argv)
{
	Jbonk::Log::Init();

	auto app = Jbonk::CreateApplication({ argc, argv });
	app->Run();
	delete app;

	return 0;
}

#endif //END WIN MAIN
#endif
