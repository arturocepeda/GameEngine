
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Content
//
//  --- GESkeleton.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEResource.h"

namespace GE { namespace Content
{
   class Bone : public Core::Object
   {
   private:
      uint iIndex;
      uint iParentIndex;
      GESTLVector(uint) vChildren;

      Matrix4 mBindMatrix;
      Matrix4 mInverseBindMatrix;

      float fSize;

   public:
      Bone(uint Index, const Core::ObjectName& Name);
      ~Bone();

      uint getIndex() const;

      uint getParentIndex() const;
      void setParentIndex(uint ParentIndex);

      uint getChildrenCount() const;
      uint getChild(uint Index) const;
      void addChild(uint ChildIndex);

      const Matrix4& getBindMatrix() const;
      const Matrix4& getInverseBindMatrix() const;
      void setBindMatrix(const Matrix4& BindMatrix);

      float getSize() const;
      void setSize(float Size);
   };


   class Skeleton : public Resource
   {
   private:
      GESTLVector(Bone*) vBones;

   public:
      static const ResourceType Type;

      Skeleton(const char* FileName);
      ~Skeleton();

      uint getBonesCount() const;
      const Bone* getBone(uint Index) const;
   };
}}
