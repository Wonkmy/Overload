/**
* @project: Overload
* @author: Overload Tech.
* @licence: MIT
*/

#pragma once

#include <OvTools/Utils/OptRef.h>

#include <OvRendering/Data/Describable.h>
#include <OvRendering/Data/Material.h>
#include <OvRendering/Resources/Mesh.h>

namespace OvRendering::Entities
{
	/**
	* Drawable entity
	*/
	struct Drawable : public Data::Describable
	{
		OvTools::Utils::OptRef<OvRendering::Resources::IMesh> mesh;
		OvTools::Utils::OptRef<OvRendering::Data::Material> material;
		OvRendering::Data::StateMask stateMask;
		OvRendering::Settings::EPrimitiveMode primitiveMode = OvRendering::Settings::EPrimitiveMode::TRIANGLES;
		OvRendering::Resources::Shader::FeatureSet shaderFeatureSet;
	};
}
