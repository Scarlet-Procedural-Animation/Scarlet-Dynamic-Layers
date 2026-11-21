#pragma once

#include "../Core/Include/Defines.h"
#include "../Core/Include/StackVariations.h"

#include <algorithm>
#include <vector>

namespace DynamicLayers
{

namespace Stacks
{

namespace gtc // General Type Cores namespace
{

// Abstract class for all 
template<typename TOutValue, template <class> class TLayer, class TDerivedClass>
class StackBasicOperationsCore : public sb::StackMultiOperation<TOutValue, TLayer, TDerivedClass>
{
public:
    using Base = sb::StackBase<TOutValue, TLayer, TDerivedClass>;

    enum class LayerBlendType : char
    {
        Override,
        Add,
        Multiply
    };
    using LayerType = LayerBlendType;

protected:
    // Order index of the top most Override-type layer. All calculations are started from this index
    size_t topOverrideLayerOrderIndex = 0;

// Friendly interface for TLayer to call
protected:
    // Called by TLayer when it's value is changed
    void onValueChanged(size_t order_index, const TOutValue& old_value, const TOutValue& new_value)
    {
        this->shouldRecalculate = true;
        this->lowestChangedOrderIndex = std::min(this->lowestChangedOrderIndex, order_index);
    }

    // Called by TLayer when it's type is changed
    void onTypeChanged(size_t order_index)
    {
        this->shouldRecalculate = true;
        this->lowestChangedOrderIndex = std::min(this->lowestChangedOrderIndex, order_index);
    }

    // Called by TLayer when it's priority is changed
    void onPriorityChanged(size_t order_index)
    {
        INT layer_id = this->stackOrder[order_index];
        INT priority = this->layers[layer_id].priority;

        sinkLayerInOrder(layer_id, priority, order_index);
        raiseLayerInOrder(layer_id, priority, this->layers[layer_id].order_index);

        this->shouldRecalculate = true;
        this->lowestChangedOrderIndex = std::min(this->lowestChangedOrderIndex, std::min(order_index, this->layers[layer_id].order_index));
    }
};

} // namespace gtc

template<typename TOutValue, template <class> class TLayer>
class StackBasicOperations final : public gtc::StackBasicOperationsCore<TOutValue, TLayer, StackBasicOperations<TOutValue, TLayer>>
{
public:
    using Base = sb::StackBase<TOutValue, TLayer, StackBasicOperations<TOutValue, TLayer>>;
    using Core = gtc::StackBasicOperationsCore<TOutValue, TLayer, StackBasicOperations<TOutValue, TLayer>>;

    using LayerType = typename Core::LayerBlendType;
    using ValueType = typename Base::ValueType;

public:
    // Returns resulting stack value
    const TOutValue& getValue()
    {
        if (this->shouldRecalculate)
            recalculate();
        return this->valueCache[size() - 1];
    }

protected:
    // Recalculates stack's value cache
    void recalculate()
    {
        // TO DO
    }
};

} // namespace Stacks
} // namespace DynamicLayers