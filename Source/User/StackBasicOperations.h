#pragma once

#include "Defines.h"
#include "StackVariations.h"
#include "Operations.h"

#include <algorithm>
#include <vector>

namespace DynamicLayers
{

namespace gtc // General Type Cores namespace
{

enum class BlendType_BasicOperations : char
{
    Override,
    Add,
    Multiply
};

// Abstract class for all 
template<typename TOutValue, template <class, class, class> class TLayer, class TDerivedClass>
class StackBasicOperationsCore : public sb::StackMultiOperation<TOutValue, TLayer, TDerivedClass>
{
public:
    using Base = sb::StackBase<TOutValue, TLayer, TDerivedClass>;
    using Super = sb::StackMultiOperation<TOutValue, TLayer, TDerivedClass>;

    using LayerBlendType = BlendType_BasicOperations;

public:
    StackBasicOperationsCore();
    virtual ~StackBasicOperationsCore() = 0;

protected:
    // Order index of the top most Override-type layer. All calculations are started from this index
    size_t topOverrideLayerOrderIndex = 0;

public:
    virtual INT addLayer(const typename Base::LayerType& layer) override
    {
        INT id = Super::addLayer(layer);
        size_t order_index = this->layers[id].order_index;

        if (layer.type == LayerBlendType::Override)
            topOverrideLayerOrderIndex = std::max(topOverrideLayerOrderIndex, order_index);

        return id;
    }

    virtual void removeLayer(INT layer_id) override
    {
        Super::removeLayer(layer_id);

        // Find top override layer from scrath
        // Not sure if optimization is possible / worth it
        topOverrideLayerOrderIndex = findTopOverrideLayer(size() - 1);
    }

// Friendly interface for TLayer to call
protected:

    // NOTE : all notify functions except onValueChanged are called AFTER the change is applied
    // onValueChanged, however is called BEFORE the change to the value is applied
    // This is to allow passing of old_value by reference (avoiding copying)
    // Remember: onValueChanged is not supposed to perform a full stack recalculation!

    // Called by TLayer when it's value is changed
    void onValueChanged(size_t order_index, const TOutValue& old_value, const TOutValue& new_value)
    {
        this->shouldRecalculate = true;
        this->lowestChangedOrderIndex = std::min(this->lowestChangedOrderIndex, order_index);
    }

    // Called by TLayer when it's type is changed
    void onTypeChanged(size_t order_index, LayerBlendType old_type, LayerBlendType new_type)
    {
        this->shouldRecalculate = true;
        this->lowestChangedOrderIndex = std::min(this->lowestChangedOrderIndex, order_index);

        if (order_index == topOverrideLayerOrderIndex && new_type != LayerBlendType::Override)
        {
            if (order_index > 0)
                topOverrideLayerOrderIndex = findTopOverrideLayer(order_index - 1);
            else
                topOverrideLayerOrderIndex = 0;
        }
            
        else if (new_type == LayerBlendType::Override)
            topOverrideLayerOrderIndex = std::max(topOverrideLayerOrderIndex, order_index);
    }

    // Called by TLayer when it's priority is changed
    void onPriorityChanged(size_t order_index, INT old_priority, INT new_priority)
    {
        INT layer_id = this->stackOrder[order_index];
        INT priority = this->layers[layer_id].priority;

        sinkLayerInOrder(layer_id, priority, order_index);
        raiseLayerInOrder(layer_id, priority, this->layers[layer_id].order_index);

        this->shouldRecalculate = true;
        this->lowestChangedOrderIndex = std::min(this->lowestChangedOrderIndex, std::min(order_index, this->layers[layer_id].order_index));
    }

protected:
    // Finds the highest override layer that stands below top_search_index
    size_t findTopOverrideLayer(size_t top_search_index)
    {
        size_t i = top_search_index;
        while (i > 0)
            if (this->layers[this->stackOrder[i]].type == LayerBlendType::Override)
                return i;

        return 0;
    }
};

} // namespace gtc

template<typename TOutValue, template <class, class, class> class TLayer>
class StackBasicOperations final : public gtc::StackBasicOperationsCore<TOutValue, TLayer, StackBasicOperations<TOutValue, TLayer>>
{
public:
    using Base = sb::StackBase<TOutValue, TLayer, StackBasicOperations<TOutValue, TLayer>>;
    using Core = gtc::StackBasicOperationsCore<TOutValue, TLayer, StackBasicOperations<TOutValue, TLayer>>;

    using LayerBlendType = typename Core::LayerBlendType;
    using ValueType = typename Base::ValueType;

public:
    StackBasicOperations();
    virtual ~StackBasicOperations() override {}

public:
    // Returns resulting stack value
    const TOutValue& getValue()
    {
        if (size() == 0)
            return Operations::zero<TOutValue>();

        if (this->shouldRecalculate)
            recalculate();
        return this->valueCache[size() - 1];
    }

protected:
    // Recalculates stack's value cache
    void recalculate()
    {
        size_t start = std::max(this->lowestChangedOrderIndex, this->topOverrideLayerOrderIndex);

        if (start < size())
            for (size_t i = start; i < size(); i++)
            {
                size_t layer_id = static_cast<size_t>(this->stackOrder[i]);
                if (i == 0)
                {
                    this->valueCache[i] == this->layers[layer_id].getValue();
                }

                else
                {
                    TOutValue layer_value = this->layers[layer_id].getValue();
                    LayerBlendType type = this->layers[layer_id].type;

                    if (type == LayerBlendType::Override) { this->valueCache[i] = layer_value; }
                    else if (type == LayerBlendType::Add)
                    {
                        this->valueCache[i] = Operations::add<TOutValue, TOutValue, TOutValue>(this->valueCache[i - 1], layer_value);
                    }
                    else // LayerBlendType::Multiply
                    {
                        this->valueCache[i] = Operations::multiply<TOutValue, TOutValue, TOutValue>(this->valueCache[i - 1], layer_value);
                    }
                }
            }

        this->shouldRecalculate = false;
        this->lowestChangedOrderIndex = size();
    }
};


template<typename TOutValue, template <class, class, class> class TLayer>
class StackBasicOperationsToggleable final : public gtc::StackBasicOperationsCore<TOutValue, TLayer, StackBasicOperations<TOutValue, TLayer>>
{
public:
    using Base = sb::StackBase<TOutValue, TLayer, StackBasicOperations<TOutValue, TLayer>>;
    using Core = gtc::StackBasicOperationsCore<TOutValue, TLayer, StackBasicOperations<TOutValue, TLayer>>;

    using LayerBlendType = typename Core::LayerBlendType;
    using ValueType = typename Base::ValueType;

public:
    StackBasicOperationsToggleable();
    virtual ~StackBasicOperationsToggleable() override {}

public:
    // Returns resulting stack value
    const TOutValue& getValue()
    {
        if (size() == 0)
            return Operations::zero<TOutValue>();
        if (this->shouldRecalculate)
            recalculate();
        return this->valueCache[size() - 1];
    }

// Friendly interface for TLayer to call
protected:

    // Called by TLayer when it's Enabled status is changed
    void onEnabledChanged(size_t order_index, bool new_enabled)
    {
        INT layer_id = this->stackOrder[order_index];
        LayerBlendType type = this->layers[layer_id].type;

        this->shouldRecalculate = true;
        this->lowestChangedOrderIndex = std::min(this->lowestChangedOrderIndex, order_index); 
    }  

protected:
    // Recalculates stack's value cache
    void recalculate()
    {
        size_t start = std::max(this->lowestChangedOrderIndex, this->topOverrideLayerOrderIndex);

        // Find the first enabled layer
        while (this->layers[this->stackOrder[start]].enabled == false && start > 0)
            start--;
        while (this->layers[this->stackOrder[start]].enabled == false && start < size())
            start++;

        if (start < size())
            for (size_t i = start; i < size(); i++)
            {
                size_t layer_id = static_cast<size_t>(this->stackOrder[i]);
                if (this->layers[layer_id].enabled)
                {
                    if (i == 0)
                    {
                        this->valueCache[i] == this->layers[layer_id].getValue();
                    }

                    else
                    {
                        TOutValue layer_value = this->layers[layer_id].getValue();
                        LayerBlendType type = this->layers[layer_id].type;

                        if (type == LayerBlendType::Override) { this->valueCache[i] = layer_value; }
                        else if (type == LayerBlendType::Add)
                        {
                            this->valueCache[i] = Operations::add<TOutValue, TOutValue, TOutValue>(this->valueCache[i - 1], layer_value);
                        }
                        else // LayerBlendType::Multiply
                        {
                            this->valueCache[i] = Operations::multiply<TOutValue, TOutValue, TOutValue>(this->valueCache[i - 1], layer_value);
                        }
                    }
                }
            }

        this->shouldRecalculate = false;
        this->lowestChangedOrderIndex = size();
    }
};

} // namespace DynamicLayers