#pragma once

#include "../Core/Include/LayerBase.h"

namespace DynamicLayers
{

// Layer class with a single value entry
template<class TStack>
class LayerMono : public lb::LayerBase<TStack>
{
public:
    using Self = LayerMono<TStack>;
    using Base = lb::LayerBase<TStack>;

    using ValueType = typename Base::ValueType;
    using LayerType = typename Base::LayerType;

public:
    LayerMono() {}
    virtual ~LayerMono() override {}

protected:
    // Value entry
    ValueType value;

public:
    // Layers main function - returning the value
    const ValueType& getValue() { return value; }

    // Updates layer's value to a new one
    void setValue(const ValueType& new_value)
    {
        this->stack->onValueChanged(this->order_index, value, new_value);
        value = new_value;
    }

private:
    friend TStack;
};

} // DynamicLayers namespace