
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GESingleton.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEUtils.h"

namespace GE { namespace Core
{
   template<typename T>
   class Singleton : private NonCopyable
   {
   protected:
      static T* cInstance;

      Singleton()
      {
         GEAssert(!cInstance);
         cInstance = (T*)this;
      }

   public:
      static T* getInstance()
      {
         return cInstance;
      }
   };

   template<typename T>
   T* Singleton<T>::cInstance = 0;
}}
