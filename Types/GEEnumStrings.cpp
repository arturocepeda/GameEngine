
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Types
//
//  --- GEEnumString.cpp ---
//
//////////////////////////////////////////////////////////////////

namespace GE
{
   const char* strAlignment[] =
   {
      "TopLeft",
      "TopCenter",
      "TopRight",
      "MiddleLeft",
      "MiddleCenter",
      "MiddleRight",
      "BottomLeft",
      "BottomCenter",
      "BottomRight"
   };
}

namespace GE { namespace Core
{
   const char* strValueType[] =
   {
      "Int",
      "UInt",
      "Float",
      "Bool",
      "Byte",
      "Short",
      "String",
      "ObjectName",
      "Vector2",
      "Vector3",
      "Color"
   };

   const char* strAllocationCategory[] =
   {
      "General",
      "STL"
   };

   const char* strSystemLanguage[] =
   {
      "en", // English
      "es", // Spanish
      "de", // German
   };
}}

namespace GE { namespace Content
{
   const char* strResourceType[] =
   {
      "ShaderProgram",
      "Texture",
      "Material",
      "Font",
      "Mesh",
      "Skeleton",
      "AnimationSet",
   };
}}

namespace GE { namespace Entities
{
   const char* strEntitySaveBehavior[] =
   {
      "Save",
      "DoNotSave"
   };

   const char* strUVMode[] =
   {
      "Normal",
      "FlipU",
      "FlipV",
      "FlipUV"
   };

   const char* strFullScreenSizeMode[] =
   {
      "None",
      "Stretch",
      "MatchWidth",
      "MatchHeight"
   };

   const char* strLightType[] =
   {
      "Directional",
      "Point",
      "Spot"
   };

   const char* strSceneBackgroundMode[] =
   {
      "SolidColor",
      "SkyBox"
   };

   const char* strParticleEmitterType[] =
   {
      "Point",
      "Sphere",
      "SphereSurface",
      "Line",
      "Mesh"
   };
}}

namespace GE { namespace Rendering
{
   const char* strGeometryType[] =
   {
      "Static",
      "Dynamic"
   };

   const char* strRenderingMode[] =
   {
      "2D",
      "3D"
   };

   const char* strBlendingMode[] =
   {
      "None",
      "Alpha",
      "Additive"
   };

   const char* strDepthBufferMode[] =
   {
      "NoDepth",
      "TestOnly",
      "TestAndWrite"
   };

   const char* strCullingMode[] =
   {
      "Back",
      "Front",
      "None"
   };

   const char* strDynamicShadowsBitMask[] =
   {
      "Cast",
      "Receive"
   };
}}
