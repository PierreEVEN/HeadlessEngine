
#pragma once
#include <algorithm>
#include <vector>

class Class;

class FamilySignature
{
    friend class Family;

  public:
    FamilySignature(std::vector<Class*> in_elements) : elements(std::move(in_elements))
    {
        std::ranges::sort(elements);
    }

    FamilySignature erased(Class* removed_component) const
    {
        auto signature = elements;
        signature.erase(std::ranges::find(signature, removed_component));
        return signature;
    }

    FamilySignature added(Class* added_component) const
    {
        auto signature = elements;
        signature.emplace_back(added_component);
        return signature;
    }

    bool operator==(const FamilySignature& other) const
    {
        if (other.elements.size() != elements.size())
            return false;

        for (size_t i = 0; i < elements.size(); ++i)
            if (other.elements[i] != elements[i])
                return false;
        return true;
    }

  private:
    friend struct std::hash<FamilySignature>;
    std::vector<Class*> elements;
};

template <> struct std::hash<FamilySignature>
{
    std::size_t operator()(const FamilySignature& s) const noexcept
    {
        size_t hash = 0;
        for (const auto& hash_value : s.elements)
            hash = hash * 37 + reinterpret_cast<size_t>(hash_value);
        return hash;
    }
}; // namespace std
