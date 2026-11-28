#pragma once

#include "Defines.h"

#include <vector>

namespace DynamicLayers
{

namespace sb // Stack Basics namespace
{

// Abstract base class for all dynamic layer stacks
template<typename TOutValue, typename TLayerBlendType, template <class, class, class> class TLayer, class TDerivedClass>
class StackBase
{
public:
    using LayerType = TLayer<TOutValue, TLayerBlendType, TDerivedClass>;

    // To be namehidden in derived classes
    using ValueType = TOutValue;

protected:
    // Dynamic layer storage, does not reflect application order
    std::vector<LayerType> layers;

    // Contains layer ids (indexing `layers` vector) in the application order
    std::vector<INT> stackOrder;

    // Contains indices of unused slots in `layers` vector
    // Such free indices appear when deleting layers (since layer ids must be maintained)
    std::vector<size_t> freeIndices;

public:
    // Default constructor
    StackBase() {}

    // Pure virtual destructor (this is an abstract class)
    virtual ~StackBase() = 0 {}

public:
    // Editable reference to a layer
    LayerType& operator[](INT id) { return layers[id]; }
    
    // Non-editable reference to a layer
    const LayerType& operator[](INT id) const { return layers[id]; }

    // Number of layers in the stack
    size_t size() { return stackOrder.size(); } // Because `layers` may contain free slots

    // Returns stack order
    // Stack order contains layer ids in the application order
    const std::vector<INT>& getOrder() const { return stackOrder; }

    // Layer manipulation

    // Adds a layer to the stack
    // Returns created layer's id - use it to access and modify layer's data
    virtual INT addLayer(const LayerType& layer)
    {
        // Storing the layer
        INT id = addLayerToStorage(layer);

        // Linking layer to this stack
        layers[id].setStack(static_cast<TDerivedClass*>(this));

        // Adding layer to order
        insertLayerToOrder(id);

        return id;
    }

    // Removes a layer from the stack
    virtual void removeLayer(INT layer_id)
    {
        // Remove from stack order
        removeFromOrder(layer_id);

        // Mark as free
        freeIndices.push_back(layer_id);
    }

protected:

    // Finds a free space or pushes back the layer
    INT addLayerToStorage(const LayerType& layer)
    {
        if (freeIndices.empty())
        {
            layers.push_back(layer);
            return layers.size() - 1;
        }

        else 
        {
            size_t free_index = freeIndices.back();
            layers[free_index] = layer;
            freeIndices.pop_back();
            return free_index;
        }
    }

    // Inserts layer into the stack order
    void insertLayerToOrder(INT layer_id)
    {
        stackOrder.push_back(layer_id);

        layers[layer_id].setOrderIndex(stackOrder.size() - 1);

        if (stackOrder.size() == 1) return;

        INT priority = layers[layer_id].getPriority();

        sinkLayerInOrder(layer_id, priority, stackOrder.size() - 1);
    }

    void sinkLayerInOrder(INT layer_id, INT priority, size_t order_index)
    {
        int i = order_index - 1;
        while (i >= 0 && priority < layers[stackOrder[i]].getPriority())
        {
            stackOrder[i + 1] = stackOrder[i];
            stackOrder[i] = layer_id;

            layers[stackOrder[i + 1]].setOrderIndex(i + 1);
            layers[layer_id].setOrderIndex(i);

            i--;
        }
    }

    void raiseLayerInOrder(INT layer_id, INT priority, size_t order_index)
    {
        size_t i = order_index + 1;
        while (i < stackOrder.size() && priority > layers[stackOrder[i]].getPriority())
        {
            stackOrder[i - 1] = stackOrder[i];
            stackOrder[i] = layer_id;

            layers[stackOrder[i - 1]].setOrderIndex(i - 1);
            layers[layer_id].setOrderIndex(i);

            i++;
        }
    }

    // Removes layer from stack order
    void removeFromOrder(INT layer_id)
    {
        size_t i = layers[layer_id].getOrderIndex();
        while (i < stackOrder.size() - 1)
        {
            stackOrder[i] = stackOrder[i + 1];
            layers[stackOrder[i]].setOrderIndex(i);
            i++;
        }
        stackOrder.pop_back();
    }
};

} // namespace sb
} // namespace DynamicLayers