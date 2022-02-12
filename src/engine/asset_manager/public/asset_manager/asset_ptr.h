#pragma once

#include <memory>

#include "asset_id.h"

/*
class AssetBase;

class IAssetPtr
{
  public:
    IAssetPtr();
    IAssetPtr(const AssetId& in_asset_id);
    IAssetPtr(AssetBase* in_asset);
    explicit IAssetPtr(const IAssetPtr& other);
    explicit IAssetPtr(IAssetPtr&& other) noexcept;
    virtual ~IAssetPtr();

    void set(AssetBase* in_asset);
    void set(const AssetId& in_asset_id);
    void clear();

    [[nodiscard]] AssetBase* get();
    [[nodiscard]] AssetBase* get_const() const;
    [[nodiscard]] AssetId    id() const
    {
        return *asset_id.get();
    }
    [[nodiscard]] std::string to_string() const;

    bool operator!() const
    {
        return !asset;
    }

    void operator=(const IAssetPtr& other);

    explicit operator bool() const
    {
        return asset && asset_id;
    }

  private:
    void unbind();
    void bind();
    void on_delete_asset(AssetBase* deleted_asset);

    std::shared_ptr<AssetId> asset_id = nullptr;
    AssetBase*               asset    = nullptr;
};

template <class AssetClass> class TAssetPtr final : public IAssetPtr
{
  public:
    TAssetPtr() : IAssetPtr()
    {
    }
    TAssetPtr(const AssetId& in_asset_id) : IAssetPtr(in_asset_id)
    {
    }
    TAssetPtr(AssetClass* in_asset) : IAssetPtr(static_cast<AssetBase*>(in_asset))
    {
    }

    AssetClass* operator->()
    {
        return static_cast<AssetClass*>(get());
    }

    AssetClass* operator->() const
    {
        return static_cast<AssetClass*>(get_const());
    }
};

*/