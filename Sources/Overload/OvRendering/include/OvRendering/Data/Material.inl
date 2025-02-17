/**
* @project: Overload
* @author: Overload Tech.
* @licence: MIT
*/

#pragma once

#include <OvDebug/Logger.h>

#include "OvRendering/Data/Material.h"

namespace OvRendering::Data
{
	template<typename T>
	inline void Material::Set(const std::string p_key, const T& p_value)
	{
		if (HasShader())
		{
			if (m_uniformsData.find(p_key) != m_uniformsData.end())
			{
				m_uniformsData[p_key] = std::any(p_value);
			}
		}
		else
		{
			OVLOG_ERROR("Material Set failed: No attached shader");
		}
	}

	template<typename T>
	inline const T& Material::Get(const std::string p_key) const
	{
		return
			m_uniformsData.find(p_key) == m_uniformsData.end() ?
			std::any_cast<T>(m_uniformsData.at(p_key)) :
			T();
	}
}