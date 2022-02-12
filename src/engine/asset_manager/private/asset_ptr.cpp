
#include "asset_manager/asset_ptr.h"

#include "asset_manager/asset_manager.h"
/*
IAssetPtr::IAssetPtr()
{
    clear();
}

IAssetPtr::IAssetPtr(const AssetId& in_asset_id)
{
    set(in_asset_id);
}

IAssetPtr::IAssetPtr(AssetBase* in_asset)
{
    set(in_asset);
}

IAssetPtr::IAssetPtr(const IAssetPtr& other) : asset_id(other.asset_id), asset(other.asset)
{
    set(other.asset);
}

IAssetPtr::IAssetPtr(IAssetPtr&& other) noexcept
{
    set(other.asset);
}

void IAssetPtr::set(AssetBase* in_asset)
{
    if (in_asset == asset)
        return;
    clear();
    if (!in_asset)
        return;
    asset_id = std::make_shared<AssetId>(in_asset->get_id());
    asset    = in_asset;
    bind();
}

void IAssetPtr::set(const AssetId& in_asset_id)
{
    if (asset_id && in_asset_id == *asset_id)
        return;
    clear();
    asset_id = std::make_shared<AssetId>(in_asset_id);
    if (AssetManager::is_valid())
        asset = AssetManager::get()->find(*asset_id);
    bind();
}

void IAssetPtr::operator=(const IAssetPtr& other)
{
    set(other.asset);
}

void IAssetPtr::clear()
{
    unbind();

    asset    = nullptr;
    asset_id = nullptr;
}

AssetBase* IAssetPtr::get()
{
    if (!asset_id)
        return nullptr;
    if (!asset)
        if (AssetManager::is_valid())
            asset = AssetManager::get()->find(*asset_id);
    if (asset)
    {
        return asset;
    }
    return nullptr;
}

AssetBase* IAssetPtr::get_const() const
{
    if (!asset_id)
        return nullptr;
    if (asset)
    {
        return asset;
    }
    return nullptr;
}

std::string IAssetPtr::to_string() const
{
    if (!asset)
        return "null";
    return asset->to_string();
}

IAssetPtr::~IAssetPtr()
{
    clear();
}

void IAssetPtr::unbind()
{
    if (asset)
    {
        asset->on_delete_asset.clear_object(this);
    }
}

void IAssetPtr::bind()
{
    if (asset)
    {
        asset->on_delete_asset.add_object(this, &IAssetPtr::on_delete_asset);
    }
}

void IAssetPtr::on_delete_asset(AssetBase* deleted_asset)
{
    if (deleted_asset == asset)
        set(nullptr);
}

*/