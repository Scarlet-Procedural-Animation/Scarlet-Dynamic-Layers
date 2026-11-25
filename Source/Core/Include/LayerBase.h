#pragma once

#include "Defines.h"

namespace DynamicLayers
{

namespace lb // Layer Basics
{

// Abstact base class for all dynamic layers 
template<class TOutValueType, class TBlendType, class TStack>
class LayerBase
{
public:
    // To be namehidden in derived classes
    using Self = LayerBase;

protected:
    // Type of the layer in the stack
    TBlendType type;

    // Priority of the layer in the stack
    INT priority;

    // Pointer to the owner stack
    TStack* stack;

    // Index of the layer in the owner stack's order
    size_t order_index;

public:
    // Constructor
    LayerBase(TBlendType type_, INT prioriry_) : type(type_), priority(prioriry_) {}

    // Pure virtual destructor
    virtual ~LayerBase() = 0;

public:
    // Type of the layer in the stack
    TBlendType getType() { return type; }

    // Set type of the layer in the stack
    void setType(TBlendType new_type) 
    { 
        TBlendType old_type = type;
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