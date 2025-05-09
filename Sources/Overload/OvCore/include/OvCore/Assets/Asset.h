#pragma once

#include <cstdint>

namespace OvCore::Assets
{
	using AssetID = uint32_t;

	class Asset
	{
	public:
		AssetID id;

		virtual bool IsLoaded() const = 0;
		virtual void Load() = 0;
		virtual void Offload() = 0;
	};
}
