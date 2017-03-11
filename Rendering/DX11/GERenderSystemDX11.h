
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Rendering Engine (DirectX 11)
//
//  --- GERenderSystemDX11.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Core/GEPlatform.h"

#if defined(GE_PLATFORM_WP8)
# include <agile.h>
#else
# include <windows.h>
# include <wrl.h>
#endif

#include "Rendering/GERenderSystem.h"
#include "Rendering/GERenderingObjects.h"
#include "GERenderingShadersDX11.h"
#include "GERenderTextureDX11.h"
#include <vector>

namespace GE { namespace Rendering
{
   struct ShaderConstantsTransform
   {
      GE::Matrix4 WorldViewProjectionMatrix;    // 64 bytes
      GE::Matrix4 WorldMatrix;                  // 128 bytes
      GE::Matrix4 InverseTransposeWorldMatrix;  // 192 bytes
      GE::Matrix4 LightWorldViewProjection;     // 256 bytes
   };

   struct ShaderConstantsMaterial
   {
      GE::Color DiffuseColor;       // 16 bytes
      GE::Color SpecularColor;      // 32 bytes
   };

   struct ShaderConstantsLighting
   {
      GE::Color AmbientLightColor;     // 16 bytes
      GE::Color LightColor;            // 32 bytes
      GE::Vector3 EyePosition;
      int LightType;                   // 48 bytes
      GE::Vector3 LightPosition;
      float Attenuation;               // 64 bytes
      GE::Vector3 LightDirection;
      float SpotAngle;                 // 80 bytes
   };


   class RenderSystemDX11 : public RenderSystem
   {
   private:
      void createBuffers();
      void createStates();

      void releaseShaders();
      void releaseStates();
      void releaseBuffers();

   public:
#if defined(GE_PLATFORM_WP8)
      RenderSystemDX11(Windows::UI::Core::CoreWindow^ window);
#else
      RenderSystemDX11(HWND WindowHandle, bool Windowed);
#endif
      ~RenderSystemDX11();
   };


   class RenderSystemDX11Helper
   {
   public:
      static void createDeviceResources();
      static void updateForWindowSizeChange();
      static void handleDeviceLost();
      static void createWindowSizeDependentResources();
      static void releaseResourcesForSuspending();
   };
}}
