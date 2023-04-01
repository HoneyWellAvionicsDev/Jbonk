#pragma once

#include "Jbonk/Core/Layer.h"

#include "Jbonk/Events/KeyEvent.h"
#include "Jbonk/Events/MouseEvent.h"
#include "Jbonk/Events/ApplicationEvent.h"

namespace Jbonk
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& e) override;
		virtual void OnImGuiRender() override;

		void Begin();
		void End();

		void BlockEvents(bool block) { m_BlockEvents = block; }
		void SetDarkThemeColors();
	private:

		bool m_BlockEvents = true;
	};
}

