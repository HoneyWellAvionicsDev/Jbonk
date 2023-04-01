#pragma once


#include "Jbonk/Core/Core.h"
#include "Jbonk/Core/LayerStack.h"
#include "Jbonk/Events/Event.h"
#include "Jbonk/Events/ApplicationEvent.h"
#include "Jbonk/imGui/imGuiLayer.h"
#include "Jbonk/Core/TimeStep.h"
#include "Jbonk/Core/Timer.h"

#include "Window.h"

namespace Jbonk
{
	struct ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			JB_CORE_ASSERT(index < Count, "Out of bounds");
			return Args[index];
		}
	};

	struct ApplicationSpecification
	{
		std::string Name = " Application";
		std::string WorkingDirectory;
		ApplicationCommandLineArgs CommandLineArgs;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& specification);
		virtual ~Application();


		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		void CloseWindow() { m_Running = false; }
		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }
		Window& GetWindow() { return *m_Window; }
		float GetLastFrameTime() const { return m_LastTime; }

		void Run();

		static Application& Get() { return *s_Instance; }

		const ApplicationSpecification& GetSpecification() const { return m_Specification; }
	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
	private:
		bool m_Running = true;
		bool m_Minimized = false;

		ApplicationSpecification m_Specification;
		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.f;
		float m_LastTime = 0.f;

		static Application* s_Instance;
	};

	Application* CreateApplication(const ApplicationCommandLineArgs args); //to be defined in client
}


