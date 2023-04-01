#include "jbpch.h"
#include "OpenGLRendererAPI.h"
#include <glad/glad.h>

namespace Jbonk
{
	void OpenGLMessageCallback(
		unsigned source,
		unsigned type,
		unsigned id,
		unsigned severity,
		int length,
		const char* message,
		const void* userParam)
	{
		switch (severity)
		{
			case GL_DEBUG_SEVERITY_HIGH:		 JB_CORE_FATAL(message); return;
			case GL_DEBUG_SEVERITY_MEDIUM:		 JB_CORE_ERROR(message); return;
			case GL_DEBUG_SEVERITY_LOW:			 JB_CORE_WARN(message);	 return;
			case GL_DEBUG_SEVERITY_NOTIFICATION: JB_CORE_TRACE(message); return;
		}

		JB_CORE_ASSERT(false, "Unknown severity level!");
	}

	void OpenGLRendererAPI::Init()
	{
#ifdef HZ_DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLMessageCallback, nullptr);

		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LINE_SMOOTH);
	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& VertexArray, uint32_t indexCount)
	{
		VertexArray->Bind();
		uint32_t count = indexCount ? indexCount : VertexArray->GetIndexBuffer()->GetCount();           //maybe switch this the other way around
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr); //elements are our indices (draw mode, how many to draw, type, pointer to elements)
	}

	void OpenGLRendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
	{
		vertexArray->Bind();
		glDrawArrays(GL_LINES, 0, vertexCount);
	}

	void OpenGLRendererAPI::SetLineWidth(float width)
	{
		glLineWidth(width);
	}
}
