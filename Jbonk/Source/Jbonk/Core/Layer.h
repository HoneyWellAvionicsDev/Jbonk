#pragma once

#include "Jbonk/Core/Core.h"
#include "Jbonk/Events/Event.h"
#include "Jbonk/Core/TimeStep.h"

namespace Jbonk
{
	//basically to create a layer, we will derive from this class and override its members
	class Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer() = default;

		virtual void OnAttach() = 0;
		virtual void OnDetach() = 0;
		virtual void OnUpdate(Timestep ts) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event) {}

		inline const std::string& GetName() const { return m_DebugName; }	
	protected:
		std::string m_DebugName;
	};

}
