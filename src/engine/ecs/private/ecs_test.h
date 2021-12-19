#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <algorithm>

using IDType = std::uint32_t;
using EntityID = IDType;
using ComponentTypeID = IDType;
const IDType NULL_ENTITY = 0;

typedef std::vector<ComponentTypeID> ArchetypeID;

template <class T> class TypeIdGenerator
{
private:
    static IDType m_count;

public:
    template <class U> static const IDType GetNewID()
    {
        static const IDType idCounter = m_count++;
        return idCounter;
    }
};

template <class T> IDType TypeIdGenerator<T>::m_count = 0;

/*********************
 * COMPONENT
 *-******************/
class ComponentBase
{
public:
    virtual ~ComponentBase()
    {
    }

    virtual void DestroyData(unsigned char* data) const = 0;
    virtual void MoveData(unsigned char* source, unsigned char* destination) const = 0;
    virtual void ConstructData(unsigned char* data) const = 0;

    virtual std::size_t GetSize() const = 0;
};

template <class C> class Component : public ComponentBase
{
public:
    virtual void DestroyData(unsigned char* data) const override
    {
        C* dataLocation = std::launder(reinterpret_cast<C*>(data));

        dataLocation->~C();
    }

    virtual void MoveData(unsigned char* source, unsigned char* destination) const override
    {
        new(&destination[0]) C(std::move(*reinterpret_cast<C*>(source)));
    }

    virtual void ConstructData(unsigned char* data) const override
    {
        new(&data[0]) C();
    }

    virtual std::size_t GetSize() const override
    {
        return sizeof(C);
    }

    static ComponentTypeID GetTypeID()
    {
        return TypeIdGenerator<ComponentBase>::GetNewID<C>();
    }
};

typedef unsigned char* ComponentData;

/*********************
 * ECS
 *-******************/
struct Archetype
{
    ArchetypeID                type;
    std::vector<ComponentData> componentData;
    std::vector<EntityID>      entityIds;
    std::vector<std::size_t>   componentDataSize;
};

class ECS
{
private:
    using ComponentTypeIDBaseMap = std::unordered_map<ComponentTypeID, ComponentBase*>;

    struct Record
    {
        Archetype*  archetype;
        std::size_t index;
    };

    using EntityArchetypeMap = std::unordered_map<EntityID, Record>;

    using ArchetypesArray = std::vector<Archetype*>;

public:
    ECS()
        : m_entityIdCounter(1)
    {
    }

    ~ECS()
    {
        for (Archetype* archetype : m_archetypes)
        {
            for (std::size_t i = 0; i < archetype->type.size(); ++i)
            {
                const ComponentBase* const comp     = m_componentMap[archetype->type[i]];
                const std::size_t&         dataSize = comp->GetSize();
                for (std::size_t e = 0; e < archetype->entityIds.size(); ++e)
                {
                    comp->DestroyData(&archetype->componentData[i][e * dataSize]);
                }
                delete[] archetype->componentData[i];
            }
            delete archetype;
        }
        for (ComponentTypeIDBaseMap::value_type& p : m_componentMap)
            delete p.second;
    }

    EntityID GetNewID()
    {
        return m_entityIdCounter++;
    }

    template <class C> void RegisterComponent()
    {
        ComponentTypeID componentTypeId = Component<C>::GetTypeID();

        if (m_componentMap.contains(componentTypeId))
            return; // can't re-register a type

        m_componentMap.emplace(componentTypeId, new Component<C>);
    }

    template <class C> bool IsComponentRegistered();

    void RegisterEntity(const EntityID entityId)
    {
        Record dummyRecord;
        dummyRecord.archetype          = nullptr;
        dummyRecord.index              = 0;
        m_entityArchetypeMap[entityId] = dummyRecord;
    }

    Archetype* GetArchetype(const ArchetypeID& id)
    {
        for (Archetype* archetype : m_archetypes)
        {
            if (archetype->type == id)
                return archetype;
        }

        // archetype doesn't exist, so create a new one

        Archetype* newArchetype = new Archetype;
        newArchetype->type      = id;
        m_archetypes.push_back(newArchetype);

        // add an empty array for each component in the type
        for (ArchetypeID::size_type i = 0; i < id.size(); ++i)
        {
            newArchetype->componentData.push_back(new unsigned char[0]);
            newArchetype->componentDataSize.push_back(0);
        }

        return newArchetype;
    }

    template <class C, typename... Args> C* AddComponent(const EntityID& entityId, Args&&...args);

    template <class C> void RemoveComponent(const EntityID& entityId);

    template <class C> C* GetComponent(const EntityID& entityId);

    template <class C> bool HasComponent(const EntityID& entityId);

    void RemoveEntity(const EntityID& entityId);

    template <class... Cs> std::vector<EntityID> GetEntitiesWith();

private:
    EntityArchetypeMap m_entityArchetypeMap;

    ArchetypesArray m_archetypes;

    EntityID m_entityIdCounter;

    ComponentTypeIDBaseMap m_componentMap;
};

template <class C, typename... Args> C* ECS::AddComponent(const EntityID& entityId, Args&&...args)
{
    ComponentTypeID newCompTypeId = Component<C>::GetTypeID();

    if (!IsComponentRegistered<C>())
    {
        std::cerr << "Component Not Registered!" << std::endl;
        return nullptr;
    }

    const std::size_t& compDataSize = m_componentMap[newCompTypeId]->GetSize();

    // this ensures the entity is added to dummy archetype if needed
    Record&    record       = m_entityArchetypeMap[entityId];
    Archetype* oldArchetype = record.archetype;

    C* newComponent = nullptr; // will be returned

    Archetype* newArchetype = nullptr;

    if (oldArchetype)
    {
        if (std::find(oldArchetype->type.begin(), oldArchetype->type.end(), newCompTypeId) != oldArchetype->type.end())
        {
            // this entity already contains this component, we can't have
            // multiple so just exit
            return nullptr;
        }

        ArchetypeID newArchetypeId = oldArchetype->type;
        newArchetypeId.push_back(newCompTypeId);
        std::sort(newArchetypeId.begin(), newArchetypeId.end());

        newArchetype = GetArchetype(newArchetypeId);

        for (std::size_t j = 0; j < newArchetypeId.size(); ++j)
        {
            const ComponentTypeID& newCompId = newArchetypeId[j];

            const ComponentBase* const newComp = m_componentMap[newCompId];

            const std::size_t& newCompDataSize = newComp->GetSize();

            std::size_t currentSize = newArchetype->entityIds.size() * newCompDataSize;
            std::size_t newSize     = currentSize + newCompDataSize;
            if (newSize > newArchetype->componentDataSize[j])
            {
                newArchetype->componentDataSize[j] *= 2;
                newArchetype->componentDataSize[j] += newCompDataSize;
                unsigned char* newData = new unsigned char[newArchetype->componentDataSize[j]];
                for (std::size_t e = 0; e < newArchetype->entityIds.size(); ++e)
                {
                    newComp->MoveData(&newArchetype->componentData[j][e * newCompDataSize], &newData[e * newCompDataSize]);
                    newComp->DestroyData(&newArchetype->componentData[j][e * newCompDataSize]);
                }
                delete[] newArchetype->componentData[j];

                newArchetype->componentData[j] = newData;
            }

            ArchetypeID oldArchetypeId = oldArchetype->type;

            for (std::size_t i = 0; i < oldArchetype->type.size(); ++i)
            {
                const ComponentTypeID& oldCompId = oldArchetype->type[i];
                if (oldCompId == newCompId)
                {
                    const ComponentBase* const oldComp = m_componentMap[oldCompId];

                    const std::size_t& oldCompDataSize = oldComp->GetSize();

                    oldComp->MoveData(&oldArchetype->componentData[i][record.index * oldCompDataSize], &newArchetype->componentData[j][currentSize]);
                    oldComp->DestroyData(&oldArchetype->componentData[i][record.index * oldCompDataSize]);

                    goto cnt;
                }
            }

            newComponent = new(&newArchetype->componentData[j][currentSize]) C(std::forward<Args>(args)...);

        cnt:;
        }

        if (!oldArchetype->entityIds.empty())
        {
            for (std::size_t i = 0; i < oldArchetype->type.size(); ++i)
            {
                const ComponentTypeID& oldCompTypeID = oldArchetype->type[i];

                if (oldCompTypeID == newCompTypeId)
                {
                    ComponentBase* removeWrapper = m_componentMap[newCompTypeId];
                    removeWrapper->DestroyData(&oldArchetype->componentData[i][record.index * sizeof(C)]);
                }

                const ComponentBase* const oldComp = m_componentMap[oldCompTypeID];

                const std::size_t& oldCompDataSize = oldComp->GetSize();

                std::size_t    currentSize = oldArchetype->entityIds.size() * oldCompDataSize;
                std::size_t    newSize     = currentSize - oldCompDataSize;
                unsigned char* newData     = new unsigned char[oldArchetype->componentDataSize[i] - oldCompDataSize];
                oldArchetype->componentDataSize[i] -= oldCompDataSize;
                for (std::size_t e = 0, ei = 0; e < oldArchetype->entityIds.size(); ++e)
                {
                    if (e == record.index)
                        continue;

                    oldComp->MoveData(&oldArchetype->componentData[i][e * oldCompDataSize], &newData[ei * oldCompDataSize]);
                    oldComp->DestroyData(&oldArchetype->componentData[i][e * oldCompDataSize]);

                    ++ei;
                }

                delete[] oldArchetype->componentData[i];

                oldArchetype->componentData[i] = newData;
            }
        }

        std::vector<EntityID>::iterator willBeRemoved = std::find(oldArchetype->entityIds.begin(), oldArchetype->entityIds.end(), entityId);

        std::for_each(willBeRemoved, oldArchetype->entityIds.end(), [this, &oldArchetype](const EntityID& eid)
        {
            Record& moveR = m_entityArchetypeMap[eid];
            --moveR.index;
        });

        oldArchetype->entityIds.erase(willBeRemoved);
    }
    else
    {
        ArchetypeID newArchetypeId(1, newCompTypeId);

        const ComponentBase* const newComp = m_componentMap[newCompTypeId];

        newArchetype = GetArchetype(newArchetypeId);

        std::size_t currentSize = newArchetype->entityIds.size() * compDataSize;
        std::size_t newSize     = currentSize + compDataSize;
        if (newSize > newArchetype->componentDataSize[0])
        {
            newArchetype->componentDataSize[0] *= 2;
            newArchetype->componentDataSize[0] += compDataSize;
            unsigned char* newData = new unsigned char[newArchetype->componentDataSize[0]];
            for (std::size_t e = 0; e < newArchetype->entityIds.size(); ++e)
            {
                newComp->MoveData(&newArchetype->componentData[0][e * compDataSize], &newData[e * compDataSize]);
                newComp->DestroyData(&newArchetype->componentData[0][e * compDataSize]);
            }
            delete[](newArchetype->componentData[0]);

            newArchetype->componentData[0] = newData;
        }

        newComponent = new(&newArchetype->componentData[0][currentSize]) C(std::forward<Args>(args)...);
    }

    newArchetype->entityIds.push_back(entityId);
    record.index     = newArchetype->entityIds.size() - 1;
    record.archetype = newArchetype;

    return newComponent;
}


/*********************
 * ENTITY
 *-******************/
class Entity
{
public:
    explicit Entity(ECS& ecs)
        : m_id(ecs.GetNewID()), m_ecs(ecs)
    {
        m_ecs.RegisterEntity(m_id);
    }

    template <class C, typename... Args> C* Add(Args&&...args)
    {
        return m_ecs.AddComponent<C>(m_id, std::forward<Args>(args)...);
    }

    template <class C> C* Add(C&& c)
    {
        return m_ecs.AddComponent<C>(m_id, std::forward<C>(c));
    }

    EntityID GetID() const
    {
        return m_id;
    }

private:
    EntityID m_id;
    ECS&     m_ecs;
};


template <class C> void ECS::RemoveComponent(const EntityID& entityId)
{
    if (!IsComponentRegistered<C>())
        return;

    ComponentTypeID compTypeId = Component<C>::GetTypeID();

    if (!m_entityArchetypeMap.contains(entityId))
        return; // it doesn't exist

    Record& record = m_entityArchetypeMap[entityId];

    Archetype* oldArchetype = record.archetype;

    if (!oldArchetype)
        return; // there's no components anyway

    if (std::find(oldArchetype->type.begin(), oldArchetype->type.end(), compTypeId) == oldArchetype->type.end())
    {
        // this entity doesn't have this component
        return;
    }

    // find the new archetypeId by removing the old ComponentTypeId
    ArchetypeID newArchetypeId = oldArchetype->type;
    newArchetypeId.erase(std::remove(newArchetypeId.begin(), newArchetypeId.end(), compTypeId), newArchetypeId.end());
    std::sort(newArchetypeId.begin(), newArchetypeId.end());

    Archetype* newArchetype = GetArchetype(newArchetypeId);

    for (std::size_t j = 0; j < newArchetypeId.size(); ++j)
    {
        const ComponentTypeID& newCompId = newArchetypeId[j];

        const ComponentBase* const newComp = m_componentMap[newCompId];

        const std::size_t& newCompDataSize = newComp->GetSize();

        std::size_t currentSize = newArchetype->entityIds.size() * newCompDataSize;
        std::size_t newSize     = currentSize + newCompDataSize;
        if (newSize > newArchetype->componentDataSize[j])
        {
            newArchetype->componentDataSize[j] *= 2;
            newArchetype->componentDataSize[j] += newCompDataSize;
            unsigned char* newData = new unsigned char[newSize];
            for (std::size_t e = 0; e < newArchetype->entityIds.size(); ++e)
            {
                newComp->MoveData(&newArchetype->componentData[j][e * newCompDataSize], &newData[e * newCompDataSize]);
                newComp->DestroyData(&newArchetype->componentData[j][e * newCompDataSize]);
            }
            delete[] newArchetype->componentData[j];

            newArchetype->componentData[j] = newData;
        }

        newComp->ConstructData(&newArchetype->componentData[j][currentSize]);

        ArchetypeID oldArchetypeId = oldArchetype->type;

        for (std::size_t i = 0; i < oldArchetype->type.size(); ++i)
        {
            const ComponentTypeID& oldCompId = oldArchetype->type[i];

            if (oldCompId == newCompId)
            {
                const std::size_t& oldCompDataSize = m_componentMap[oldCompId]->GetSize();

                ComponentBase* removeWrapper = m_componentMap[oldCompId];
                removeWrapper->MoveData(&oldArchetype->componentData[i][record.index * oldCompDataSize], &newArchetype->componentData[j][currentSize]);

                removeWrapper->DestroyData(&oldArchetype->componentData[i][record.index * oldCompDataSize]);

                break;
            }
        }
    }

    for (std::size_t i = 0; i < oldArchetype->type.size(); ++i)
    {
        const ComponentTypeID& oldCompTypeID = oldArchetype->type[i];

        // if this is the component being removed, we should also destruct it
        if (oldCompTypeID == compTypeId)
        {
            ComponentBase* removeWrapper = m_componentMap[compTypeId];
            removeWrapper->DestroyData(&oldArchetype->componentData[i][record.index * sizeof(C)]);
        }

        const ComponentBase* const oldComp = m_componentMap[oldCompTypeID];

        const std::size_t& oldCompDataSize = oldComp->GetSize();

        std::size_t    currentSize = oldArchetype->entityIds.size() * oldCompDataSize;
        std::size_t    newSize     = currentSize - oldCompDataSize;
        unsigned char* newData     = new unsigned char[oldArchetype->componentDataSize[i] - oldCompDataSize];
        oldArchetype->componentDataSize[i] -= oldCompDataSize;
        for (std::size_t e = 0, ei = 0; e < oldArchetype->entityIds.size(); ++e)
        {
            if (e == record.index)
                continue;

            oldComp->MoveData(&oldArchetype->componentData[i][e * oldCompDataSize], &newData[ei * oldCompDataSize]);
            oldComp->DestroyData(&oldArchetype->componentData[i][e * oldCompDataSize]);

            ++ei;
        }

        delete[] oldArchetype->componentData[i];

        oldArchetype->componentData[i] = newData;
    }

    // each entity in the old archetypes entityIds after this one now
    // has an index 1 less
    std::vector<EntityID>::iterator willBeRemoved = std::find(oldArchetype->entityIds.begin(), oldArchetype->entityIds.end(), entityId);

    std::for_each(willBeRemoved, oldArchetype->entityIds.end(), [this, &oldArchetype](const EntityID& eid)
    {
        Record& moveR = m_entityArchetypeMap[eid];
        --moveR.index;
    });

    oldArchetype->entityIds.erase(std::remove(oldArchetype->entityIds.begin(), oldArchetype->entityIds.end(), entityId), oldArchetype->entityIds.end());

    newArchetype->entityIds.push_back(entityId);
    record.index     = newArchetype->entityIds.size() - 1;
    record.archetype = newArchetype;
}

void ECS::RemoveEntity(const EntityID& entityId)
{
    if (!m_entityArchetypeMap.contains(entityId))
        return; // it doesn't exist

    Record& record = m_entityArchetypeMap[entityId];

    Archetype* oldArchetype = record.archetype;

    if (!oldArchetype)
    {
        m_entityArchetypeMap.erase(entityId);
        return; // we wouldn't know where to delete
    }

    for (std::size_t i = 0; i < oldArchetype->type.size(); ++i)
    {
        const ComponentTypeID& oldCompId = oldArchetype->type[i];

        const ComponentBase* const comp = m_componentMap[oldCompId];

        const std::size_t& compSize = comp->GetSize();

        comp->DestroyData(&oldArchetype->componentData[i][record.index * compSize]);
    }

    for (std::size_t i = 0; i < oldArchetype->type.size(); ++i)
    {
        const ComponentTypeID& oldCompID = oldArchetype->type[i];

        const ComponentBase* const oldComp = m_componentMap[oldCompID];

        const std::size_t& oldCompDataSize = oldComp->GetSize();

        std::size_t    currentSize = oldArchetype->entityIds.size() * oldCompDataSize;
        std::size_t    newSize     = currentSize - oldCompDataSize;
        unsigned char* newData     = new unsigned char[oldArchetype->componentDataSize[i] - oldCompDataSize];
        oldArchetype->componentDataSize[i] -= oldCompDataSize;
        for (std::size_t e = 0, ei = 0; e < oldArchetype->entityIds.size(); ++e)
        {
            if (e == record.index)
                continue;

            oldComp->MoveData(&oldArchetype->componentData[i][e * oldCompDataSize], &newData[ei * oldCompDataSize]);

            oldComp->DestroyData(&oldArchetype->componentData[i][e * oldCompDataSize]);

            ++ei;
        }

        delete[] oldArchetype->componentData[i];

        oldArchetype->componentData[i] = newData;
    }

    m_entityArchetypeMap.erase(entityId);

    std::vector<EntityID>::iterator willBeRemoved = std::find(oldArchetype->entityIds.begin(), oldArchetype->entityIds.end(), entityId);

    std::for_each(willBeRemoved, oldArchetype->entityIds.end(), [this, &oldArchetype, &entityId](const EntityID& eid)
    {
        if (eid == entityId)
            return; // no need to adjust our removing one
        Record& moveR = m_entityArchetypeMap[eid];
        moveR.index -= 1;
    });

    oldArchetype->entityIds.erase(willBeRemoved);
}
