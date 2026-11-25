#pragma once

#include "LayerBase.h"

namespace DynamicLayers
{

// Layer class with a single value entry
template<class TOutValueType, class TBlendType, class TStack>
class LayerMono : public lb::LayerBase<TStack>
{
public:
    using Self = LayerMono<TStack>;
    using Base = lb::LayerBase<TStack>;

public:
    LayerMono() {}
    virtual ~LayerMono() override {}

protected:
    // Value entry
    TOutValueType value;

public:
    // Layers main function - returning the value
    const TOutValueType& getValue() { return value; }

    // Updates layer's value to a new one
    void setValue(const TOutValueType& new_value)
    {
        this->stack->onValueChanged(this->order_index, value, new_value);
        value = new_value;
    }

private:
    friend TStack;
};

} // DynamicLayers namespace