
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
#include <functional>

namespace GE { namespace Core
{
   enum class InterpolationMode
   {
      Linear,
      Quadratic,
      Logarithmic
   };


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

      float getInterpolatedValue(float fStartValue, float fEndValue, float fFactor)
      {
         return Math::lerp(fStartValue, fEndValue, fFactor);
      }
      Vector2 getInterpolatedValue(const Vector2& vStartValue, const Vector2& vEndValue, float fFactor)
      {
         return Vector2::lerp(vStartValue, vEndValue, fFactor);
      }
      Vector3 getInterpolatedValue(const Vector3& vStartValue, const Vector3& vEndValue, float fFactor)
      {
         return Vector3::lerp(vStartValue, vEndValue, fFactor);
      }
      Color getInterpolatedValue(const Color& sStartValue, const Color& sEndValue, float fFactor)
      {
         return Color::lerp(sStartValue, sEndValue, fFactor);
      }

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
            float fInterpolationFactor = getInterpolationFactor(fElapsedTime / fDuration);
            setter(getInterpolatedValue(tStartValue, tEndValue, fInterpolationFactor));
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
            float fInterpolationFactor = getInterpolationFactor(fElapsedTime / fDuration);
            setter(getInterpolatedValue(tStartValue, tEndValue, fInterpolationFactor));
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
            float fInterpolationFactor = getInterpolationFactor(fElapsedTime / fDuration);
            setter(getInterpolatedValue(tEndValue, tStartValue, fInterpolationFactor));
         }
      }

      float getInterpolationFactor(float t)
      {
         switch(eInterpolationMode)
         {
         case InterpolationMode::Linear:
            {
               return t;
            }
         case InterpolationMode::Quadratic:
            {
               return t * t;
            }
         case InterpolationMode::Logarithmic:
            {
               //
               //  log(1) = 0 and log(e) = 1, so we need to map [0, 1] to [1, e] and then calculate the logarithm
               //
               float fMappedT = 1.0f + t * (GE_E - 1.0f);
               return log(fMappedT);
            }
         }

         return t;
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
}}
