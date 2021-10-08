

#include "assets/asset_base.h"

static std::shared_ptr<AssetManager> asset_manager_instance;

bool AssetManager::is_valid()
{
    return asset_manager_instance.get();
}

void AssetManager::destroy()
{
    asset_manager_instance = nullptr;
}

AssetManager::~AssetManager()
{
    std::lock_guard lock(asset_map_lock);
    for (auto& item : assets)
        delete item.second;

    std::lock_guard lock2(dirty_assets_lock);
    assets_to_delete.clear();
}

void AssetManager::remove(IAssetPtr* asset_reference)
{
    if (!asset_reference || !*asset_reference)
        return;

    std::lock_guard lock(asset_map_lock);
    auto            found_asset = assets.find(asset_reference->id());
    AssetBase*      asset_ptr   = found_asset->second;
    assets.erase(found_asset);
    if (!asset_ptr)
    {
        LOG_ERROR("error : referenced asset %s is NULL", asset_reference->id().to_string().c_str());
        return;
    }
    std::lock_guard lock2(dirty_assets_lock);
    assets_to_delete.push_back(asset_ptr);
}

AssetBase* AssetManager::find(const AssetId& id)
{
    std::lock_guard<std::mutex> lock(asset_map_lock);
    auto                        asset = assets.find(id);
    if (asset == assets.end())
        return nullptr;
    return asset->second;
}

std::unordered_map<AssetId, AssetBase*> AssetManager::get_assets()
{
    std::lock_guard lock(asset_map_lock);
    return assets;
}

void AssetManager::try_delete_dirty_items()
{
    std::lock_guard         lock(dirty_assets_lock);
    std::vector<AssetBase*> deleted_assets;
    for (const auto& asset : assets_to_delete)
    {
        if (asset->try_delete())
            deleted_assets.emplace_back(asset);
    }

    for (const auto& asset : deleted_assets)
    {
        on_delete_asset.execute(asset);
        delete asset;
        assets_to_delete.erase(std::find(assets_to_delete.begin(), assets_to_delete.end(), asset));
    }
}

void AssetManager::set(std::shared_ptr<AssetManager> in_asset_manager)
{
    asset_manager_instance = in_asset_manager;
}

std::shared_ptr<AssetManager> AssetManager::get_internal()
{
    if (!asset_manager_instance)
    {
        LOG_ERROR("cannot get asset manager instance while it has not been created");
        return nullptr;
    }
    return asset_manager_instance;
}

std::string AssetBase::to_string() const
{
    return asset_id->to_string();
}

AssetId AssetBase::get_id() const
{
    if (!asset_id)
        LOG_FATAL("asset id is null : %x", this);
    return AssetId(*asset_id);
}

bool AssetBase::is_transient_resource()
{
    return true;
}

bool AssetBase::try_load()
{
    return true;
}

bool AssetBase::try_delete()
{
    return true;
}

AssetBase::~AssetBase()
{
    delete asset_id;
    asset_id = nullptr;
}

void AssetBase::internal_constructor(const AssetId& id)
{
    asset_id = new AssetId(id);
}

AssetId AssetManager::find_valid_asset_id(const std::string& asset_name)
{
    if (!find(asset_name))
        return asset_name;

    int asset_index = 1;
    while (find(asset_name + "_" + std::to_string(asset_index)))
    {
        asset_index++;
    }

    return asset_name + "_" + std::to_string(asset_index);
}
