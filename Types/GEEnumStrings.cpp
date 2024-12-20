
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda P�rez
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
      "Cubic",
      "CubicInverse",
      "Quartic",
      "QuarticInverse",
      "Quintic",
      "QuinticInverse",
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
      "UShort",
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
      "English",
      "Spanish",
      "German",
      "French",
      "Italian",
      "Portuguese",
      "Russian",
      "ChineseSimplified",
      "ChineseTraditional",
      "Japanese",
      "Korean"
   };
}}

namespace GE { namespace Entities
{
   const char* strSpriteLayer[] =
   {
      "GUI",
      "Pre3D",
      "PostGUI"
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

   const char* strSizeMode[] =
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

   const char* strOffsetMode[] =
   {
      "Absolute",
      "Relative"
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

   const char* strParticleType[] =
   {
      "Billboard",
      "Mesh",
      "TextBillboard",
      "Text3D"
   };

   const char* strParticleSystemSettingsBitMask[] =
   {
      "Prewarm",
      "DynamicShadows",
      "LocalSpace"
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
      "ThreadSafe"
   };

   const char* strCanvasSettingsBitMask[] =
   {
      "RenderAfter2DElements"
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

   const char* strRenderPass[] =
   {
      "None",
      "01_Pre3D",
      "02_OpaqueMeshes",
      "03_Labels3D",
      "04_TransparentMeshes",
      "05_UI3DFirst",
      "06_UI2D",
      "07_UI3DSecond",
      "08_PostUI",
      "09_DebugGeometry"
   };
}}

namespace GE { namespace Audio
{
   const char* strAudioBankType[] =
   {
      "Buffered",
      "Streamed"
   };

   const char* strAudioBankState[] =
   {
      "Unloaded",
      "Loading",
      "Loaded"
   };

   const char* strAudioEventPlayMode[] =
   {
      "OneShot",
      "Loop"
   };
}}
