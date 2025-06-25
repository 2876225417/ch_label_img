#ifndef METHOD_CONCEPTS_H
#define METHOD_CONCEPTS_H

#include <pch.h>

#if __cplusplus >= 202002L
template <typename T>
concept InternalHelper = true;

template <typename T>
concept EventHelper = InternalHelper<T>;





template <typename T>
concept PureHelper = InternalHelper<T>;

template <typename T>
concept ContextDependentHelper = InternalHelper<T>;



#endif // __cplusplus >= 202002L


#endif // METHOD_CONCEPTS_H