#pragma once

class Class;

class ObjectBase
{
public:
    [[nodiscard]] virtual Class* get_class() const = 0;

    
};