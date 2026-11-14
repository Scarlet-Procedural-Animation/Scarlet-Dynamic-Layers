#pragma once

#include <map>
#include <string>
#include <vector>

// ENUMS

// Blending mode for location/rotation/... layers in stacks
enum class LayerBlendingMode
{
	Normal,
	Add,
	Multiply
};

// The rule by which the elements in a single layer are combined
enum class LayerCombinationMode
{
	Add,
	Multiply,
	Average
};


// SOURCE INTERFACE

// Interface for sources in stacks and layers
class LayerValueSource
{
public:
	// Is this source active?
	virtual bool IsSourceActive() const;
};


// CLASS TEMPLATES

// Zero value template
template<typename T>
struct OpZero;

template<typename T, typename LHS, typename RHS>
struct OpAdd
{
	static T Add(const LHS& lhs, const RHS& rhs)
	{
		return rhs + lhs;
	}
};

template<typename T, typename LHS, typename RHS>
struct OpMultiply
{
	static T Multiply(const LHS& lhs, const RHS& rhs)
	{
		return rhs * lhs;
	}
};

template<typename T, typename LHS, typename RHS>
struct OpDivide
{
	static T Divide(const LHS& lhs, const RHS& rhs)
	{
		return lhs / rhs;
	}
};

template<typename T>
struct OpInterpolate
{
	static T Interpolate(const T& Start, const T& Finish, float Factor)
	{
		return Start + Factor * (Finish - Start);
	}
};

template<typename T>
struct LayerBase
{
	// Whether the layer affects the associated stack or not
	bool Enabled = true;

	// The way the layer is applied on top of underlying layers 
	LayerBlendingMode BlendingMode;

	// Blending factor used when applying this layer on top of underlying layers
	float BlendingFactor = 1.f;

	// The way layer elements are combined to get the resulting value
	LayerCombinationMode CombinationMode;

	// Layer elements with pointers to their sources, allowing for non-destructive addition and removal of elements to/from the layer
	std::map<LayerValueSource*, T> LayerElements;

	// Higher priority -> higher in the stack
	int Priority = 0;

	// Usually only active elements are used in calculating the resulting value, this flag forces the use of all layer elements
	bool ForceAllElementsActive = false;

	// Name of the layer, used for debugging
	std::string Name = "";


	LayerBase(LayerBlendingMode InBlendingMode = LayerBlendingMode::Normal,
		float InBlendingFactor = 1.f,
		LayerCombinationMode InLayerCombinationMode = LayerCombinationMode::Average,
		int InPriority = 0,
		bool InForceAllElementsActive = false,
		std::string InName = "") :
		BlendingMode(InBlendingMode), BlendingFactor(InBlendingFactor), CombinationMode(InLayerCombinationMode), Priority(InPriority), 
		ForceAllElementsActive(InForceAllElementsActive), Name(InName) {}


	// Calculates the resulting value from the layer elements
	T CalculateLayerValue(bool& OutHasActiveElements) const
	{
		T OutValue = OpZero<T>::Get();
		int ActiveElementCount = 0;

		bool FirstElement = false;

		switch (CombinationMode)
		{

		case LayerCombinationMode::Average:

			for (auto& Source : LayerElements)
			{
				if (!Source.Key.IsValid()) continue;

				bool SourceActive = true;
				if (Source.first)
					SourceActive = Source.first->IsSourceActive();

				bool CurrentActive = ForceAllElementsActive || SourceActive;

				if (CurrentActive)
					OutValue = OpAdd<T, T, T>::Add(OutValue, Source.Value);
				ActiveElementCount += CurrentActive;
			}

			if (ActiveElementCount > 0)
				OutValue = OpDivide<T, T, int>::Divide(OutValue, ActiveElementCount);

			break;

		case LayerCombinationMode::Add:

			for (auto& Source : LayerElements)
			{
				if (!Source.Key.IsValid()) continue;

				bool SourceActive = true;
				if (Source.first)
					SourceActive = Source.first->IsSourceActive();

				bool CurrentActive = ForceAllElementsActive || SourceActive;

				if (CurrentActive)
					OutValue = OpAdd<T, T, T>::Add(OutValue, Source.Value);
				ActiveElementCount += CurrentActive;
			}
			break;

		case LayerCombinationMode::Multiply:

			FirstElement = true;
			for (auto& Source : LayerElements)
			{
				if (!Source.Key.IsValid()) continue;

				bool SourceActive = true;
				if (Source.first)
					SourceActive = Source.first->IsSourceActive();

				bool CurrentActive = ForceAllElementsActive || SourceActive;

				ActiveElementCount += CurrentActive;
				if (CurrentActive)
				{
					if (FirstElement)
					{
						OutValue = Source.Value;
						FirstElement = false;
					}

					else
						OutValue = OpMultiply<T, T, T>::Multiply(OutValue, Source.Value);
				}
			}
			break;

		default:
			break;
		}

		OutHasActiveElements = ActiveElementCount > 0;
		return OutValue;
	}

	// Adds/Updates a sourced value in the layer.
	void SetSourceValue(LayerValueSource* InSource, const T& InSourceValue)
	{
		LayerElements.Add(InSource, InSourceValue);
	}

	// Removes a source value from the layer. If succeded: returns true, false - overwise
	bool RemoveSourceValue(LayerValueSource* InSource)
	{
		return LayerElements.Remove(InSource) != 0;
	}

	// Returns the value associated with the specified source, returns default value if source isn't valid
	const T& GetSourceValue(LayerValueSource* InSource) const
	{
		const T* ValueP = LayerElements.Find(InSource);
		if (ValueP)
			return *ValueP;

		return OpZero<T>::Get();
	}
};


template<typename T>
struct StackBase
{
	// [LayerID] -> <Layer>
	std::vector<LayerBase<T>> Layers;

	// [Execution Step] -> <LayerID>
	std::vector<int> StackOrder;

	// Since normal layers override everything that lies beneath them, it is logical to start compuatation from the top-most normal layer with blending factor of 1.0f
	// But that layer may not be active for any number of reasons, so we cache all of the normal layers with blending factor of 1.0f in this array, so we can check if they are active in runtime
	// Layers are identified by their stack order id and placed top to bottom (0 - the top-most such layer)
	std::vector<int> StartingLayersCache;

	// Name of the stack, mainly used for debugging
	std::string StackName = "";


	StackBase(const std::string& InName = "") : StackName(InName) {}

	// Number of layers in the stack
	int size() const { return Layers.size(); }

	// Access to the stack layers
	LayerBase<T>& operator[] (int InID) { return Layers[InID]; }


	// Adds a new layer into the stack and updates the stack order
	int AddLayer(LayerBase<T> InLayer)
	{
		// Storing the layer
        Layers.push_back(InLayer);
		int ID = Layers.size() - 1;

		// Updating stack order
        StackOrder.push_back(ID);
		int LastIndex = StackOrder.size() - 1;

		// Insertion sort for just the last element (works in O(N), N - number of elements in the stack)
		int i = LastIndex - 1;
		while (i >= 0 && Layers[StackOrder[i]].Priority > InLayer.Priority)
		{
			StackOrder[i + 1] = StackOrder[i];
			i--;
		}

		StackOrder[i + 1] = ID;
		RecalculateStartingLayerCache();

		return ID;
	}

	// Removes a layer from the stack
	bool RemoveLayer(int InLayerID)
	{
		if (!(InLayerID >= 0 && InLayerID < size())) return false;

		Layers.erase(Layers.begin() + InLayerID);
		StackOrder.erase(remove(StackOrder.begin(), StackOrder.end(), InLayerID), StackOrder.end());

		RecalculateStartingLayerCache();

		return true;
	}

	// Recalucates StartingLayersCache array, should be called whenether a new layer is added/removed or some normal layer's blending factor is changed
	void RecalculateStartingLayerCache()
	{
		StartingLayersCache.clear();

		for (int i = size() - 1; i >= 0; i--)
			if (Layers[StackOrder[i]].BlendingMode == LayerBlendingMode::Normal && Layers[StackOrder[i]].BlendingFactor == 1.f)
				StartingLayersCache.push_back(i);
	}

	// Calculates the final value of the stack based on it's layers
	T CalculateStackValue() const
	{
		T FinalValue;

		int StartingLayer = -1;
		for (int i = 0; i < StartingLayersCache.size(); i++)
		{
			int StackOrderID = StartingLayersCache[i];

			const LayerBase<T>& Layer = Layers[StackOrder[StackOrderID]];

			if (!Layer.Enabled) continue;

			bool HasActiveElements = false;
			FinalValue = Layer.CalculateLayerValue(HasActiveElements);

			if (HasActiveElements && Layer.BlendingFactor == 1.0f)
			{
				StartingLayer = StackOrderID + 1; // +1 because we have already calculated this layer's value and stored it in FinalValue
				break;
			}
		}

		if (StartingLayer == -1) // Rare case, when there are no normal layers with blending factor of 1.0f present
		{
			FinalValue = OpZero<T>::Get();
			StartingLayer = 0;
		}

		for (int LayerOrderID = StartingLayer; LayerOrderID < size(); LayerOrderID++)
		{
			const LayerBase<T>& Layer = Layers[StackOrder[LayerOrderID]];

			if (!Layer.Enabled) continue;

			bool HasActiveElements = false;
			T LayerValue = Layer.CalculateLayerValue(HasActiveElements);

			if (HasActiveElements)
			{
				switch (Layer.BlendingMode)
				{
				case LayerBlendingMode::Normal:
					FinalValue = OpInterpolate<T>::Interpolate(FinalValue, LayerValue, Layer.BlendingFactor);
					break;

				case LayerBlendingMode::Add:
					FinalValue = OpInterpolate<T>::Interpolate(FinalValue, OpAdd<T, T, T>::Add(FinalValue, LayerValue), Layer.BlendingFactor);
					break;

				case LayerBlendingMode::Multiply:
					FinalValue = OpInterpolate<T>::Interpolate(FinalValue, OpMultiply<T, T, T>::Multiply(FinalValue, LayerValue), Layer.BlendingFactor);
					break;

				default: break;
				}
			}
		}

		return FinalValue;
	}
};
