#pragma once

#include "Jbonk/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace Jbonk
{
	class OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext(GLFWwindow* windowHandle);

		virtual void Init() override;
		virtual void SwapBuffers() override;
	private:
		GLFWwindow* m_WindowHandle;
	};
}
