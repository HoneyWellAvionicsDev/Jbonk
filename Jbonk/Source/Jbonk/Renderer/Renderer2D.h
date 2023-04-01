#pragma once

#include "OrthographicCamera.h"
#include "Camera.h"
#include "Texture.h"
#include "SubTexture2D.h"
#include "EditorCamera.h"

#include "Jbonk/Scene/Components.h"

namespace Jbonk
{
	class Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const EditorCamera& camera); 
		static void BeginScene(const OrthographicCamera& camera); //remove this
		static void EndScene();
		static void Flush();

		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);
		static void DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec4& tint = glm::vec4(1.f), float tileFactor = 1.f, int entityID = -1);

		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& tint = glm::vec4(1.f), float tileFactor = 1.f);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& tint = glm::vec4(1.f), float tileFactor = 1.f);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& texture, const glm::vec4& tint = glm::vec4(1.f), float tileFactor = 1.f);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<SubTexture2D>& texture, const glm::vec4& tint = glm::vec4(1.f), float tileFactor = 1.f);

		//rotation accepts angle in radians by default
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotationInRadians, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotationInRadians, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotationInRadians, const Ref<Texture2D>& texture, const glm::vec4& tint = glm::vec4(1.f), float tileFactor = 1.f);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotationInRadians, const Ref<Texture2D>& texture, const glm::vec4& tint = glm::vec4(1.f), float tileFactor = 1.f);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotationInRadians, const Ref<SubTexture2D>& texture, const glm::vec4& tint = glm::vec4(1.f), float tileFactor = 1.f);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotationInRadians, const Ref<SubTexture2D>& texture, const glm::vec4& tint = glm::vec4(1.f), float tileFactor = 1.f);

		static void DrawCircle(const glm::mat4& transform, const glm::vec4& color, float thickness = 1.0f, float fade = 0.005f, int entityID = -1);

		static void DrawLine(const glm::vec3& p0, glm::vec3& p1, const glm::vec4& color, int entityID = -1);

		static void DrawRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, int entityID = -1);
		static void DrawRect(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);

		static void DrawSprite(const glm::mat4& transform, SpriteRendererComponent& src, int entityID);

		static float GetLineWidth();
		static void SetLineWidth(float width);

		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;

			uint32_t GetTotalVertexCount() { return QuadCount * 4; }
			uint32_t GetTotalIndexCount() { return QuadCount * 6; }
		};
		static void ResetStats();
		static Statistics GetStats();
	private:
		static void StartBatch();
		static void StartNewBatch();
	};
}

