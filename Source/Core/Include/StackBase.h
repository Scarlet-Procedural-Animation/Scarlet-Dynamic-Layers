#pragma once

#include "Defines.h"

#include <vector>

namespace DynamicLayers
{

namespace sb // Stack Basics namespace
{

// Abstract base class for all dynamic layer stacks
template<typename TOutValue, template <class> class TLayer, class TDerivedClass>
class StackBase
{
public:
    using LayerT = TLayer<TDerivedClass>;

    // To be namehidden in derived classes
    using ValueType = TOutValue; 
    using LayerType = char;

protected:
    // Dynamic layer storage, does not reflect application order
    std::vector<LayerT> layers;

    // Contains layer ids (indexing `layers` vector) in the application order
    std::vector<INT> stackOrder;

    // Contains indices of unused slots in `layers` vector
    // Such free indices appear when deleting layers (since layer ids must be maintained)
    std::vector<size_t> freeIndices;

public:
    // Default constructor
    StackBase() {}

    // Pure virtual destructor (this is an abstract class)
    virtual ~StackBase() = 0;

public:
    // Editable reference to a layer
    LayerT& operator[](INT index) { return layers[index]; }
    
    // Non-editable reference to a layer
    const LayerT& operator[](INT index) const { return layers[index]; }

    // Returns stack order
    // Stack order contains layer ids in the application order
    const std::vector<INT>& getOrder() { return stackOrder; }

    // Layer manipulation

    // Adds a layer to the stack
    // Returns created layer's id - use it to access and modify layer's data
    virtual INT addLayer(const LayerT& layer)
    {
        // Storing the layer
        INT id = addLayerToStorage(layer);

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
    INT addLayerToStorage(const LayerT& layer)
    {
        if (freeIndices.empty())
        {
            layers.push_back(layer);
            return layers.size() - 1;
        }

        else 
        {
            size_t free_index = *(freeIndices.end());
            layers[free_index] = layer;
            freeIndices.pop_back();
            return free_index;
        }
    }

    // Inserts layer into the stack order
    void insertLayerToOrder(INT layer_id)
    {
        stackOrder.push_back(layer_id);

        if (stackOrder.size() == 1) return;

        INT priority = layers[layer_id].priority;

        size_t i = stackOrder.size() - 2;
        while (priority < layers[stackOrder[i]].priority)
        {
            stackOrder[i + 1] = stackOrder[i];
            stackOrder[i] = layer_id;

            layers[stackOrder[i + 1]].order_index = i + 1;
            layers[layer_id].order_index = i;

            i++;
        }
    }

    // Removes layer from stack order
    void removeFromOrder(INT layer_id)
    {
        size_t i = layers[layer_id].order_index;
        while (i < stackOrder.size() - 1)
        {
            stackOrder[i] = stackOrder[i + 1];
            i++;
        }
        stackOrder.pop_back();
    }

private:
    friend class LayerT::Self;
};

} // namespace sb
} // namespace DynamicLayers