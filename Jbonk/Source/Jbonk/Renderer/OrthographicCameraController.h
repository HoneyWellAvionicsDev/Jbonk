#pragma once

#include "Jbonk/Renderer/OrthographicCamera.h"
#include "Jbonk/Core/TimeStep.h"
#include "Jbonk/Events/ApplicationEvent.h"
#include "Jbonk/Events/MouseEvent.h"

namespace Jbonk
{
	struct OrthographicCameraBounds
	{
		float Left, Right;
		float Bottom, Top;

		float GetWidth() { return Right - Left; }
		float GetHeight() { return Top - Bottom; }
	};

	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRatio, bool rotation = false);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);
		void OnResize(float width, float height);

		OrthographicCamera& GetCamera() { return m_Camera; }
		const OrthographicCamera& GetCamera() const { return m_Camera; }
		const OrthographicCameraBounds& GetBounds() const { return m_Bounds; }
		float GetZoomLevel() const { return m_ZoomLevel; }
		void SetZoomLevel(float level) { m_ZoomLevel = level; CalculateView(); }
	private:
		void CalculateView();
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		float m_AspectRatio;
		float m_ZoomLevel = 1.f;
		OrthographicCameraBounds m_Bounds;  //--|order matters
		OrthographicCamera m_Camera;        //__|here

		glm::vec3 m_CameraPosition = { 0.f, 0.f, 0.f };
		float m_CameraTranslationSpeed = 5.5f;

		bool m_Rotation;
		float m_CameraRotationAngle = 0;  //in degress, anti clockwise direction
		float m_CameraRotationSpeed = 45.f;
	};
}

