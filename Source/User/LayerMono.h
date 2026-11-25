#pragma once

#include "LayerBase.h"

namespace DynamicLayers
{

// Layer class with a single value entry
template<typename TOutValueType, typename TBlendType, class TStack>
class LayerMono : public lb::LayerBase<TOutValueType, TBlendType, TStack>
{
public:
    using Base = lb::LayerBase<TOutValueType, TBlendType, TStack>;

public:
    LayerMono(TBlendType type_, INT priority_) : Base(type_, priority_) {}
    virtual ~LayerMono() override {}

protected:
    // Value entry
    TOutValueType value;

public:
    // Layers main function - returning the value
    const TOutValueType& getValue() const { return value; }

    // Updates layer's value to a new one
    void setValue(const TOutValueType& new_value)
    {
        this->stack->onValueChanged(this->order_index, value, new_value);
        value = new_value;
    }
};

} // DynamicLayers namespace