
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEInterpolator.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEInterpolator.h"
#include "GELog.h"

using namespace GE;
using namespace GE::Core;


//
//  CurvePropertyInterpolator
//
CurvePropertyInterpolator::CurvePropertyInterpolator(Curve* cCurve, Serializable* cSerializable,
   const ObjectName& cPropertyName, PropertyValueComponent ePropertyValueComponent)
   : Interpolator<float>(cCurve->getInterpolationMode())
   , fCurveLength(cCurve->getLength())
   , eValueComponent(ePropertyValueComponent)
{
   GEAssert(cCurve);
   GEAssert(cSerializable);

   // property validation
   const Property* cProperty = cSerializable->getProperty(cPropertyName);
   bool bPropertyValidated = true;

   if(!cProperty)
   {
      Log::log(LogType::Warning, "There is no '%s' property in the '%s' type",
         cPropertyName.getString(), cSerializable->getClassName().getString());
      bPropertyValidated = false;
   }
   else if(!cProperty->Setter)
   {
      Log::log(LogType::Warning, "The '%s' property is read-only", cPropertyName.getString());
      bPropertyValidated = false;
   }
   else
   {
      switch(eValueComponent)
      {
      case PropertyValueComponent::None:
         bPropertyValidated =
            cProperty->Type == ValueType::Float || cProperty->Type == ValueType::UInt || cProperty->Type == ValueType::Int;
         break;
      case PropertyValueComponent::X:
      case PropertyValueComponent::Y:
         bPropertyValidated = cProperty->Type == ValueType::Vector2 || cProperty->Type == ValueType::Vector3;
         break;
      case PropertyValueComponent::Z:
         bPropertyValidated = cProperty->Type == ValueType::Vector3;
         break;
      case PropertyValueComponent::Red:
      case PropertyValueComponent::Green:
      case PropertyValueComponent::Blue:
      case PropertyValueComponent::Alpha:
         bPropertyValidated = cProperty->Type == ValueType::Color;
         break;
      default:
         bPropertyValidated = false;
      }

      if(!bPropertyValidated)
      {
         Log::log(LogType::Warning, "The '%s' property does not contain the provided component (%s)",
            cPropertyName.getString(), strPropertyValueComponent[(int)eValueComponent]);
      }
   }

   // setter attachment
   if(bPropertyValidated)
   {
      Interpolator<float>::attachSetter([this, cCurve, cProperty](const float& t)
      {
         const ValueType eValueType = cProperty->Type;
         const float fCurveValue = cCurve->getValue(t);

         Value cPropertyValue = cProperty->Getter();

         if(eValueComponent == PropertyValueComponent::None)
         {
            if(eValueType == ValueType::Float)
            {
               cPropertyValue = Value(fCurveValue);
            }
            else if(eValueType == ValueType::UInt)
            {
               cPropertyValue = Value((uint)fCurveValue);
            }
            else if(eValueType == ValueType::Int)
            {
               cPropertyValue = Value((int)fCurveValue);
            }
         }
         else if(eValueComponent == PropertyValueComponent::X)
         {
            if(eValueType == ValueType::Vector2)
            {
               Vector2 v = cPropertyValue.getAsVector2();
               v.X = fCurveValue;
               cPropertyValue = Value(v);
            }
            else if(eValueType == ValueType::Vector3)
            {
               Vector3 v = cPropertyValue.getAsVector3();
               v.X = fCurveValue;
               cPropertyValue = Value(v);
            }
         }
         else if(eValueComponent == PropertyValueComponent::Y)
         {
            if(eValueType == ValueType::Vector2)
            {
               Vector2 v = cPropertyValue.getAsVector2();
               v.Y = fCurveValue;
               cPropertyValue = Value(v);
            }
            else if(eValueType == ValueType::Vector3)
            {
               Vector3 v = cPropertyValue.getAsVector3();
               v.Y = fCurveValue;
               cPropertyValue = Value(v);
            }
         }
         else if(eValueComponent == PropertyValueComponent::Z)
         {
            Vector3 v = cPropertyValue.getAsVector3();
            v.Z = fCurveValue;
            cPropertyValue = Value(v);
         }
         else if(eValueComponent == PropertyValueComponent::Red)
         {
            Color c = cPropertyValue.getAsColor();
            c.Red = fCurveValue;
            cPropertyValue = Value(c);
         }
         else if(eValueComponent == PropertyValueComponent::Green)
         {
            Color c = cPropertyValue.getAsColor();
            c.Green = fCurveValue;
            cPropertyValue = Value(c);
         }
         else if(eValueComponent == PropertyValueComponent::Blue)
         {
            Color c = cPropertyValue.getAsColor();
            c.Blue = fCurveValue;
            cPropertyValue = Value(c);
         }
         else if(eValueComponent == PropertyValueComponent::Alpha)
         {
            Color c = cPropertyValue.getAsColor();
            c.Alpha = fCurveValue;
            cPropertyValue = Value(c);
         }

         cProperty->Setter(cPropertyValue);
      });
   }
   else
   {
      Interpolator<float>::attachSetter([](const float& t)
      {
      });
   }
}

void CurvePropertyInterpolator::animate()
{
   Interpolator<float>::animate(0.0f, fCurveLength, fCurveLength, nullptr);
}

void CurvePropertyInterpolator::animateInverse()
{
   Interpolator<float>::animate(fCurveLength, 0.0f, fCurveLength, nullptr);
}


//
//  BezierPropertyInterpolator
//
BezierPropertyInterpolator::BezierPropertyInterpolator(BezierCurve* cBezierCurve,
   Serializable* cSerializable, const ObjectName& cPropertyName, InterpolationMode eMode)
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

void BezierPropertyInterpolator::animate(float fDuration)
{
   Interpolator<float>::animate(0.0f, 1.0f, fDuration, nullptr);
}

void BezierPropertyInterpolator::animateInverse(float fDuration)
{
   Interpolator<float>::animate(1.0f, 0.0f, fDuration, nullptr);
}
