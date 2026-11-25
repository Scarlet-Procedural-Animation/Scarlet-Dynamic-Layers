#pragma once

#include "Defines.h"
#include "StackBase.h"

#include <vector>

namespace DynamicLayers
{
    
namespace sb // Stack Basics namespace
{

// An abstract version of stack base that is meant to be used for stacks with multiple incompatible types of operations
template<typename TOutValue, template <class, class, class> class TLayer, class TDerivedClass>
class StackMultiOperation : public StackBase<TOutValue, TLayer, TDerivedClass>
{
public:
    using Base = StackBase<TOutValue, TLayer, TDerivedClass>;
    
protected:
    // Fully calculated value cache for each execution step - synced with stackOrder
    std::vector<TOutValue> valueCache;

    // Whether value needs to be recalculated upon next getValue() or not
    bool shouldRecalculate = false;

    // Lowest order index of a layer that was changed between calculations
    size_t lowestChangedOrderIndex = 0;

public:
    // Default constructor
    StackMultiOperation();

    // Pure virtual destructor (this is an abstract class)
    virtual ~StackMultiOperation() = 0;

public:

    // Adds a layer to the stack
    // Returns created layer's id - use it to access and modify layer's data
    virtual INT addLayer(const typename Base::LayerType& layer) override
    {
        INT id = Base::addLayer(layer);
        
        valueCache.push_back(TOutValue());

        shouldRecalculate = true;
        lowestChangedOrderIndex = this->layers[id].order_index;
    }

    // Removes a layer from the stack
    virtual void removeLayer(INT layer_id) override
    {
        shouldRecalculate = true;
        lowestChangedOrderIndex = this->layers[layer_id].order_index;

        valueCache.pop_back();

        Base::removeLayer(layer_id);
    }

private:
    friend typename Base::LayerType::Self;
};

} // namespace sb
} // namespace DynamicLayers