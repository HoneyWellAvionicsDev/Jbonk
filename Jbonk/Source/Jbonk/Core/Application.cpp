#include "jbpch.h"
#include "Application.h"

#include "Jbonk/Core/Log.h"
#include "Jbonk/Core/Input.h"
#include "Jbonk/Renderer/Renderer.h"
#include "Jbonk/Utils/PlatfromUtils.h"

#include <GLFW/glfw3.h>

namespace Jbonk
{
	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& specification)
	: m_Specification(specification)
	{
		PROFILE_FUNCTION();

		JB_CORE_ASSERT(!s_Instance, "Application already exsists!");
		s_Instance = this;

		m_Window = Window::Create(WindowProps(m_Specification.Name));
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
		m_Window->SetVSync(true);

		Renderer::Init();

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{
		PROFILE_FUNCTION();
		//Renderer::Shutdown();
	}

	void Application::PushLayer(Layer* layer)
	{
		PROFILE_FUNCTION();
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		PROFILE_FUNCTION();
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void Application::OnEvent(Event& event)
	{
		PROFILE_FUNCTION();

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));                 //if dispatcher recieves windowCloseEvent then we call OnWindowClose
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)                      //collects all events from the layer stack from TOP to DOWN
		{
			if (event.Handled)
				break;
			(*it)->OnEvent(event);
		}
	}

	void Application::Run()
	{
		PROFILE_FUNCTION();

		while (m_Running)
		{
			Timer timer;
			PROFILE_SCOPE("RunLoop");

			float time = Time::GetTime();                                                   
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			if (!m_Minimized)
			{
				PROFILE_SCOPE("LayerStack OnUpdate");
				for (Layer* layer : m_LayerStack)                                               //updates all the layers in the layerstack from BOTTOM to TOP
					layer->OnUpdate(timestep);
			}

			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();
			m_ImGuiLayer->End();
			m_Window->OnUpdate();
			m_LastTime = timer.Elapsed();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false; //terminates the loop and exits appilcation
		return true;
	}
	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		PROFILE_FUNCTION();

		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}

		m_Minimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}
}