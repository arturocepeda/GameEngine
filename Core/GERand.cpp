
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GERand.cpp ---
//
//////////////////////////////////////////////////////////////////


#include "GERand.h"

std::random_device cRandomDevice;
std::mt19937 cRandomEngine(cRandomDevice());

using namespace GE;
using namespace GE::Core;

//
//  Rand
//
Rand::Rand()
   : cRandomDist01(0.0f, 1.0f)
   , cRandomDistM11(-1.0f, 1.0f)
{
}

float Rand::getBetween0and1()
{
   return cRandomDist01(cRandomEngine);
}

float Rand::getBetweenMinus1and1()
{
   return cRandomDistM11(cRandomEngine);
}



//
//  RandInt
//
RandInt::RandInt(int MinValue, int MaxValue)
{
   if(MinValue < MaxValue)
   {
      iMin = MinValue;
      iMax = MaxValue;
   }
   else
   {
      iMin = MaxValue;
      iMax = MinValue;
   }
}

int RandInt::generate()
{
   std::uniform_int_distribution<int> cRandomDist(iMin, iMax);
   return cRandomDist(cRandomEngine);
}



//
//  RandFloat
//
RandFloat::RandFloat(float MinValue, float MaxValue)
{
   if(MinValue < MaxValue)
   {
      fMin = MinValue;
      fMax = MaxValue;
   }
   else
   {
      fMin = MaxValue;
      fMax = MinValue;
   }
}

float RandFloat::generate()
{
   return fMin + (getBetween0and1() * (fMax - fMin));
}



//
//  RandEvent
//
RandEvent::RandEvent(float Probability)
   : fProbability(Probability)
{
}

void RandEvent::setProbability(float Probability)
{
   fProbability = Probability;
}

bool RandEvent::occurs()
{
   return getBetween0and1() <= fProbability;
}



//
//  RandDie
//
RandDie::RandDie()
   : cRandomDist(1, 6)
{
}

int RandDie::roll()
{
   return cRandomDist(cRandomEngine);
}



//
//  RandUniform
//
void RandUniform::addNumber(int Number)
{
   iValues.push_back(Number);
}

int RandUniform::extractNumber()
{
   std::uniform_int_distribution<int> cRandomDist(0, (int)iValues.size() - 1);
   return (iValues[cRandomDist(cRandomEngine)]);
}

uint RandUniform::getSize()
{
   return (uint)iValues.size();
}



//
//  RandUrn
//
int RandUrn::extractNumber()
{
   std::uniform_int_distribution<int> cRandomDist(0, (int)iValues.size() - 1);
   int iPosition = cRandomDist(cRandomEngine);
   int iValue = iValues[iPosition];

   iValues.erase(iValues.begin() + iPosition);

   return iValue;
}



//
//  RandExponential
//
RandExponential::RandExponential(float Lambda)
   : fLambda(Lambda)
{
}

float RandExponential::generate()
{
    float fValue = 0.0f;

    while(fValue == 0.0f)
        fValue = getBetween0and1();

    return (-log(fValue) / fLambda);
}



//
//  RandNormal
//
RandNormal::RandNormal(float Mean, float StandardDeviation)
   : fMean(Mean)
   , fStandardDeviation(StandardDeviation)
   , iN(12)
   , fHalfN(6.0f)
   , fScale(1.0f)
{
}

float RandNormal::generate()
{
    float fSum = 0.0f;

    for(int i = 0; i < iN; i++)
        fSum += getBetween0and1();

    return (fStandardDeviation * fScale * (fSum - fHalfN) + fMean);
}
