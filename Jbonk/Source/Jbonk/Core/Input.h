#pragma once

#include "Jbonk/Core/Core.h"

namespace Jbonk
{
	class Input
	{
	public:

		static bool IsKeyPressed(int keycode);
		static bool IsMouseButtonPressed(int button);
		static bool IsMouseButtonReleased(int button);
		static std::pair<float, float> GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
	
	};
}