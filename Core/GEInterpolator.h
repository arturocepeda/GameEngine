
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEInterpolator.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEConstants.h"
#include "GEMath.h"
#include "GETime.h"
#include "GESerializable.h"
#include "Types/GECurve.h"
#include "Types/GEBezierCurve.h"

#include <functional>

namespace GE { namespace Core
{
   template<typename T>
   class Interpolator
   {
   public:
      typedef std::function<T()> Getter;
      typedef std::function<void(const T& value)> Setter;

   private:
      InterpolationMode eInterpolationMode;
      Getter getter;
      Setter setter;

      T tStartValue;
      T tEndValue;
      float fDuration;

      enum class State
      {
         Inactive,
         Animating,
         AlternatingAscending,
         AlternatingDescending
      };

      State eState;
      uint iClockIndex;
      float fElapsedTime;

      bool bStopOnStartValue;
      bool bStopOnEndValue;

      std::function<void()> onFinished;

      void executeOnFinishedAction()
      {
         if(onFinished)
            onFinished();

         onFinished = nullptr;
      }

      void updateAnimatingState()
      {
         if(fElapsedTime >= fDuration)
         {
            setter(tEndValue);
            eState = State::Inactive;
            executeOnFinishedAction();
         }
         else
         {
            float fInterpolationFactor = Math::getInterpolationFactor(fElapsedTime / fDuration, eInterpolationMode);
            setter(Math::getInterpolatedValue(tStartValue, tEndValue, fInterpolationFactor));
         }
      }

      void updateAlternatingAscendingState()
      {
         if(fElapsedTime >= fDuration)
         {
            setter(tEndValue);

            if(bStopOnEndValue)
            {
               eState = State::Inactive;
               executeOnFinishedAction();
            }
            else
            {
               eState = State::AlternatingDescending;
               fElapsedTime = 0.0f;
            }
         }
         else
         {
            float fInterpolationFactor = Math::getInterpolationFactor(fElapsedTime / fDuration, eInterpolationMode);
            setter(Math::getInterpolatedValue(tStartValue, tEndValue, fInterpolationFactor));
         }
      }

      void updateAlternatingDescendingState()
      {
         if(fElapsedTime >= fDuration)
         {
            setter(tStartValue);

            if(bStopOnStartValue)
            {
               eState = State::Inactive;
               executeOnFinishedAction();
            }
            else
            {
               eState = State::AlternatingAscending;
               fElapsedTime = 0.0f;
            }
         }
         else
         {
            float fInterpolationFactor = Math::getInterpolationFactor(fElapsedTime / fDuration, eInterpolationMode);
            setter(Math::getInterpolatedValue(tEndValue, tStartValue, fInterpolationFactor));
         }
      }

   public:
      Interpolator(InterpolationMode Mode)
         : eInterpolationMode(Mode)
         , getter(nullptr)
         , setter(nullptr)
         , onFinished(nullptr)
         , iClockIndex(0)
      {
      }

      bool getGetter() const
      {
         return getter;
      }

      bool getSetter() const
      {
         return setter;
      }

      bool getActive() const
      {
         return eState != State::Inactive;
      }

      void attachSetter(Setter setter)
      {
         this->setter = setter;
      }

      void attachGetter(Getter getter)
      {
         this->getter = getter;
      }

      uint getClockIndex() const
      {
         return iClockIndex;
      }

      void setClockIndex(uint ClockIndex)
      {
         iClockIndex = ClockIndex;
      }

      void animate(const T& EndValue, float Duration, std::function<void()> onFinished = nullptr)
      {
         GEAssert(getter);
         GEAssert(setter);

         tStartValue = getter();
         tEndValue = EndValue;
         fDuration = Duration;
         this->onFinished = onFinished;

         eState = State::Animating;
         fElapsedTime = 0.0f;
      }

      void animate(const T& StartValue, const T& EndValue, float Duration, std::function<void()> onFinished = nullptr)
      {
         GEAssert(setter);

         tStartValue = StartValue;
         tEndValue = EndValue;
         fDuration = Duration;
         this->onFinished = onFinished;

         eState = State::Animating;
         fElapsedTime = 0.0f;
         setter(StartValue);
      }

      void alternate(const T& StartValue, const T& EndValue, float Duration)
      {
         GEAssert(setter);

         this->tStartValue = StartValue;
         this->tEndValue = EndValue;
         this->fDuration = Duration;

         eState = State::AlternatingAscending;
         fElapsedTime = 0.0f;
         bStopOnStartValue = false;
         bStopOnEndValue = false;
         setter(StartValue);
      }

      void stop()
      {
         eState = State::Inactive;
      }

      void stopAndSetStartValue()
      {
         if(eState != State::Inactive)
         {
            setter(tStartValue);
            eState = State::Inactive;
         }
      }

      void stopAndSetEndValue()
      {
         if(eState != State::Inactive)
         {
            setter(tEndValue);
            eState = State::Inactive;
         }
      }

      void stopOnStartValue(std::function<void()> onFinished = nullptr)
      {
         bStopOnStartValue = true;
         this->onFinished = onFinished;
      }

      void stopOnEndValue(std::function<void()> onFinished = nullptr)
      {
         bStopOnEndValue = true;
         this->onFinished = onFinished;
      }

      float getRemainingTime() const
      {
         return getActive() ? (fDuration - fElapsedTime) : 0.0f;
      }

      void update()
      {
         if(eState == State::Inactive)
            return;

         fElapsedTime += Time::getClock(iClockIndex).getDelta();

         switch(eState)
         {
         case State::Inactive:
            break;
         case State::Animating:
            updateAnimatingState();
            break;
         case State::AlternatingAscending:
            updateAlternatingAscendingState();
            break;
         case State::AlternatingDescending:
            updateAlternatingDescendingState();
            break;
         }
      }
   };


   template<typename T>
   class PropertyInterpolator : public Interpolator<T>
   {
   public:
      PropertyInterpolator(Serializable* cSerializable, const ObjectName& cPropertyName, InterpolationMode eMode)
         : Interpolator<T>(eMode)
      {
         GEAssert(cSerializable);

         const Property* cProperty = cSerializable->getProperty(cPropertyName);
         GEAssert(cProperty);
         GEAssert(cProperty->Getter);
         GEAssert(cProperty->Setter);

         Interpolator<T>::attachGetter([cProperty]() -> T
         {
            Value cValue = cProperty->Getter();
            return cValue.getAs<T>();
         });
         Interpolator<T>::attachSetter([cProperty](const T& v)
         {
            Value cValue = Value(v);
            cProperty->Setter(cValue);
         });
      }
      
      void animate(const T& tValue, float fDuration)
      {
         Interpolator<T>::animate(tValue, fDuration, nullptr);
      }
   };


   class CurvePropertyInterpolator : public Interpolator<float>
   {
   private:
      float fCurveLength;

   public:
      CurvePropertyInterpolator(Curve* cCurve, Serializable* cSerializable, const ObjectName& cPropertyName)
         : Interpolator<float>(cCurve->getInterpolationMode())
         , fCurveLength(cCurve->getLength())
      {
         GEAssert(cCurve);
         GEAssert(cSerializable);

         const Property* cProperty = cSerializable->getProperty(cPropertyName);
         GEAssert(cProperty);
         GEAssert(cProperty->Type == ValueType::Vector3);
         GEAssert(cProperty->Setter);

         Interpolator<float>::attachSetter([cCurve, cProperty](const float& t)
         {
            Value cValue = cCurve->getValue(t);
            cProperty->Setter(cValue);
         });
      }

      void animate()
      {
         Interpolator<float>::animate(0.0f, fCurveLength, fCurveLength, nullptr);
      }
   };


   class BezierPropertyInterpolator : public Interpolator<float>
   {
   public:
      BezierPropertyInterpolator(BezierCurve* cBezierCurve, Serializable* cSerializable, const ObjectName& cPropertyName, InterpolationMode eMode)
         : Interpolator<float>(eMode)
      {
         GEAssert(cBezierCurve);
         GEAssert(cSerializable);

         const Property* cProperty = cSerializable->getProperty(cPropertyName);
         GEAssert(cProperty);
         GEAssert(cProperty->Type == ValueType::Vector3);
         GEAssert(cProperty->Setter);

         Interpolator<float>::attachSetter([cBezierCurve, cProperty](const float& v)
         {
            Value cValue = Value(cBezierCurve->getPoint(v));
            cProperty->Setter(cValue);
         });
      }

      void animate(float fDuration)
      {
         Interpolator<float>::animate(0.0f, 1.0f, fDuration, nullptr);
      }
   };
}}
