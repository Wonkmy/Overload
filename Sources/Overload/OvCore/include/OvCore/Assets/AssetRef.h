#pragma once

#include <concepts>
#include <OvCore/Assets/Asset.h>

namespace OvCore::Assets
{
	template<typename T>
	concept AssetType = std::is_base_of<Asset, T>::value;

	/**
	* A simple asset reference. Doesn't need the underlying asset to be loaded.
	*/
	struct GenericAssetRef
	{
		AssetID id;
	};

	/**
	* Provide utility functions to manage the underlying asset.
	*/
	template<AssetType T>
	class AssetRef : public GenericAssetRef
	{

	};
}
