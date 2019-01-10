
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Types
//
//  --- GEEnumStrings.cpp ---
//
//////////////////////////////////////////////////////////////////

namespace GE
{
   const char* strInterpolationMode[] =
   {
      "Linear",
      "Quadratic",
      "QuadraticInverse",
      "Logarithmic"
   };

   const char* strAlignment[] =
   {
      "None",
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

   const char* strCurveValueType[] =
   {
      "Default",
      "EulerAngleInRadians",
      "EulerAngleInDegrees"
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
      "STL",
      "Scripting"
   };

   const char* strPropertyValueComponent[] =
   {
      "None",
      "X",
      "Y",
      "Z",
      "Red",
      "Green",
      "Blue",
      "Alpha"
   };

   const char* strSystemLanguage[] =
   {
      "en", // English
      "es", // Spanish
      "de", // German
   };
}}

namespace GE { namespace Entities
{
   const char* strSpriteLayer[] =
   {
      "GUI",
      "Pre3D"
   };

   const char* strUVMode[] =
   {
      "Normal",
      "FlipU",
      "FlipV",
      "FlipUV"
   };

   const char* strCenterMode[] =
   {
      "Absolute",
      "Relative"
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

   const char* strParticleSystemSettingsBitMask[] =
   {
      "Prewarm",
      "DynamicShadows"
   };

   const char* strParticleEmitterType[] =
   {
      "Point",
      "Sphere",
      "SphereSurface",
      "Line",
      "Mesh"
   };

   const char* strValueProviderType[] =
   {
      "Constant",
      "Random",
      "Curve"
   };

   const char* strMeshSettingsBitMask[] =
   {
      "Transparency",
      "Skinning"
   };

   const char* strLabelSettingsBitMask[] =
   {
      "Justify",
      "VariableReplacement",
      "RichTextSupport",
      "FitSizeToLineWidth"
   };

   const char* strScriptSettingsBitMask[] =
   {
      "Active",
      "ThreadSafe",
      "SharedEnvironment"
   };

   const char* strCanvasSettingsBitMask[] =
   {
      "RenderAfterTransparentGeometry"
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

   const char* strVertexElementsBitMask[] =
   {
      "Position",
      "Color",
      "Normal",
      "Tangent",
      "Binormal",
      "TextureCoordinate"
   };

   const char* strDynamicShadowsBitMask[] =
   {
      "Cast",
      "Receive"
   };

   const char* strMaterialFlagsBitMask[] =
   {
      "RenderOncePerLight",
      "BatchRendering"
   };

   const char* strTextureSettingsBitMask[] =
   {
      "AtlasUV"
   };

   const char* strTextureWrapMode[] =
   {
      "Clamp",
      "Repeat"
   };
}}

namespace GE { namespace Audio
{
   const char* strAudioEventPlayMode[] =
   {
      "OneShot",
      "Loop"
   };
}}
