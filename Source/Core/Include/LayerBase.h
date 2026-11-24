#pragma once

#include "Defines.h"

namespace DynamicLayers
{

namespace lb // Layer Basics
{

// Abstact base class for all dynamic layers 
template<class TStack>
class LayerBase
{
public:
    using ValueType = typename TStack::ValueType;
    using LayerType = typename TStack::LayerType;

    // To be namehidden in derived classes
    using Self = LayerBase;

public:
    LayerBase() {}
    virtual ~LayerBase() = 0;

protected:
    // Type of the layer in the stack
    LayerType type;

    // Priority of the layer in the stack
    INT priority;

    // Pointer to the owner stack
    TStack* stack;

    // Index of the layer in the owner stack's order
    size_t order_index;

public:
    // Constructor
    LayerBase(LayerType type_, INT prioriry_) : type(type_), priority(prioriry_) {}

    // Pure virtual destructor
    virtual ~LayerBase() = 0;

public:
    // Type of the layer in the stack
    LayerType getType() { return type; }

    // Set type of the layer in the stack
    void setType(LayerType new_type) 
    { 
        LayerType old_type = type;
        type = new_type; 
        stack->onTypeChanged(order_index, old_type, new_type);
    }

    // Priority of the layer in the stack
    INT getPriority() { return priority; }

    // Set priority of the layer in the stack
    void setPriority(INT new_priority) 
    { 
        INT old_priority = priority;
        priority = new_priority; 
        stack->onPriorityChanged(order_index, old_priority, new_priority);
    }

private:
    friend TStack;
};

} // namespace lb

} // namespace DynamicLayers