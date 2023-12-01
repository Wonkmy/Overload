/**
* @project: Overload
* @author: Overload Tech.
* @licence: MIT
*/

#pragma once

#include <OvRendering/Entities/Camera.h>
#include <OvRendering/Features/DebugShapeRenderFeature.h>
#include <OvRendering/Core/CompositeRenderer.h>

#include <OvCore/ECS/Actor.h>
#include <OvCore/SceneSystem/SceneManager.h>
#include <OvCore/ECS/Components/CModelRenderer.h>
#include <OvCore/Resources/Material.h>
#include <OvCore/ECS/Components/CAmbientBoxLight.h>
#include <OvCore/ECS/Components/CAmbientSphereLight.h>

#include "OvEditor/Core/Context.h"

namespace OvEditor::Rendering
{
	/**
	* Draw the scene for actor picking
	*/
	class OutlineRenderFeature : public OvRendering::Features::ARenderFeature
	{
	public:
		/**
		* Constructor
		* @param p_renderer
		*/
		OutlineRenderFeature(OvRendering::Core::CompositeRenderer& p_renderer);

		/**
		* Draw an outline around the given actor
		* @param p_actor
		* @param p_color
		* @param p_thickness
		*/
		virtual void DrawOutline(OvCore::ECS::Actor& p_actor, const OvMaths::FVector4& p_color, float p_thickness);

	private:
		void DrawStencilPass(OvCore::ECS::Actor& p_actor);
		void DrawOutlinePass(OvCore::ECS::Actor& p_actor, const OvMaths::FVector4& p_color, float p_thickness);
		
		void DrawActorToStencil(OvCore::ECS::Actor& p_actor);
		void DrawActorOutline(OvCore::ECS::Actor& p_actor);
		void DrawModelToStencil(const OvMaths::FMatrix4& p_worldMatrix, OvRendering::Resources::Model& p_model);
		void DrawModelOutline(const OvMaths::FMatrix4& p_worldMatrix, OvRendering::Resources::Model& p_model);

	private:
		OvCore::Resources::Material m_stencilFillMaterial;
		OvCore::Resources::Material m_outlineMaterial;
	};
}