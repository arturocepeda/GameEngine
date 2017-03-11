
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
      RandDie();
      int roll();
   };



   //
   //  RandUniform
   //
   class RandUniform : public Rand
   {
   protected:
      GESTLVector(int) iValues;

   public:
      void addNumber(int Number);
      int extractNumber();
      uint getSize();
   };



   //
   //  RandUrn
   //
   class RandUrn : public RandUniform
   {
   public:
      int extractNumber();
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
