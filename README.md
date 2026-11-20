# Scarlet Dynamic Layers
An optimized C++ layer-based value management system for gamedev applications.

**This is the core of the library, designed to be used as a submodule - it contains no building functionality.**
### General
Dynamic Layers is a custom data structure developed for game development purposes. They provide a non destructive and highly optimized layer-based value combination approach for dynamic systems.

*A typical use case for it is the following:*
Value `A` of an arbitrary type depends on a vector of values `V`, that need to be combined using arithmetic (or any other sort of) operations in order to calculate the resulting `A`. Values in `V` come from different sources and can be changed or deleted and reordered at any time.

*Solution to the typical case, achieved using Dynamic Layers:*
`A` is represented by a stack of layers, each layer contains one or multiple values, that can be controlled by external sources. When needed, resulting value of `A` can be acquired using `getValue()` method. 
Value `A` is recalculated upon calling `getValue()` **only if** there was a change in layer values since last calculation, otherwise a cached value is used. Additional optimizations are applied to reduce the number of calculations to the possible minimum.
### Code Design

![](Documentation/Code%20Design.png)
[Code Design Excalidraw](Documentation/Code%20Design.md)

Dynamic layers are designed primarily using C++ templates. No virtual methods are used at any stage of calculations (only for layer insertion and deletion). With the result of careful design, all functions are expected to inline nicely.