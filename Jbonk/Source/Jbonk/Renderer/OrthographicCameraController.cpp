#include "jbpch.h"
#include "Jbonk/Renderer/OrthographicCameraController.h"
#include "Jbonk/Core/Input.h"
#include "Jbonk/Core/KeyCodes.h"
#include "Jbonk/Core/MouseButtonCodes.h"

namespace Jbonk
{
	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
		: m_AspectRatio(aspectRatio), m_Bounds({ -m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel }), m_Camera(m_Bounds.Left, m_Bounds.Right, m_Bounds.Bottom, m_Bounds.Top), m_Rotation(rotation)
	{
	}

	void OrthographicCameraController::OnUpdate(Timestep ts)
	{
		PROFILE_FUNCTION();

		if (Input::IsKeyPressed(HZ_KEY_W))
		{
			m_CameraPosition.x += -sin(glm::radians(m_CameraRotationAngle)) * m_CameraTranslationSpeed * ts;
			m_CameraPosition.y += cos(glm::radians(m_CameraRotationAngle)) * m_CameraTranslationSpeed * ts;
		}
		else if (Input::IsKeyPressed(HZ_KEY_S))
		{
			m_CameraPosition.x -= -sin(glm::radians(m_CameraRotationAngle)) * m_CameraTranslationSpeed * ts;
			m_CameraPosition.y -= cos(glm::radians(m_CameraRotationAngle)) * m_CameraTranslationSpeed * ts;
		}
		if (Input::IsKeyPressed(HZ_KEY_A))
		{
			m_CameraPosition.x -= cos(glm::radians(m_CameraRotationAngle)) * m_CameraTranslationSpeed * ts;
			m_CameraPosition.y -= sin(glm::radians(m_CameraRotationAngle)) * m_CameraTranslationSpeed * ts;
		}
		if (Input::IsKeyPressed(HZ_KEY_D))
		{
			m_CameraPosition.x += cos(glm::radians(m_CameraRotationAngle)) * m_CameraTranslationSpeed * ts;
			m_CameraPosition.y += sin(glm::radians(m_CameraRotationAngle)) * m_CameraTranslationSpeed * ts;
		}
		
		if (m_Rotation)
		{
			if (Input::IsKeyPressed(HZ_KEY_Q))
				m_CameraRotationAngle -= m_CameraRotationSpeed * ts;
			if (Input::IsKeyPressed(HZ_KEY_E))
				m_CameraRotationAngle += m_CameraRotationSpeed * ts;

			if (m_CameraRotationAngle > 180.f)
				m_CameraRotationAngle -= 360.f;
			else if (m_CameraRotationAngle <= -180.f)
				m_CameraRotationAngle += 360.f;


			m_Camera.SetRotation(m_CameraRotationAngle);
		}

		m_Camera.SetPosition(m_CameraPosition);
		m_CameraTranslationSpeed = m_ZoomLevel;
	}

	void OrthographicCameraController::OnEvent(Event& e)
	{
		PROFILE_FUNCTION();
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(OrthographicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OrthographicCameraController::OnWindowResize));
	}

	void OrthographicCameraController::OnResize(float width, float height)
	{
		m_AspectRatio = width / height;
		CalculateView();
	}

	void OrthographicCameraController::CalculateView()
	{
		m_Bounds = { -m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel };
		m_Camera.SetProjection(m_Bounds.Left, m_Bounds.Right, m_Bounds.Bottom, m_Bounds.Top);
	}

	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		PROFILE_FUNCTION();
		m_ZoomLevel -= e.GetYOffset() * 0.8f;
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
		CalculateView();
		return false;
	}

	bool OrthographicCameraController::OnWindowResize(WindowResizeEvent& e)
	{
		PROFILE_FUNCTION();
		m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
		CalculateView();
		return false;
	}
}
