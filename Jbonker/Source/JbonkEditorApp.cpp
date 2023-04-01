#include <Jbonk.h>
#include <Jbonk/Core/EntryPoint.h>

#include "JbonkEditorLayer.h"

namespace Jbonk
{
	class JbonkEditor : public Application
	{
	public:
		JbonkEditor(const ApplicationSpecification& specification)
			: Application(specification)
		{
			PushLayer(new EditorLayer());
		}
	
		~JbonkEditor()
		{
	
		}
	};
	
	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		ApplicationSpecification spec;
		spec.Name = "Jbonk Physics";
		spec.CommandLineArgs = args;

		return new JbonkEditor(spec);
	}
}