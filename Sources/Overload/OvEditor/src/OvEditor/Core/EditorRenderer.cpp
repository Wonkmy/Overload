/**
* @project: Overload
* @author: Overload Tech.
* @restrictions: This software may not be resold, redistributed or otherwise conveyed to a third party.
*/

#include <OvCore/ECS/Components/CCamera.h>
#include <OvCore/ECS/Components/CPhysicalBox.h>
#include <OvCore/ECS/Components/CPhysicalSphere.h>
#include <OvCore/ECS/Components/CPhysicalCapsule.h>
#include <OvCore/ECS/Components/CMaterialRenderer.h>
#include <OvCore/ECS/Components/CPointLight.h>
#include <OvCore/ECS/Components/CDirectionalLight.h>
#include <OvCore/ECS/Components/CSpotLight.h>

#include <OvDebug/Utils/Assertion.h>

#include "OvEditor/Core/EditorRenderer.h"
#include "OvEditor/Core/EditorResources.h"
#include "OvEditor/Panels/AView.h"
#include "OvEditor/Core/GizmoBehaviour.h"

#include "OvEditor/Core/EditorActions.h"

using namespace OvMaths;
using namespace OvRendering::Resources;
using namespace OvCore::Resources;

OvEditor::Core::EditorRenderer::EditorRenderer(Context& p_context) : m_context(p_context)
{
	m_context.renderer->SetCapability(OvRendering::Settings::ERenderingCapability::STENCIL_TEST, true);
	m_context.renderer->SetStencilOperations(OvRendering::Settings::EOperation::KEEP, OvRendering::Settings::EOperation::KEEP, OvRendering::Settings::EOperation::REPLACE);
	m_context.renderer->SetStencilAlgorithm(OvRendering::Settings::EComparaisonAlgorithm::ALWAYS, 1, 0xFF);

	InitMaterials();

	m_context.renderer->RegisterModelMatrixSender([this](const OvMaths::FMatrix4 & p_modelMatrix)
	{
		m_context.engineUBO->SetSubData(OvMaths::FMatrix4::Transpose(p_modelMatrix), 0);
	});

	m_context.renderer->RegisterUserMatrixSender([this](const OvMaths::FMatrix4 & p_userMatrix)
	{
		m_context.engineUBO->SetSubData
		(
			p_userMatrix, 

			// UBO layout offset
			sizeof(OvMaths::FMatrix4) +
			sizeof(OvMaths::FMatrix4) +
			sizeof(OvMaths::FMatrix4) +
			sizeof(OvMaths::FVector3) +
			sizeof(float)
		);
	});
}

void OvEditor::Core::EditorRenderer::InitMaterials()
{
	/* Default Material */
	m_defaultMaterial.SetShader(m_context.shaderManager[":Shaders\\Standard.glsl"]);
	m_defaultMaterial.Set("u_Diffuse", FVector4(1.f, 1.f, 1.f, 1.f));
	m_defaultMaterial.Set("u_Shininess", 100.0f);
	m_defaultMaterial.Set<OvRendering::Resources::Texture*>("u_DiffuseMap", nullptr);

	/* Empty Material */
	m_emptyMaterial.SetShader(m_context.shaderManager[":Shaders\\Unlit.glsl"]);
	m_emptyMaterial.Set("u_Diffuse", FVector4(1.f, 0.f, 1.f, 1.0f));
	m_emptyMaterial.Set<OvRendering::Resources::Texture*>("u_DiffuseMap", nullptr);

	/* Grid Material */
	m_gridMaterial.SetShader(m_context.editorResources->GetShader("Grid"));
	m_gridMaterial.SetBlendable(true);
	m_gridMaterial.SetBackfaceCulling(false);
	m_gridMaterial.SetDepthTest(false);

	/* Camera Material */
	m_cameraMaterial.SetShader(m_context.shaderManager[":Shaders\\Lambert.glsl"]);
	m_cameraMaterial.Set("u_Diffuse", FVector4(0.0f, 0.3f, 0.7f, 1.0f));
	m_cameraMaterial.Set<OvRendering::Resources::Texture*>("u_DiffuseMap", nullptr);

	/* Stencil Fill Material */
	m_stencilFillMaterial.SetShader(m_context.shaderManager[":Shaders\\Unlit.glsl"]);
	m_stencilFillMaterial.SetBackfaceCulling(true);
	m_stencilFillMaterial.SetDepthTest(false);
	m_stencilFillMaterial.SetColorWriting(false);
	m_stencilFillMaterial.Set<OvRendering::Resources::Texture*>("u_DiffuseMap", nullptr);

	/* Texture Material */
	m_textureMaterial.SetShader(m_context.shaderManager[":Shaders\\Unlit.glsl"]);
	m_textureMaterial.Set("u_Diffuse", FVector4(1.f, 1.f, 1.f, 1.f));
	m_textureMaterial.SetBackfaceCulling(false);
	m_textureMaterial.SetBlendable(true);
	m_textureMaterial.Set<OvRendering::Resources::Texture*>("u_DiffuseMap", nullptr);

	/* Outline Material */
	m_outlineMaterial.SetShader(m_context.shaderManager[":Shaders\\Unlit.glsl"]);
	m_outlineMaterial.Set("u_Diffuse", FVector4(1.f, 1.f, 0.f, 1.0f));
	m_outlineMaterial.Set<OvRendering::Resources::Texture*>("u_DiffuseMap", nullptr);
	m_outlineMaterial.SetDepthTest(false);

	/* Gizmo Arrow Material */
	m_gizmoArrowMaterial.SetShader(m_context.editorResources->GetShader("Gizmo"));
	m_gizmoArrowMaterial.SetGPUInstances(3);
	m_gizmoArrowMaterial.Set("u_IsBall", false);
	m_gizmoArrowMaterial.Set("u_IsPickable", false);

	/* Gizmo Ball Material */
	m_gizmoBallMaterial.SetShader(m_context.editorResources->GetShader("Gizmo"));
	m_gizmoBallMaterial.Set("u_IsBall", true);
	m_gizmoBallMaterial.Set("u_IsPickable", false);

	/* Gizmo Pickable Material */
	m_gizmoPickingMaterial.SetShader(m_context.editorResources->GetShader("Gizmo"));
	m_gizmoPickingMaterial.SetGPUInstances(3);
	m_gizmoPickingMaterial.Set("u_IsBall", false);
	m_gizmoPickingMaterial.Set("u_IsPickable", true);

	/* Picking Material */
	m_actorPickingMaterial.SetShader(m_context.shaderManager[":Shaders\\Unlit.glsl"]);
	m_actorPickingMaterial.Set("u_Diffuse", FVector4(1.f, 1.f, 1.f, 1.0f));
	m_actorPickingMaterial.Set<OvRendering::Resources::Texture*>("u_DiffuseMap", nullptr);
	m_actorPickingMaterial.SetFrontfaceCulling(false);
	m_actorPickingMaterial.SetBackfaceCulling(false);
}

void OvEditor::Core::EditorRenderer::PreparePickingMaterial(OvCore::ECS::Actor& p_actor)
{
	uint32_t actorID = static_cast<uint32_t>(p_actor.GetID());

	auto bytes = reinterpret_cast<uint8_t*>(&actorID);
	auto color = FVector4{ bytes[0] / 255.0f, bytes[1] / 255.0f, bytes[2] / 255.0f, 1.0f };

	m_actorPickingMaterial.Set("u_Diffuse", color);
}

OvMaths::FMatrix4 OvEditor::Core::EditorRenderer::CalculateCameraModelMatrix(OvCore::ECS::Actor& p_actor)
{
	auto translation = FMatrix4::Translation(p_actor.transform.GetWorldPosition());
	auto rotation = FQuaternion::ToMatrix4(p_actor.transform.GetWorldRotation());
	auto scale = FMatrix4::Scaling({ 0.4f, 0.4f, 0.4f });

	return translation * rotation * scale;
}

void OvEditor::Core::EditorRenderer::RenderScene(const OvMaths::FVector3& p_cameraPosition)
{
	/* Render the actors */
	m_context.lightSSBO->Bind(0);
	m_context.renderer->RenderScene(*m_context.sceneManager.GetCurrentScene(), p_cameraPosition, &m_emptyMaterial);
	m_context.lightSSBO->Unbind();
}

void OvEditor::Core::EditorRenderer::RenderSceneForActorPicking()
{
	auto& scene = *m_context.sceneManager.GetCurrentScene();

	/* Render models */
	for (auto modelRenderer : scene.GetFastAccessComponents().modelRenderers)
	{
		auto& actor = modelRenderer->owner;

		if (actor.IsActive())
		{
			if (auto model = modelRenderer->GetModel())
			{

				if (auto materialRenderer = modelRenderer->owner.GetComponent<OvCore::ECS::Components::CMaterialRenderer>())
				{
					const OvCore::ECS::Components::CMaterialRenderer::MaterialList& materials = materialRenderer->GetMaterials();
					auto modelMatrix = actor.transform.GetWorldMatrix();
					PreparePickingMaterial(actor);

					for (auto mesh : model->GetMeshes())
					{
						OvCore::Resources::Material* material = nullptr;

						if (mesh->GetMaterialIndex() < MAX_MATERIAL_COUNT)
						{
							material = materials.at(mesh->GetMaterialIndex());
							if (!material || !material->GetShader())
								material = &m_emptyMaterial;
						}

						if (material)
						{
							m_actorPickingMaterial.SetBackfaceCulling(material->HasBackfaceCulling());
							m_actorPickingMaterial.SetFrontfaceCulling(material->HasFrontfaceCulling());
							m_actorPickingMaterial.SetColorWriting(material->HasColorWriting());
							m_actorPickingMaterial.SetDepthTest(material->HasDepthTest());
							m_actorPickingMaterial.SetDepthWriting(material->HasDepthWriting());

							m_context.renderer->DrawMesh(*mesh, m_actorPickingMaterial, &modelMatrix);
						}
					}
				}
			}
		}
	}

	/* Render cameras */
	for (auto camera : m_context.sceneManager.GetCurrentScene()->GetFastAccessComponents().cameras)
	{
		auto& actor = camera->owner;

		if (actor.IsActive())
		{
			PreparePickingMaterial(actor);
			auto& model = *m_context.editorResources->GetModel("Camera");
			auto modelMatrix = CalculateCameraModelMatrix(actor);

			m_context.renderer->DrawModelWithSingleMaterial(model, m_actorPickingMaterial, &modelMatrix);
		}
	}
}

void OvEditor::Core::EditorRenderer::RenderUI()
{
	m_context.uiManager->Render();
}

void OvEditor::Core::EditorRenderer::RenderCameras()
{
	for (auto camera : m_context.sceneManager.GetCurrentScene()->GetFastAccessComponents().cameras)
	{
		auto& actor = camera->owner;

		if (actor.IsActive())
		{
			auto& model = *m_context.editorResources->GetModel("Camera");
			auto modelMatrix = CalculateCameraModelMatrix(actor);
			
			m_context.renderer->DrawModelWithSingleMaterial(model, m_cameraMaterial, &modelMatrix);
		}
	}
}

void OvEditor::Core::EditorRenderer::RenderGizmo(const OvMaths::FVector3& p_position, const OvMaths::FQuaternion& p_rotation, OvEditor::Core::EGizmoOperation p_operation, bool p_pickable)
{
	using namespace OvMaths;

	FMatrix4 model = FMatrix4::Translation(p_position) * FQuaternion::ToMatrix4(FQuaternion::Normalize(p_rotation));

	if (!p_pickable)
	{
		FMatrix4 sphereModel = model * OvMaths::FMatrix4::Scaling({ 0.1f, 0.1f, 0.1f });
		m_context.renderer->DrawModelWithSingleMaterial(*m_context.editorResources->GetModel("Sphere"), m_gizmoBallMaterial, &sphereModel);
	}

	OvRendering::Resources::Model* arrowModel = nullptr;

	switch (p_operation)
	{
	case OvEditor::Core::EGizmoOperation::TRANSLATION:
		arrowModel = m_context.editorResources->GetModel("Arrow_Translate");
		break;
	case OvEditor::Core::EGizmoOperation::ROTATION:
		arrowModel = m_context.editorResources->GetModel("Arrow_Rotate");
		break;
	case OvEditor::Core::EGizmoOperation::SCALE:
		arrowModel = m_context.editorResources->GetModel("Arrow_Scale");
		break;
	}

	if (arrowModel)
	{
		m_context.renderer->DrawModelWithSingleMaterial(*arrowModel, p_pickable ? m_gizmoPickingMaterial : m_gizmoArrowMaterial, &model);
	}
}

void OvEditor::Core::EditorRenderer::RenderModelToStencil(const OvMaths::FMatrix4& p_worldMatrix, OvRendering::Resources::Model& p_model)
{
	m_context.renderer->SetStencilMask(0xFF);
	m_context.renderer->DrawModelWithSingleMaterial(p_model, m_stencilFillMaterial, &p_worldMatrix);
	m_context.renderer->SetStencilMask(0x00);
}

void OvEditor::Core::EditorRenderer::RenderModelOutline(const OvMaths::FMatrix4& p_worldMatrix, OvRendering::Resources::Model& p_model)
{
	m_context.renderer->SetStencilAlgorithm(OvRendering::Settings::EComparaisonAlgorithm::NOTEQUAL, 1, 0xFF);
	m_context.renderer->SetRasterizationMode(OvRendering::Settings::ERasterizationMode::LINE);
	m_context.renderer->SetRasterizationLinesWidth(5.f);
	m_context.renderer->DrawModelWithSingleMaterial(p_model, m_outlineMaterial, &p_worldMatrix);
	m_context.renderer->SetRasterizationLinesWidth(1.f);
	m_context.renderer->SetRasterizationMode(OvRendering::Settings::ERasterizationMode::FILL);
	m_context.renderer->SetStencilAlgorithm(OvRendering::Settings::EComparaisonAlgorithm::ALWAYS, 1, 0xFF);
}

void OvEditor::Core::EditorRenderer::RenderActorAsSelected(OvCore::ECS::Actor& p_actor, bool p_toStencil)
{
	if (p_actor.IsActive())
	{
		/* Render static mesh outline */
		if (auto modelRenderer = p_actor.GetComponent<OvCore::ECS::Components::CModelRenderer>(); modelRenderer && modelRenderer->GetModel())
		{
			if (p_toStencil)
				RenderModelToStencil(p_actor.transform.GetWorldMatrix(), *modelRenderer->GetModel());
			else
				RenderModelOutline(p_actor.transform.GetWorldMatrix(), *modelRenderer->GetModel());
		}

		/* Render camera component outline */
		if (auto cameraComponent = p_actor.GetComponent<OvCore::ECS::Components::CCamera>(); cameraComponent)
		{
			auto model = FMatrix4::Translation(p_actor.transform.GetWorldPosition()) * FQuaternion::ToMatrix4(FQuaternion::Normalize(p_actor.transform.GetWorldRotation())) * FMatrix4::Scaling({ 0.4f, 0.4f, 0.4f });

			if (p_toStencil)
				RenderModelToStencil(model, *m_context.editorResources->GetModel("Camera"));
			else
				RenderModelOutline(model, *m_context.editorResources->GetModel("Camera"));
		}

		/* Render the actor collider */
		if (p_actor.GetComponent<OvCore::ECS::Components::CPhysicalObject>() && !p_toStencil)
		{
			RenderActorCollider(p_actor);
		}

		/* Render the actor ambient light */
		if (auto ambientBoxComp = p_actor.GetComponent<OvCore::ECS::Components::CAmbientBoxLight>(); ambientBoxComp && !p_toStencil)
		{
			RenderAmbientBoxVolume(*ambientBoxComp);
		}

		if (auto ambientSphereComp = p_actor.GetComponent<OvCore::ECS::Components::CAmbientSphereLight>(); ambientSphereComp && !p_toStencil)
		{
			RenderAmbientSphereVolume(*ambientSphereComp);
		}

		for (auto& child : p_actor.GetChildren())
			RenderActorAsSelected(*child, p_toStencil);
	}
}

void OvEditor::Core::EditorRenderer::RenderActorCollider(OvCore::ECS::Actor & p_actor)
{
	using namespace OvCore::ECS::Components;
	using namespace OvPhysics::Entities;

	bool depthTestBackup = m_context.renderer->GetCapability(OvRendering::Settings::ERenderingCapability::DEPTH_TEST);
	m_context.renderer->SetCapability(OvRendering::Settings::ERenderingCapability::DEPTH_TEST, false);

	/* Draw the box collider if any */
	if (auto boxColliderComponent = p_actor.GetComponent<OvCore::ECS::Components::CPhysicalBox>(); boxColliderComponent)
	{
		OvMaths::FQuaternion rotation = p_actor.transform.GetWorldRotation();
		OvMaths::FVector3 center = p_actor.transform.GetWorldPosition();
		OvMaths::FVector3 colliderSize = boxColliderComponent->GetSize();
		OvMaths::FVector3 actorScale = p_actor.transform.GetWorldScale();
		OvMaths::FVector3 halfSize = { colliderSize.x * actorScale.x, colliderSize.y * actorScale.y, colliderSize.z * actorScale.z };
		OvMaths::FVector3 size = halfSize * 2.f;
		
		m_context.shapeDrawer->DrawLine(center + rotation * OvMaths::FVector3{ -halfSize.x, -halfSize.y, -halfSize.z }, center	+ rotation * OvMaths::FVector3{ -halfSize.x, -halfSize.y, +halfSize.z }, OvMaths::FVector3{ 0.f, 1.f, 0.f }, 1.f);
		m_context.shapeDrawer->DrawLine(center + rotation * OvMaths::FVector3{ -halfSize.x, halfSize.y, -halfSize.z }, center	+ rotation * OvMaths::FVector3{ -halfSize.x, +halfSize.y, +halfSize.z }, OvMaths::FVector3{ 0.f, 1.f, 0.f }, 1.f);
		m_context.shapeDrawer->DrawLine(center + rotation * OvMaths::FVector3{ -halfSize.x, -halfSize.y, -halfSize.z }, center	+ rotation * OvMaths::FVector3{ -halfSize.x, +halfSize.y, -halfSize.z }, OvMaths::FVector3{ 0.f, 1.f, 0.f }, 1.f);
		m_context.shapeDrawer->DrawLine(center + rotation * OvMaths::FVector3{ -halfSize.x, -halfSize.y, +halfSize.z }, center	+ rotation * OvMaths::FVector3{ -halfSize.x, +halfSize.y, +halfSize.z }, OvMaths::FVector3{ 0.f, 1.f, 0.f }, 1.f);
		m_context.shapeDrawer->DrawLine(center + rotation * OvMaths::FVector3{ +halfSize.x, -halfSize.y, -halfSize.z }, center	+ rotation * OvMaths::FVector3{ +halfSize.x, -halfSize.y, +halfSize.z }, OvMaths::FVector3{ 0.f, 1.f, 0.f }, 1.f);
		m_context.shapeDrawer->DrawLine(center + rotation * OvMaths::FVector3{ +halfSize.x, halfSize.y, -halfSize.z }, center	+ rotation * OvMaths::FVector3{ +halfSize.x, +halfSize.y, +halfSize.z }, OvMaths::FVector3{ 0.f, 1.f, 0.f }, 1.f);
		m_context.shapeDrawer->DrawLine(center + rotation * OvMaths::FVector3{ +halfSize.x, -halfSize.y, -halfSize.z }, center	+ rotation * OvMaths::FVector3{ +halfSize.x, +halfSize.y, -halfSize.z }, OvMaths::FVector3{ 0.f, 1.f, 0.f }, 1.f);
		m_context.shapeDrawer->DrawLine(center + rotation * OvMaths::FVector3{ +halfSize.x, -halfSize.y, +halfSize.z }, center	+ rotation * OvMaths::FVector3{ +halfSize.x, +halfSize.y, +halfSize.z }, OvMaths::FVector3{ 0.f, 1.f, 0.f }, 1.f);
		m_context.shapeDrawer->DrawLine(center + rotation * OvMaths::FVector3{ -halfSize.x, -halfSize.y, -halfSize.z }, center	+ rotation * OvMaths::FVector3{ +halfSize.x, -halfSize.y, -halfSize.z }, OvMaths::FVector3{ 0.f, 1.f, 0.f }, 1.f);
		m_context.shapeDrawer->DrawLine(center + rotation * OvMaths::FVector3{ -halfSize.x, +halfSize.y, -halfSize.z }, center	+ rotation * OvMaths::FVector3{ +halfSize.x, +halfSize.y, -halfSize.z }, OvMaths::FVector3{ 0.f, 1.f, 0.f }, 1.f);
		m_context.shapeDrawer->DrawLine(center + rotation * OvMaths::FVector3{ -halfSize.x, -halfSize.y, +halfSize.z }, center	+ rotation * OvMaths::FVector3{ +halfSize.x, -halfSize.y, +halfSize.z }, OvMaths::FVector3{ 0.f, 1.f, 0.f }, 1.f);
		m_context.shapeDrawer->DrawLine(center + rotation * OvMaths::FVector3{ -halfSize.x, +halfSize.y, +halfSize.z }, center	+ rotation * OvMaths::FVector3{ +halfSize.x, +halfSize.y, +halfSize.z }, OvMaths::FVector3{ 0.f, 1.f, 0.f }, 1.f);
	}

	/* Draw the sphere collider if any */
	if (auto sphereColliderComponent = p_actor.GetComponent<OvCore::ECS::Components::CPhysicalSphere>(); sphereColliderComponent)
	{
		FVector3 actorScale = p_actor.transform.GetWorldScale();
		OvMaths::FQuaternion rotation = p_actor.transform.GetWorldRotation();
		OvMaths::FVector3 center = p_actor.transform.GetWorldPosition();
		float radius = sphereColliderComponent->GetRadius() * std::max(std::max(std::max(actorScale.x, actorScale.y), actorScale.z), 0.0f);

		for (float i = 0; i <= 360.0f; i += 10.0f)
		{
			m_context.shapeDrawer->DrawLine(center + rotation * (OvMaths::FVector3{ cos(i * (3.14f / 180.0f)), sin(i * (3.14f / 180.0f)), 0.f } * radius), center + rotation * (OvMaths::FVector3{ cos((i + 10.0f) * (3.14f / 180.0f)), sin((i + 10.0f) * (3.14f / 180.0f)), 0.f } * radius), OvMaths::FVector3{ 0.f, 1.f, 0.f }, 1.f);
			m_context.shapeDrawer->DrawLine(center + rotation * (OvMaths::FVector3{ 0.f, sin(i * (3.14f / 180.0f)), cos(i * (3.14f / 180.0f)) } * radius), center + rotation * (OvMaths::FVector3{ 0.f, sin((i + 10.0f) * (3.14f / 180.0f)), cos((i + 10.0f) * (3.14f / 180.0f)) } * radius), OvMaths::FVector3{ 0.f, 1.f, 0.f }, 1.f);
			m_context.shapeDrawer->DrawLine(center + rotation * (OvMaths::FVector3{ cos(i * (3.14f / 180.0f)), 0.f, sin(i * (3.14f / 180.0f)) } * radius), center + rotation * (OvMaths::FVector3{ cos((i + 10.0f) * (3.14f / 180.0f)), 0.f, sin((i + 10.0f) * (3.14f / 180.0f)) } * radius), OvMaths::FVector3{ 0.f, 1.f, 0.f }, 1.f);
		}
	}

	/* Draw the capsule collider if any */
	if (auto capsuleColliderComponent = p_actor.GetComponent<OvCore::ECS::Components::CPhysicalCapsule>(); capsuleColliderComponent)
	{
		float radius = abs(capsuleColliderComponent->GetRadius() * std::max(std::max(p_actor.transform.GetWorldScale().x, p_actor.transform.GetWorldScale().z), 0.f));
		float height = abs(capsuleColliderComponent->GetHeight() * p_actor.transform.GetWorldScale().y);
		float halfHeight = height / 2;

		FVector3 actorScale = p_actor.transform.GetWorldScale();
		OvMaths::FQuaternion rotation = p_actor.transform.GetWorldRotation();
		OvMaths::FVector3 center = p_actor.transform.GetWorldPosition();

		OvMaths::FVector3 hVec = { 0.0f, halfHeight, 0.0f };
		for (float i = 0; i < 360.0f; i += 10.0f)
		{
			m_context.shapeDrawer->DrawLine(center + rotation * (hVec + OvMaths::FVector3{ cos(i * (3.14f / 180.0f)), 0.f, sin(i * (3.14f / 180.0f)) } *radius), center + rotation * (hVec + OvMaths::FVector3{ cos((i + 10.0f) * (3.14f / 180.0f)), 0.f, sin((i + 10.0f) * (3.14f / 180.0f)) } *radius), OvMaths::FVector3{ 0.f, 1.f, 0.f }, 1.f);
			m_context.shapeDrawer->DrawLine(center + rotation * (-hVec + OvMaths::FVector3{ cos(i * (3.14f / 180.0f)), 0.f, sin(i * (3.14f / 180.0f)) } *radius), center + rotation * (-hVec + OvMaths::FVector3{ cos((i + 10.0f) * (3.14f / 180.0f)), 0.f, sin((i + 10.0f) * (3.14f / 180.0f)) } *radius), OvMaths::FVector3{ 0.f, 1.f, 0.f }, 1.f);

			if (i < 180.f)
			{
				m_context.shapeDrawer->DrawLine(center + rotation * (hVec + OvMaths::FVector3{ cos(i * (3.14f / 180.0f)), sin(i * (3.14f / 180.0f)), 0.f } * radius), center + rotation * (hVec + OvMaths::FVector3{ cos((i + 10.0f) * (3.14f / 180.0f)), sin((i + 10.0f) * (3.14f / 180.0f)), 0.f } * radius), OvMaths::FVector3{ 0.f, 1.f, 0.f }, 1.f);
				m_context.shapeDrawer->DrawLine(center + rotation * (hVec + OvMaths::FVector3{ 0.f, sin(i * (3.14f / 180.0f)), cos(i * (3.14f / 180.0f)) } * radius), center + rotation * (hVec + OvMaths::FVector3{ 0.f, sin((i + 10.0f) * (3.14f / 180.0f)), cos((i + 10.0f) * (3.14f / 180.0f)) } * radius), OvMaths::FVector3{ 0.f, 1.f, 0.f }, 1.f);
			}
			else
			{
				m_context.shapeDrawer->DrawLine(center + rotation * (-hVec + OvMaths::FVector3{ cos(i * (3.14f / 180.0f)), sin(i * (3.14f / 180.0f)), 0.f } * radius), center + rotation * (-hVec + OvMaths::FVector3{ cos((i + 10.0f) * (3.14f / 180.0f)), sin((i + 10.0f) * (3.14f / 180.0f)), 0.f } * radius), OvMaths::FVector3{ 0.f, 1.f, 0.f }, 1.f);
				m_context.shapeDrawer->DrawLine(center + rotation * (-hVec + OvMaths::FVector3{ 0.f, sin(i * (3.14f / 180.0f)), cos(i * (3.14f / 180.0f)) } * radius), center + rotation * (-hVec + OvMaths::FVector3{ 0.f, sin((i + 10.0f) * (3.14f / 180.0f)), cos((i + 10.0f) * (3.14f / 180.0f)) } * radius), OvMaths::FVector3{ 0.f, 1.f, 0.f }, 1.f);
			}
		}

		m_context.shapeDrawer->DrawLine(center + rotation * (OvMaths::FVector3{ -radius, -halfHeight, 0.f }),	center + rotation * (OvMaths::FVector3{ -radius, +halfHeight, 0.f }), OvMaths::FVector3{ 0.f, 1.f, 0.f }, 1.f);
		m_context.shapeDrawer->DrawLine(center + rotation * (OvMaths::FVector3{ radius, -halfHeight, 0.f }),	center + rotation * (OvMaths::FVector3{ radius, +halfHeight, 0.f }), OvMaths::FVector3{ 0.f, 1.f, 0.f }, 1.f);
		m_context.shapeDrawer->DrawLine(center + rotation * (OvMaths::FVector3{ 0.f, -halfHeight, -radius }),	center + rotation * (OvMaths::FVector3{ 0.f, +halfHeight, -radius }), OvMaths::FVector3{ 0.f, 1.f, 0.f }, 1.f);
		m_context.shapeDrawer->DrawLine(center + rotation * (OvMaths::FVector3{ 0.f, -halfHeight, radius }),	center + rotation * (OvMaths::FVector3{ 0.f, +halfHeight, radius }), OvMaths::FVector3{ 0.f, 1.f, 0.f }, 1.f);
	}

	m_context.renderer->SetCapability(OvRendering::Settings::ERenderingCapability::DEPTH_TEST, depthTestBackup);
	m_context.renderer->SetRasterizationLinesWidth(1.0f);
}

void OvEditor::Core::EditorRenderer::RenderAmbientBoxVolume(OvCore::ECS::Components::CAmbientBoxLight & p_ambientBoxLight)
{
	bool depthTestBackup = m_context.renderer->GetCapability(OvRendering::Settings::ERenderingCapability::DEPTH_TEST);
	m_context.renderer->SetCapability(OvRendering::Settings::ERenderingCapability::DEPTH_TEST, false);

	auto& data = p_ambientBoxLight.GetData();

	FMatrix4 model =
		FMatrix4::Translation(p_ambientBoxLight.owner.transform.GetWorldPosition()) *
		FMatrix4::Scaling({ data.constant * 2.f, data.linear * 2.f, data.quadratic * 2.f });

	OvMaths::FVector3 center = p_ambientBoxLight.owner.transform.GetWorldPosition();
	OvMaths::FVector3 size = { data.constant * 2.f, data.linear * 2.f, data.quadratic * 2.f };
	OvMaths::FVector3 actorScale = p_ambientBoxLight.owner.transform.GetWorldScale();
	OvMaths::FVector3 halfSize = size / 2.f;

	m_context.shapeDrawer->DrawLine(center + OvMaths::FVector3{ -halfSize.x, -halfSize.y, -halfSize.z }, center + OvMaths::FVector3{ -halfSize.x, -halfSize.y, +halfSize.z }, OvMaths::FVector3{ 1.f, 1.f, 0.f }, 1.f);
	m_context.shapeDrawer->DrawLine(center + OvMaths::FVector3{ -halfSize.x, halfSize.y, -halfSize.z }, center + OvMaths::FVector3{ -halfSize.x, +halfSize.y, +halfSize.z }, OvMaths::FVector3{ 1.f, 1.f, 0.f }, 1.f);
	m_context.shapeDrawer->DrawLine(center + OvMaths::FVector3{ -halfSize.x, -halfSize.y, -halfSize.z }, center + OvMaths::FVector3{ -halfSize.x, +halfSize.y, -halfSize.z }, OvMaths::FVector3{ 1.f, 1.f, 0.f }, 1.f);
	m_context.shapeDrawer->DrawLine(center + OvMaths::FVector3{ -halfSize.x, -halfSize.y, +halfSize.z }, center + OvMaths::FVector3{ -halfSize.x, +halfSize.y, +halfSize.z }, OvMaths::FVector3{ 1.f, 1.f, 0.f }, 1.f);
	m_context.shapeDrawer->DrawLine(center + OvMaths::FVector3{ +halfSize.x, -halfSize.y, -halfSize.z }, center + OvMaths::FVector3{ +halfSize.x, -halfSize.y, +halfSize.z }, OvMaths::FVector3{ 1.f, 1.f, 0.f }, 1.f);
	m_context.shapeDrawer->DrawLine(center + OvMaths::FVector3{ +halfSize.x, halfSize.y, -halfSize.z }, center + OvMaths::FVector3{ +halfSize.x, +halfSize.y, +halfSize.z }, OvMaths::FVector3{ 1.f, 1.f, 0.f }, 1.f);
	m_context.shapeDrawer->DrawLine(center + OvMaths::FVector3{ +halfSize.x, -halfSize.y, -halfSize.z }, center + OvMaths::FVector3{ +halfSize.x, +halfSize.y, -halfSize.z }, OvMaths::FVector3{ 1.f, 1.f, 0.f }, 1.f);
	m_context.shapeDrawer->DrawLine(center + OvMaths::FVector3{ +halfSize.x, -halfSize.y, +halfSize.z }, center + OvMaths::FVector3{ +halfSize.x, +halfSize.y, +halfSize.z }, OvMaths::FVector3{ 1.f, 1.f, 0.f }, 1.f);
	m_context.shapeDrawer->DrawLine(center + OvMaths::FVector3{ -halfSize.x, -halfSize.y, -halfSize.z }, center + OvMaths::FVector3{ +halfSize.x, -halfSize.y, -halfSize.z }, OvMaths::FVector3{ 1.f, 1.f, 0.f }, 1.f);
	m_context.shapeDrawer->DrawLine(center + OvMaths::FVector3{ -halfSize.x, +halfSize.y, -halfSize.z }, center + OvMaths::FVector3{ +halfSize.x, +halfSize.y, -halfSize.z }, OvMaths::FVector3{ 1.f, 1.f, 0.f }, 1.f);
	m_context.shapeDrawer->DrawLine(center + OvMaths::FVector3{ -halfSize.x, -halfSize.y, +halfSize.z }, center + OvMaths::FVector3{ +halfSize.x, -halfSize.y, +halfSize.z }, OvMaths::FVector3{ 1.f, 1.f, 0.f }, 1.f);
	m_context.shapeDrawer->DrawLine(center + OvMaths::FVector3{ -halfSize.x, +halfSize.y, +halfSize.z }, center + OvMaths::FVector3{ +halfSize.x, +halfSize.y, +halfSize.z }, OvMaths::FVector3{ 1.f, 1.f, 0.f }, 1.f);

	m_context.renderer->SetCapability(OvRendering::Settings::ERenderingCapability::DEPTH_TEST, depthTestBackup);
}

void OvEditor::Core::EditorRenderer::RenderAmbientSphereVolume(OvCore::ECS::Components::CAmbientSphereLight & p_ambientSphereLight)
{
	bool depthTestBackup = m_context.renderer->GetCapability(OvRendering::Settings::ERenderingCapability::DEPTH_TEST);
	m_context.renderer->SetCapability(OvRendering::Settings::ERenderingCapability::DEPTH_TEST, false);

	auto& data = p_ambientSphereLight.GetData();

	OvMaths::FQuaternion rotation = p_ambientSphereLight.owner.transform.GetWorldRotation();
	OvMaths::FVector3 center = p_ambientSphereLight.owner.transform.GetWorldPosition();
	float radius = data.constant;

	for (float i = 0; i <= 360.0f; i += 10.0f)
	{
		m_context.shapeDrawer->DrawLine(center + rotation * (OvMaths::FVector3{ cos(i * (3.14f / 180.0f)), sin(i * (3.14f / 180.0f)), 0.f } *radius), center + rotation * (OvMaths::FVector3{ cos((i + 10.0f) * (3.14f / 180.0f)), sin((i + 10.0f) * (3.14f / 180.0f)), 0.f } *radius), OvMaths::FVector3{ 1.f, 1.f, 0.f }, 1.f);
		m_context.shapeDrawer->DrawLine(center + rotation * (OvMaths::FVector3{ 0.f, sin(i * (3.14f / 180.0f)), cos(i * (3.14f / 180.0f)) } *radius), center + rotation * (OvMaths::FVector3{ 0.f, sin((i + 10.0f) * (3.14f / 180.0f)), cos((i + 10.0f) * (3.14f / 180.0f)) } *radius), OvMaths::FVector3{ 1.f, 1.f, 0.f }, 1.f);
		m_context.shapeDrawer->DrawLine(center + rotation * (OvMaths::FVector3{ cos(i * (3.14f / 180.0f)), 0.f, sin(i * (3.14f / 180.0f)) } *radius), center + rotation * (OvMaths::FVector3{ cos((i + 10.0f) * (3.14f / 180.0f)), 0.f, sin((i + 10.0f) * (3.14f / 180.0f)) } *radius), OvMaths::FVector3{ 1.f, 1.f, 0.f }, 1.f);
	}

	m_context.renderer->SetCapability(OvRendering::Settings::ERenderingCapability::DEPTH_TEST, depthTestBackup);
}

void OvEditor::Core::EditorRenderer::RenderModelAsset(OvRendering::Resources::Model& p_model)
{
	FMatrix4 model = OvMaths::FMatrix4::Scaling({ 3.f, 3.f, 3.f });
	m_context.renderer->DrawModelWithSingleMaterial(p_model, m_defaultMaterial, &model);
}

void OvEditor::Core::EditorRenderer::RenderTextureAsset(OvRendering::Resources::Texture & p_texture)
{
	FMatrix4 model = FMatrix4::RotateOnAxisX(FMatrix4::Scaling({ 5.f, 5.f, 5.f }), 90.f * 0.0174f);

	m_textureMaterial.Set<Texture*>("u_DiffuseMap", &p_texture);
	m_context.renderer->DrawModelWithSingleMaterial(*m_context.editorResources->GetModel("Plane"), m_textureMaterial, &model);
}

void OvEditor::Core::EditorRenderer::RenderMaterialAsset(OvCore::Resources::Material & p_material)
{
	FMatrix4 model = OvMaths::FMatrix4::Scaling({ 3.f, 3.f, 3.f });
	m_context.renderer->DrawModelWithSingleMaterial(*m_context.editorResources->GetModel("Sphere"), p_material, &model, &m_emptyMaterial);
}

void OvEditor::Core::EditorRenderer::RenderGrid(const OvMaths::FVector3& p_viewPos, const OvMaths::FVector3& p_color)
{
	FMatrix4 model = FMatrix4::Scaling({ 1000.f, 1.f, 1000.f });
	m_gridMaterial.Set("u_Color", p_color);
	m_context.renderer->DrawModelWithSingleMaterial(*m_context.editorResources->GetModel("Plane"), m_gridMaterial, &model);

	// m_context.shapeDrawer->DrawGrid(p_viewPos, p_color, 1000, 0.1f, 0.5f, 20.0f, 1.0f);
}

void OvEditor::Core::EditorRenderer::UpdateLights(OvCore::SceneSystem::Scene& p_scene)
{
	std::vector<FMatrix4> lightMatrices;
	m_context.renderer->FindLightMatrices(p_scene, lightMatrices);
	m_context.lightSSBO->SendBlocks<FMatrix4>(lightMatrices.data(), lightMatrices.size() * sizeof(FMatrix4));
}