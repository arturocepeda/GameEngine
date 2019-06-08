
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Types
//
//  --- GESTLTypes.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Core/GEAllocator.h"


//
//  Containers
//
#include <array>
#include <vector>
#include <list>
#include <queue>
#include <set>
#include <map>

#define GESTLArray(T, Size) std::array<T, Size>
#define GESTLVector(T) std::vector<T, GE::Core::STLAllocator<T>>
#define GESTLList(T) std::list<T, GE::Core::STLAllocator<T>>
#define GESTLDeque(T) std::deque<T, GE::Core::STLAllocator<T>>
#define GESTLQueue(T) std::queue<T, GESTLDeque(T)>
#define GESTLPriorityQueue(T) std::priority_queue<T, GESTLVector(T)>
#define GESTLPriorityQueueCustom(T, Comparator) std::priority_queue<T, GESTLVector(T), Comparator>
#define GESTLSet(T) std::set<T, std::less<T>, GE::Core::STLAllocator<T>>
#define GESTLSetCustom(T, Comparator) std::set<T, Comparator, GE::Core::STLAllocator<T>>
#define GESTLMultiset(T) std::multiset<T, std::less<T>, GE::Core::STLAllocator<T>>
#define GESTLMultisetCustom(T, Comparator) std::multiset<T, Comparator, GE::Core::STLAllocator<T>>
#define GESTLMap(T, U) std::map<T, U, std::less<T>, GE::Core::STLAllocator<std::pair<const T, U>>>
#define GESTLMapCustom(T, U, Comparator) std::map<T, U, Comparator, GE::Core::STLAllocator<std::pair<const T, U>>>


//
//  Strings
//
#include <string>

#define GESTLString std::basic_string<char, std::char_traits<char>, GE::Core::STLAllocator<char>>
