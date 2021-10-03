# Assets

**Declaration**

**Conventions note : all asset classes begins with a A**

```cpp
#pragma once
#include <assets/asset_base.h>

class AAssetClass : public AssetBase
{
  public:
    AAssetClass(Ctor_parameters...);
};
```

**Instantiation**
```cpp
AssetManager::get()->create<AAssetClass>(Asset_UID, Ctor_parameters...); -> return a TAssetPtr<AAssetClass>
```

When instancied, assets are automatically registered in the asset manager.
After that, you can access this asset from anywhere using `TAssetPtr<AAssetClass>(Asset_UID)`
Assets will be automatically deleted on program exit.

# Scene


A scene contains multiple nodes represented in a scene graph.
The scene is rendered using a CameraNode (the same scene can be rendered through multiple views)

**Conventions note : all node classes begins with a N**

**Declaration**
```cpp
Scene my_scene;

my_scene->add_node<NNodeClass>(NodeUID, Ctor_parameters...); -> return a std::shared_ptr<NNodeClass>
```

**Instantiation**

```cpp
#include <scene/node_base.h>

class NNodeClass : public NodeBase
{
  public:
    NNodeClass(Ctor_parameters...);
};
```