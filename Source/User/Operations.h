#pragma once

namespace DynamicLayers
{
  
namespace Operations
{
  
template<typename T>
struct OpZero
{
  static T get() { return T{}; }
};

template<typename T, typename LHS, typename RHS>
struct OpAdd
{
  static T add(const LHS& lhs, const RHS& rhs)
  {
    return lhs + rhs;
  }
};

template<typename T, typename LHS, typename RHS>
struct OpMultiply
{
  static T multiply(const LHS& lhs, const RHS& rhs)
  {
    return lhs * rhs;
  }
};

template<typename T, typename A, typename B, typename FAC>
struct OpInterpolate
{
  static T interpolate(const A& a, const B& b, const FAC& fac)
  {
    return (1 - fac) * a + fac * b;
  }
};
  
} // namespace Operations
} // namespace DynamicLayers