#include "jbpch.h"
#include "ContentViewPanel.h"

#include <imgui/imgui.h>

namespace Jbonk
{

	extern const std::filesystem::path g_AssetPath = "assets";

	ContentViewPanel::ContentViewPanel()
		: m_CurrentDirectory(g_AssetPath)
	{
		m_DirectoryIcon = Texture2D::Upload("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_FileIcon = Texture2D::Upload("Resources/Icons/ContentBrowser/FileIcon.png");
	}

	void ContentViewPanel::OnImGuiRender(float padding, float thumbnailSize)
	{
		ImGui::Begin("Content Browser"); 

		if ((m_CurrentDirectory != std::filesystem::path(g_AssetPath)))
		{
			if (ImGui::Button("<-"))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
		}

		
		float cellSize = thumbnailSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;

		ImGui::Columns(columnCount, 0, false);

		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory)) //TODO: add a filter to prevent certain files from being displayed
		{
			const auto& path = directoryEntry.path();
			std::string filenameString = path.filename().string();

			Ref<Texture2D> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::PushID(filenameString.c_str()); //we push an ID to each image so that identical images do not have dupe IDs
			ImGui::ImageButton((ImTextureID)icon->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });

			if (ImGui::BeginDragDropSource())
			{
				auto relativePath = std::filesystem::relative(path, g_AssetPath);
				const wchar_t* itemPath = relativePath.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t), ImGuiCond_Once); //our payload gets copied into imgui so be aware of what our payload is
				ImGui::EndDragDropSource();
			}

			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (directoryEntry.is_directory())
					m_CurrentDirectory /= path.filename();
			}

			ImGui::TextWrapped(filenameString.c_str());
			ImGui::NextColumn();
			ImGui::PopID();
		}

		ImGui::Columns(1);

		//TODO: status bar
		ImGui::End();
	}
}