#pragma once

#include "Jbonk/Core/Core.h"
#include "Jbonk/Core/Log.h"
#include "Jbonk/Scene/Scene.h"
#include "Jbonk/Scene/Entity.h"

namespace Jbonk
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& scene);

		void SetContext(const Ref<Scene>& scene);
		void SetSelectionContext(const Entity& ent) { m_SelectionContext = ent; }
		
		void OnImGuiRender();

		Entity GetSelectedEntity() const { return m_SelectionContext; }
		Entity GetDefferedEntity() const { return m_DefferedEntity; }
		void ResetDeffered() { m_DefferedEntity = Entity(); }
		bool DependencyCheck() const { return m_SetDependency; }
		void CheckTerminate() { m_SetDependency = false; }
	private:
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);
		template<typename T>
		void DisplayAddComponentEntry(const std::string& entryName);
	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;
		Entity m_DefferedEntity = Entity();
		bool m_SetDependency = false;
	};
}

