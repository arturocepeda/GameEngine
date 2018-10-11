
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GERand.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Types/GESTLTypes.h"
#include <random>

namespace GE { namespace Core
{
   //
   //  Rand
   //
   class Rand
   {
   protected:
      static std::random_device cRandomDevice;
      static std::mt19937 cRandomEngine;

      std::uniform_real_distribution<float> cRandomDist01;
      std::uniform_real_distribution<float> cRandomDistM11;

      Rand();

      float getBetween0and1();
      float getBetweenMinus1and1();
   };



   //
   //  RandInt
   //
   class RandInt : public Rand
   {
   private:
      int iMin;
      int iMax;

   public:
      RandInt(int MinValue, int MaxValue);

      int generate();
   };



   //
   //  RandFloat
   //
   class RandFloat : public Rand
   {
   private:
      float fMin;
      float fMax;

   public:
      RandFloat(float MinValue, float MaxValue);

      float generate();
   };



   //
   //  RandEvent
   //
   class RandEvent : public Rand
   {
   private:
      float fProbability;

   public:
      RandEvent(float Probability);

      void setProbability(float Probability);
      bool occurs();
   };



   //
   //  RandDie
   //
   class RandDie : public Rand
   {
   private:
      std::uniform_int_distribution<int> cRandomDist;

   public:
      RandDie(int pMin = 1, int pMax = 6);

      int roll();
   };



   //
   //  RandUniform
   //
   template<typename T>
   class RandUniform : public Rand
   {
   protected:
      GESTLVector(T) mValues;

   public:
      void insert(T pValue)
      {
         mValues.push_back(pValue);
      }

      T extract()
      {
         std::uniform_int_distribution<int> randomDist(0, (int)mValues.size() - 1);
         return (mValues[randomDist(cRandomEngine)]);
      }

      void clear()
      {
         mValues.clear();
      }

      uint32_t getSize() const
      {
         return (uint32_t)mValues.size();
      }
   };



   //
   //  RandUrn
   //
   template<typename T>
   class RandUrn : public RandUniform<T>
   {
   public:
      T extract()
      {
         std::uniform_int_distribution<int> randomDist(0, (int)RandUniform<T>::mValues.size() - 1);

         const int position = randomDist(Rand::cRandomEngine);
         T value = RandUniform<T>::mValues[position];

         RandUniform<T>::mValues.erase(RandUniform<T>::mValues.begin() + position);

         return value;
      }
   };



   //
   //  RandExponential
   //
   class RandExponential : public Rand
   {
   private:
      float fLambda;

   public:
      RandExponential(float Lambda);

      float generate();
   };



   //
   //  RandNormal
   //
   class RandNormal : public Rand
   {
   private:
      float fMean;
      float fStandardDeviation;

      int iN;
      float fHalfN;
      float fScale;

   public:
      RandNormal(float Mean, float StandardDeviation);

      float generate();
   };
}}
