
#include "main.h"
#include "Core/GEUtils.h"
#include "Core/GEObjectManager.h"
#include "Core/GEApplication.h"
#include "Rendering/GEMaterial.h"

#include <map>
#include <iostream>
#include <fstream>
#include <cassert>

#pragma comment(lib, "./../GameEngine.DX11.lib")
#pragma comment(lib, "./../pugixml.Windows.lib")
#pragma comment(lib, "./../stb.Windows.lib")

#if defined (_M_X64)
# pragma comment(lib, "./../../Externals/Brofiler/ProfilerCore64.lib")
#else
# pragma comment(lib, "./../../Externals/Brofiler/ProfilerCore32.lib")
#endif

using namespace GE;
using namespace GE::Core;
using namespace GE::Content;
using namespace GE::Entities;
using namespace GE::Rendering;
using namespace pugi;

char sSourceFileName[64];
int iMeshCount;

FbxImporter* fbxImporter = 0;

xml_document xmlPrefab;
xml_document xmlMaterials;
xml_node* xmlMaterialsRoot;

bool bSkinningData = false;
bool bUseTexture = false;
bool bSplitPoints = false;

std::map<uint, SubMesh> mSubMeshes;
ObjectManager<ImportBone> cSkeletonBones;
ObjectManager<Animation> cAnimations;

ObjectManager<ShaderProgram> mManagerShaderPrograms;
ObjectManager<Material> mManagerMaterials;
ObjectManager<Texture> mManagerTextures;

const char Zero = 0;

int main(int argc, char* argv[])
{
   std::cout << "\n Game Engine\n Arturo Cepeda\n Tools\n\n";

   if(argc < 2)
   {
      std::cout << " Usage: fbx2mesh <filename_without_extension> [-split]\n\n";
      return 0;
   }

   // input filename
   int iCharIndex = strlen(argv[1]);

   for(; iCharIndex > 0; iCharIndex--)
   {
      if(argv[1][iCharIndex - 1] == '\\' || argv[1][iCharIndex - 1] == '/')
         break;
   }

   strcpy(sSourceFileName, argv[1] + iCharIndex);

   char sSourceFullFileName[64];
   sprintf(sSourceFullFileName, "%s.fbx", argv[1]);

   // split points?
   bSplitPoints = argc > 2 && strcmp(argv[2], "-split") == 0;

   // create FBXSDK objects
   FbxManager* fbxManager = FbxManager::Create();
   FbxIOSettings* fbxSettings = FbxIOSettings::Create(fbxManager, IOSROOT);
   fbxManager->SetIOSettings(fbxSettings);
   FbxScene* fbxScene = FbxScene::Create(fbxManager, "FBX Scene");
   fbxImporter = FbxImporter::Create(fbxManager, "FBX Importer");

   // import FBX file
   int iFileFormat;

   if(!fbxManager->GetIOPluginRegistry()->DetectReaderFileFormat(sSourceFullFileName, iFileFormat))
      iFileFormat = fbxManager->GetIOPluginRegistry()->FindReaderIDByDescription("FBX binary (*.fbx)");

   bool bImportStatus = fbxImporter->Initialize(sSourceFullFileName, iFileFormat, fbxManager->GetIOSettings());

   if(!bImportStatus)
   {
      std::cout << " ERROR: FbxImporter could not be initialized\n\n";
      fbxImporter->Destroy();
      fbxScene->Destroy();
      fbxSettings->Destroy();
      fbxManager->Destroy();
      exit(1);
   }

   std::cout << " Importing scene...\n";
   bImportStatus = fbxImporter->Import(fbxScene);

   if(!bImportStatus)
   {
      std::cout << " ERROR: the data from the FBX file could not be imported\n\n";
      fbxImporter->Destroy();
      fbxScene->Destroy();
      fbxSettings->Destroy();
      fbxManager->Destroy();
      exit(1);
   }

   Application::startUp();

   registerObjectManagers();

   // set axis system
   std::cout << " Setting axis system...\n";
   FbxAxisSystem fbxAxisSystem(
      FbxAxisSystem::EUpVector::eYAxis,
      FbxAxisSystem::EFrontVector::eParityEven,
      FbxAxisSystem::eRightHanded);
   fbxAxisSystem.ConvertScene(fbxScene);

   // triangulate the scene
   std::cout << " Triangulating scene...\n";

   FbxGeometryConverter fbxGeometryConverter(fbxManager);
   fbxGeometryConverter.Triangulate(fbxScene, true);

   // load mesh data
   iMeshCount = 0;
   xml_node xmlPrefabRoot = xmlPrefab.append_child("Prefab");
   xml_node xmlMaterialsRootInstance = xmlMaterials.append_child("Materials");
   xmlMaterialsRoot = &xmlMaterialsRootInstance;

   importSkeletonData(fbxScene->GetRootNode());
   importMeshData(fbxScene->GetRootNode());

   // load animations
   importAnimations(fbxScene, fbxScene->GetRootNode());

   // generate mesh files
   char sSubMeshName[64];
   uint iSubMeshIndex = 1;
   std::map<uint, SubMesh>::const_iterator it = mSubMeshes.begin();

   xml_node xmlPrefabTransform = xmlPrefabRoot.append_child("Component");
   xmlPrefabTransform.append_attribute("type").set_value("Transform");

   for(; it != mSubMeshes.end(); it++, iSubMeshIndex++)
   {
      const SubMesh& sSubMesh = it->second;

      if(mSubMeshes.size() > 1)
         sprintf(sSubMeshName, "%s_%03d", sSourceFileName, iSubMeshIndex);
      else
         strcpy(sSubMeshName, sSourceFileName);

      xml_node xmlSubMeshEntity = xmlPrefabRoot.append_child("Entity");
      xmlSubMeshEntity.append_attribute("name").set_value(sSubMeshName);

      xml_node xmlSubMeshEntityTransform = xmlSubMeshEntity.append_child("Component");
      xmlSubMeshEntityTransform.append_attribute("type").set_value("Transform");

      xml_node xmlSubMeshEntityMesh = xmlSubMeshEntity.append_child("Component");
      xmlSubMeshEntityMesh.append_attribute("type").set_value("Mesh");
      xml_node xmlSubMeshEntityMeshProperty = xmlSubMeshEntityMesh.append_child("Property");
      xmlSubMeshEntityMeshProperty.append_attribute("name").set_value("Mesh");
      xmlSubMeshEntityMeshProperty.append_attribute("value").set_value(sSubMeshName);
      xmlSubMeshEntityMeshProperty = xmlSubMeshEntityMesh.append_child("Property");
      xmlSubMeshEntityMeshProperty.append_attribute("name").set_value("MaterialName");
      xmlSubMeshEntityMeshProperty.append_attribute("value").set_value(sSubMesh.MaterialName);

      generateMeshFile(sSubMesh, sSubMeshName);
   }

   // generate animation files
   cAnimations.iterate([](Animation* cAnimation)
   {
      generateAnimationFile(cAnimation);
      return true;
   });

   // generate XML files
   char sDestinyFileName[128];
   sprintf(sDestinyFileName, "%s.prefab.xml", sSourceFileName);
   xmlPrefab.save_file(sDestinyFileName, "  ");

   sprintf(sDestinyFileName, "%s.materials.xml", sSourceFileName);
   xmlMaterials.save_file(sDestinyFileName, "  ");

   if(bSkinningData)
   {
      ImportBone** cBones = new ImportBone*[cSkeletonBones.count()];

      xml_document xmlSkeleton;
      xml_node xmlSkeletonRoot = xmlSkeleton.append_child("Skeleton");
      xmlSkeletonRoot.append_attribute("bonesCount").set_value(cSkeletonBones.count());

      cSkeletonBones.iterate([&cBones, &xmlSkeletonRoot](ImportBone* cBone)
      {
         cBones[cBone->getIndex()] = cBone;

         if(cBone->ParentName == ObjectName::Empty)
            xmlSkeletonRoot.append_attribute("rootIndex").set_value(cBone->getIndex());

         return true;
      });

      for(uint i = 0; i < cSkeletonBones.count(); i++)
      {
         ImportBone* cBone = cBones[i];

         xml_node xmlBone = xmlSkeletonRoot.append_child("Bone");
         xmlBone.append_attribute("index").set_value(cBone->getIndex());
         xmlBone.append_attribute("name").set_value(cBone->getName().getString().c_str());

         if(cBone->ParentName != ObjectName::Empty)
         {
            Bone* cParent = cSkeletonBones.get(cBone->ParentName);
            xmlBone.append_attribute("parentIndex").set_value(cParent->getIndex());
            xmlBone.append_attribute("parentName").set_value(cParent->getName().getString().c_str());
         }

         char sVector3Str[64];
         Parser::writeVector3(getTranslation(cBone->fbxLinkTransform.GetT()), sVector3Str);
         xmlBone.append_attribute("bindT").set_value(sVector3Str);
         Parser::writeVector3(getRotation(cBone->fbxLinkTransform.GetR()), sVector3Str);
         xmlBone.append_attribute("bindR").set_value(sVector3Str);
         Parser::writeVector3(getScale(cBone->fbxLinkTransform.GetS()), sVector3Str);
         xmlBone.append_attribute("bindS").set_value(sVector3Str);
         xmlBone.append_attribute("size").set_value(cBone->getSize());

         xml_node xmlBoneChildren = xmlBone.append_child("Children");

         for(uint i = 0; i < cBone->getChildrenCount(); i++)
         {
            Bone* cChild = cBones[cBone->getChild(i)];
            xml_node xmlBoneChild = xmlBoneChildren.append_child("Child");
            xmlBoneChild.append_attribute("index").set_value(cChild->getIndex());
            xmlBoneChild.append_attribute("name").set_value(cChild->getName().getString().c_str());
         }
      }

      delete[] cBones;

      sprintf(sDestinyFileName, "%s.skeleton.xml", sSourceFileName);
      xmlSkeleton.save_file(sDestinyFileName, "  ");
   }

   Application::shutDown();

   return 0;
}

void registerObjectManagers()
{
   ObjectManagers::getInstance()->registerObjectManager<ShaderProgram>("ShaderProgram", &mManagerShaderPrograms);
   ObjectManagers::getInstance()->registerObjectManager<Material>("Material", &mManagerMaterials);
   ObjectManagers::getInstance()->registerObjectManager<Texture>("Texture", &mManagerTextures);
}

void importMeshData(FbxNode* fbxNode)
{
   FbxMesh* fbxMesh = fbxNode->GetMesh();

   if(fbxMesh)
   {
      std::cout << " Importing mesh '" << fbxMesh->GetName() << "'...\n";

      // import all the materials in the node
      importMaterials(fbxNode);

      // add a mesh entry in the prefab definition file
      char sMeshName[64];

      if(strlen(fbxMesh->GetName()) > 0)
         sprintf(sMeshName, "%s_%s", sSourceFileName, fbxMesh->GetName());
      else
         sprintf(sMeshName, "%s_%03d", sSourceFileName, iMeshCount);

      iMeshCount++;

      for(int i = 0; i < fbxNode->GetMaterialCount(); i++)
         importGeometry(fbxNode, sMeshName, i);
   }

   for(int i = 0; i < fbxNode->GetChildCount(); i++)
      importMeshData(fbxNode->GetChild(i));
}

void importSkeletonData(FbxNode* fbxNode)
{
   importBones(fbxNode);
   bSkinningData = cSkeletonBones.count() > 0;

   cSkeletonBones.iterate([](ImportBone* cBone)
   {
      if(cBone->ParentName != ObjectName::Empty)
      {
         Bone* cParent = cSkeletonBones.get(cBone->ParentName);
         cParent->addChild(cBone->getIndex());
      }

      return true;
   });
}

void importBones(FbxNode* fbxNode)
{
   if(fbxNode->GetNodeAttribute() &&
      fbxNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
   {
      ObjectName cBoneName = ObjectName(fbxNode->GetName());
      ImportBone* cBone = cSkeletonBones.get(cBoneName);

      if(!cBone)
      {
         cBone = new ImportBone(cSkeletonBones.count(), cBoneName);
         cBone->fbxNode = fbxNode;
         FbxSkeleton* fbxSkeleton = (FbxSkeleton*)fbxNode->GetNodeAttribute();

         if(fbxSkeleton->GetSkeletonType() == FbxSkeleton::eLimbNode)
            cBone->setSize((float)fbxSkeleton->Size.Get());

         FbxNode* fbxParentNode = fbxNode->GetParent();

         if(fbxParentNode && fbxParentNode->GetNodeAttribute() &&
            fbxParentNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
         {
            cBone->ParentName = ObjectName(fbxParentNode->GetName());
         }

         cSkeletonBones.add(cBone);
      }
   }

   for(int i = 0; i < fbxNode->GetChildCount(); i++)
      importBones(fbxNode->GetChild(i));
}

void importMaterials(FbxNode* fbxNode)
{
   int iMaterialCount = fbxNode->GetMaterialCount();

   for(int iMaterialIndex = 0; iMaterialIndex < iMaterialCount; iMaterialIndex++)
   {
      FbxSurfaceMaterial* fbxMaterial = fbxNode->GetMaterial(iMaterialIndex);
      const char* sMaterialName = fbxMaterial->GetName();
      bool bMaterialRegistered = false;

      for(xml_node_iterator it = xmlMaterialsRoot->begin(); it != xmlMaterialsRoot->end(); it++)
      {
         xml_node node = *it;

         if(strcmp(node.attribute("name").value(), sMaterialName) == 0)
         {
            bMaterialRegistered = true;
            break;
         }
      }

      if(bMaterialRegistered)
         continue;

      Material cMaterial(sMaterialName);

      xml_node xmlMaterial = xmlMaterialsRoot->append_child("Material");
      xml_attribute xmlMaterialName = xmlMaterial.append_attribute("name");
      xmlMaterialName.set_value(sMaterialName);

      const FbxImplementation* fbxImplementation = GetImplementation(fbxMaterial, FBXSDK_IMPLEMENTATION_HLSL);

      if(!fbxImplementation)
         fbxImplementation = GetImplementation(fbxMaterial, FBXSDK_IMPLEMENTATION_CGFX);

      if(!fbxImplementation)
      {
         if(fbxMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
         {
            FbxSurfacePhong* fbxSurfacePhong = static_cast<FbxSurfacePhong*>(fbxMaterial);
            FbxDouble3 fDiffuse = fbxSurfacePhong->Diffuse;
            FbxDouble fDiffuseFactor = fbxSurfacePhong->DiffuseFactor;
            FbxDouble fTransparencyFactor = fbxSurfacePhong->TransparencyFactor;

            Color cDiffuseColor = Color(
               (float)(fDiffuse[0] * fDiffuseFactor),
               (float)(fDiffuse[1] * fDiffuseFactor),
               (float)(fDiffuse[2] * fDiffuseFactor),
               (float)(1.0 - fTransparencyFactor));
            cMaterial.setDiffuseColor(cDiffuseColor);
         }
      }

      FbxProperty lProperty = fbxMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);

      int lLayeredTextureCount = lProperty.GetSrcObjectCount<FbxTexture>();

      if(lLayeredTextureCount > 0)
      {
         cMaterial.setShaderProgram(ObjectName("MeshTextureUnlit"));

         for(int j = 0; j < lLayeredTextureCount; ++j)
         {
            FbxTexture* fbxTexture = lProperty.GetSrcObject<FbxTexture>(j);
            cMaterial.setDiffuseTextureName(ObjectName(fbxTexture->GetName()));
         }
      }
      else
      {
         cMaterial.setShaderProgram(ObjectName("MeshColorUnlit"));
      }

      cMaterial.saveToXml(xmlMaterial);
   }
}

FbxAMatrix getGeometry(FbxNode* fbxNode)
{
   const FbxVector4 T = fbxNode->GetGeometricTranslation(FbxNode::eSourcePivot);
   const FbxVector4 R = fbxNode->GetGeometricRotation(FbxNode::eSourcePivot);
   const FbxVector4 S = fbxNode->GetGeometricScaling(FbxNode::eSourcePivot);

   return FbxAMatrix(T, R, S);
}

void importGeometry(FbxNode* fbxNode, const char* sMeshName, int iMaterialIndex)
{
   FbxMesh* fbxMesh = fbxNode->GetMesh();

   if(bSplitPoints)
      fbxMesh->SplitPoints(FbxLayerElement::eUV);

   int iPolyCount = fbxMesh->GetPolygonCount();
   int iUniqueVerticesCount = 0;

   std::vector<Vector3> vVertices;
   std::vector<Vector3> vNormals;
   std::vector<Vector2> vTexCoords;
   std::vector<uint> vIndices;
   std::vector<std::vector<VertexBoneAttachment>> vVertexBoneAttachments;

   char sMeshFinalName[64];
   strcpy(sMeshFinalName, sMeshName);

   assert(fbxMesh->GetLayerCount() > 0);

   FbxLayerElementMaterial* fbxLayerMaterial = fbxMesh->GetLayer(0)->GetMaterials();
   FbxLayerElementArrayTemplate<int>& fbxPolygonMaterialMapping = fbxLayerMaterial->GetIndexArray();

   FbxSurfaceMaterial* fbxMaterial = fbxNode->GetMaterial(iMaterialIndex);
   const char* sMaterialName = fbxMaterial->GetName();

   uint iMaterialNameHash = Core::hash(sMaterialName);
   std::map<uint, SubMesh>::iterator it = mSubMeshes.find(iMaterialNameHash);
   SubMesh& sSubMesh = it != mSubMeshes.end() ? it->second : SubMesh();

   // indices
   for(int iPoly = 0; iPoly < iPolyCount; iPoly++)
   {
      assert(fbxMesh->GetPolygonSize(iPoly) == 3);

      for(int iVertex = 0; iVertex < 3; iVertex++)
      {
         int iVertexIndex = fbxMesh->GetPolygonVertex(iPoly, iVertex);
         assert(iVertexIndex != -1);

         if(iVertexIndex > iUniqueVerticesCount)
            iUniqueVerticesCount = iVertexIndex;

         vIndices.push_back((ushort)iVertexIndex);
      }
   }

   // geometry data
   iUniqueVerticesCount++;
   assert(iUniqueVerticesCount == fbxMesh->GetControlPointsCount());

   vVertices.resize(iUniqueVerticesCount);
   vNormals.resize(iUniqueVerticesCount);

   FbxLayerElementArrayTemplate<FbxVector2>* fbxTextureUV = 0;
   bUseTexture = fbxMesh->GetTextureUV(&fbxTextureUV);

   if(bUseTexture)
      vTexCoords.resize(iUniqueVerticesCount);

   float fMaxUVValue = 0.0f;

   for(int iPoly = 0; iPoly < iPolyCount; ++iPoly)
   {
      if(fbxPolygonMaterialMapping.GetAt(iPoly) != iMaterialIndex)
         continue;

      for(int iVertex = 0; iVertex < 3; ++iVertex)
      {
         int iVertexIndex = fbxMesh->GetPolygonVertex(iPoly, iVertex);

         // position
         FbxVector4 fbxVertex = fbxMesh->GetControlPointAt(iVertexIndex);
         vVertices[iVertexIndex] = getTranslation(fbxVertex);

         // normal
         FbxVector4 fbxNormal;
         fbxMesh->GetPolygonVertexNormal(iPoly, iVertex, fbxNormal);
         fbxNormal.Normalize();
         vNormals[iVertexIndex] = getTranslation(fbxNormal);

         // texture coordinate
         if(bUseTexture)
         {
            FbxGeometryElementUV* fbxUV = fbxMesh->GetElementUV(0);
            FbxVector2 vUV;
            bool bUnmapped;
            fbxMesh->GetPolygonVertexUV(iPoly, iVertex, fbxUV->GetName(), vUV, bUnmapped);
            vTexCoords[iVertexIndex] = Vector2((float)vUV[0], 1.0f - (float)vUV[1]);
         }
      }
   }

   // skinning data
   if(bSkinningData)
   {
      vVertexBoneAttachments.resize(iUniqueVerticesCount);

      for(int iDeformerIndex = 0; iDeformerIndex < fbxMesh->GetDeformerCount(); iDeformerIndex++)
      {
         FbxSkin* fbxSkin = reinterpret_cast<FbxSkin*>(fbxMesh->GetDeformer(iDeformerIndex, FbxDeformer::eSkin));

         if(!fbxSkin)
            continue;

         FBXSDK_NAMESPACE::FbxAMatrix fbxGeometryTransform = getGeometry(fbxNode);

         for(int iClusterIndex = 0; iClusterIndex < fbxSkin->GetClusterCount(); iClusterIndex++)
         {
            FbxCluster* fbxCluster = fbxSkin->GetCluster(iClusterIndex);
            ImportBone* cBone = cSkeletonBones.get(ObjectName(fbxCluster->GetLink()->GetName()));

            FBXSDK_NAMESPACE::FbxAMatrix fbxTransformMatrix;
            FBXSDK_NAMESPACE::FbxAMatrix fbxTransformLinkMatrix;
            fbxCluster->GetTransformMatrix(fbxTransformMatrix);
            fbxCluster->GetTransformLinkMatrix(fbxTransformLinkMatrix);

            cBone->fbxInverseLinkTransform = fbxTransformLinkMatrix.Inverse() * fbxTransformMatrix * fbxGeometryTransform;
            cBone->fbxLinkTransform = cBone->fbxInverseLinkTransform.Inverse();
            cBone->LinkTransformMatrixSet = true;

            for(int i = 0; i < fbxCluster->GetControlPointIndicesCount(); i++)
            {
               uint iVertexIndex = (uint)fbxCluster->GetControlPointIndices()[i];

               if(vVertexBoneAttachments[iVertexIndex].size() == Mesh::BoneAttachmentsPerVertex)
                  continue;

               VertexBoneAttachment sVertexBoneAttachment;
               sVertexBoneAttachment.BoneIndex = cBone->getIndex();
               sVertexBoneAttachment.Weight = (float)fbxCluster->GetControlPointWeights()[i];

               vVertexBoneAttachments[iVertexIndex].push_back(sVertexBoneAttachment);
            }
         }
      }

      // set up the pose for the bones without any cluster attached
      cSkeletonBones.iterate([&fbxNode] (ImportBone* b)
      {
         if(!b->LinkTransformMatrixSet)
         {
            ImportBone* cParent = cSkeletonBones.get(b->ParentName);

            if(cParent)
            {
               FBXSDK_NAMESPACE::FbxAMatrix fbxLocalTransform;
               fbxLocalTransform.SetTRS(
                  b->fbxNode->LclTranslation.Get(),
                  b->fbxNode->LclRotation.Get(),
                  b->fbxNode->LclScaling.Get());
               b->fbxLinkTransform = cParent->fbxLinkTransform * fbxLocalTransform;
            }
         }

         return true;
      });

      // make sure that each vertex has exactly 4 bone attachments
      VertexBoneAttachment sEmptyVertexBoneAttachment;

      for(int i = 0; i < iUniqueVerticesCount; i++)
      {
         while(vVertexBoneAttachments[i].size() < Mesh::BoneAttachmentsPerVertex)
            vVertexBoneAttachments[i].push_back(sEmptyVertexBoneAttachment);
      }
   }

   // compose the vertex data
   for(uint i = 0; i < vVertices.size(); i++)
   {
      sSubMesh.VertexData.push_back(vVertices[i].X);
      sSubMesh.VertexData.push_back(vVertices[i].Y);
      sSubMesh.VertexData.push_back(vVertices[i].Z);

      sSubMesh.VertexData.push_back(vNormals[i].X);
      sSubMesh.VertexData.push_back(vNormals[i].Y);
      sSubMesh.VertexData.push_back(vNormals[i].Z);

      if(bUseTexture)
      {
         sSubMesh.VertexData.push_back(vTexCoords[i].X);
         sSubMesh.VertexData.push_back(vTexCoords[i].Y);
      }
   }

   for(uint i = 0; i < vIndices.size(); i++)
      sSubMesh.Indices.push_back(vIndices[i] + sSubMesh.CurrentIndexOffset);
   
   if(bSkinningData)
   {
      for(uint i = 0; i < vVertexBoneAttachments.size(); i++)
         sSubMesh.SkinningData.push_back(vVertexBoneAttachments[i]);
   }

   sSubMesh.CurrentIndexOffset += (ushort)vVertices.size();

   strcpy(sSubMesh.MaterialName, sMaterialName);
   mSubMeshes[Core::hash(sMaterialName)] = sSubMesh;
}

void importAnimations(FbxScene* fbxScene, FbxNode* fbxNode)
{
   ImportBone* cBone = cSkeletonBones.get(ObjectName(fbxNode->GetName()));

   if(cBone)
   {
      FbxAMatrix fbxAnimTransform;

      if(cBone->ParentName == ObjectName::Empty)
      {
         FbxVector4 fbxAnimPreRotation = fbxNode->GetPreRotation(FbxNode::eSourcePivot);
         FbxVector4 fbxAnimScaleFactor = fbxNode->EvaluateLocalTransform().GetS();
         fbxAnimTransform.SetR(fbxAnimPreRotation);
         fbxAnimTransform.SetS(fbxAnimScaleFactor);
         fbxAnimTransform = fbxAnimTransform.Inverse();
      }
      
      FbxAnimEvaluator* fbxAnimEvaluator = fbxScene->GetAnimationEvaluator();

      for(int iAnimStackIndex = 0; iAnimStackIndex < fbxScene->GetSrcObjectCount<FbxAnimStack>(); iAnimStackIndex++)
      {
         FbxAnimStack* fbxAnimStack = fbxScene->GetSrcObject<FbxAnimStack>(iAnimStackIndex);
         FbxAnimLayer* fbxAnimLayer = fbxAnimStack->GetMember<FbxAnimLayer>(0);
         
         if(!fbxAnimLayer)
            continue;

         FbxTakeInfo* fbxTakeInfo = fbxImporter->GetTakeInfo(0);
         FbxTimeSpan fbxTimeSpan = fbxTakeInfo->mLocalTimeSpan;

         FbxLongLong iTimeBegin = fbxTimeSpan.GetStart().GetMilliSeconds();
         FbxLongLong iTimeEnd = fbxTimeSpan.GetStop().GetMilliSeconds();
         FbxLongLong iDurationMs = iTimeEnd - iTimeBegin;

         FbxTime::EMode fbxTimeMode = fbxScene->GetGlobalSettings().GetTimeMode();
         const uint FrameRate = fbxTimeMode == FbxTime::eFrames30 ? 30 : 24;
         uint iKeyCount = (uint)((float)iDurationMs * 0.001f * (float)FrameRate);

         FbxLongLong iTimePerKeyFrame = iDurationMs / iKeyCount;
         FbxLongLong iCurrentTimeMs = iTimeBegin;

         ObjectName cAnimationName = ObjectName(fbxAnimStack->GetName());
         Animation* cAnimation = cAnimations.get(cAnimationName);

         if(!cAnimation)
         {              
            cAnimation = new Animation(cAnimationName, iKeyCount, cSkeletonBones.count());
            cAnimations.add(cAnimation);
         }

         for(uint iKeyIndex = 0; iKeyIndex < iKeyCount; iKeyIndex++)
         {
            FbxTime fbxCurrentTime;
            fbxCurrentTime.SetMilliSeconds(iCurrentTimeMs);

            AnimationKeyFrame& sKeyFrame = cAnimation->getKeyFrame(iKeyIndex, cBone->getIndex());
            sKeyFrame.TimeInSeconds = (float)iCurrentTimeMs * 0.001f;

            FBXSDK_NAMESPACE::FbxAMatrix fbxKeyFrameBoneBindTransform = fbxNode->EvaluateLocalTransform(fbxCurrentTime);
            fbxKeyFrameBoneBindTransform = fbxAnimTransform * fbxKeyFrameBoneBindTransform;

            sKeyFrame.FramePosition = getTranslation(fbxKeyFrameBoneBindTransform.GetT());
            sKeyFrame.FrameRotation = getRotation(fbxKeyFrameBoneBindTransform.GetR());
            sKeyFrame.FrameScale = getScale(fbxKeyFrameBoneBindTransform.GetS());

            iCurrentTimeMs += iTimePerKeyFrame;
         }
      }
   }
   
   for(int i = 0; i < fbxNode->GetChildCount(); i++)
      importAnimations(fbxScene, fbxNode->GetChild(i));
}

Vector3 getTranslation(const FbxVector4& fbxVector)
{
   return Vector3((float)fbxVector[0], (float)fbxVector[1], (float)fbxVector[2]);
}

Vector3 getRotation(const FbxVector4& fbxVector)
{
   return Vector3((float)-fbxVector[0], (float)-fbxVector[1], (float)-fbxVector[2]);
}

Vector3 getScale(const FbxVector4& fbxVector)
{
   return Vector3((float)fbxVector[0], (float)fbxVector[1], (float)fbxVector[2]);
}

void generateMeshFile(const SubMesh& sSubMesh, const char* sName)
{
   uint iFloatValuesPerVertex = 3 + 3;    // Position (Vector3), Normal (Vector3)

   if(bUseTexture)
      iFloatValuesPerVertex += 2;         // UV (Vector2)

   uint iVertexStride = iFloatValuesPerVertex * sizeof(float);
   uint iNumVertices = sSubMesh.VertexData.size() / iFloatValuesPerVertex;
   uint iNumIndices = sSubMesh.Indices.size();

   char sDestinyFileName[64];
   sprintf(sDestinyFileName, "%s.mesh.ge", sName);
   std::ofstream fDestinyFile(sDestinyFileName, std::ios::binary);

   if(!fDestinyFile.is_open())
   {
      std::cout << " ERROR: cannot create the output file \"" << sDestinyFileName << "\"\n\n";
      exit(1);
   }
   
   std::cout << " Generating \"" << sDestinyFileName << "\"...\n";

   // header (24 bytes)
   fDestinyFile.write("GEMesh  ", 8);
   fDestinyFile.write(reinterpret_cast<char*>(&iNumVertices), sizeof(uint));
   fDestinyFile.write(reinterpret_cast<char*>(&iNumIndices), sizeof(uint));
   fDestinyFile.write(reinterpret_cast<char*>(&iVertexStride), sizeof(uint));

   uint iValue = (uint)bUseTexture;
   fDestinyFile.write(reinterpret_cast<char*>(&iValue), sizeof(uint));

   iValue = (uint)bSkinningData;
   fDestinyFile.write(reinterpret_cast<char*>(&iValue), sizeof(uint));

   // reserved
   for(uint i = 0; i < Mesh::FileFormatHeaderReservedBytes; i++)
      fDestinyFile.write(&Zero, 1);

   // vertex data
   fDestinyFile.write(reinterpret_cast<const char*>(&sSubMesh.VertexData[0]), sSubMesh.VertexData.size() * sizeof(float));

   // indices
   fDestinyFile.write(reinterpret_cast<const char*>(&sSubMesh.Indices[0]), sSubMesh.Indices.size() * sizeof(ushort));

   // skinning data
   if(bSkinningData)
   {
      for(uint i = 0; i < iNumVertices; i++)
      {
         for(uint j = 0; j < Mesh::BoneAttachmentsPerVertex; j++)
         {
            const VertexBoneAttachment& cAttachment = sSubMesh.SkinningData[i][j];
            fDestinyFile.write(reinterpret_cast<const char*>(&cAttachment.BoneIndex), sizeof(uint));
            fDestinyFile.write(reinterpret_cast<const char*>(&cAttachment.Weight), sizeof(float));
         }
      }
   }

   fDestinyFile.close();
}

void generateAnimationFile(GE::Content::Animation* cAnimation)
{
   const uint AnimationNameSize = 32;

   char sAnimationName[AnimationNameSize];
   memset(sAnimationName, 0, AnimationNameSize);
   memcpy(sAnimationName, cAnimation->getName().getString().c_str(), strlen(cAnimation->getName().getString().c_str()));

   uint iKeyFramesCount = cAnimation->getKeyFramesCount();
   uint iSkeletonBonesCount = cAnimation->getSkeletonBonesCount();

   char sDestinyFileName[128];
   sprintf(sDestinyFileName, "%s.%s.animation.ge", sSourceFileName, sAnimationName);
   std::ofstream fDestinyFile(sDestinyFileName, std::ios::binary);

   if(!fDestinyFile.is_open())
   {
      std::cout << " ERROR: cannot create the output file \"" << sDestinyFileName << "\"\n\n";
      exit(1);
   }

   std::cout << " Generating \"" << sDestinyFileName << "\"...\n";

#if defined (_DEBUG)
   xml_document xmlAnimation;
   xml_node xmlAnimationRoot = xmlAnimation.append_child("Animation");
   xmlAnimationRoot.append_attribute("name").set_value(sAnimationName);
   xmlAnimationRoot.append_attribute("keyFrames").set_value(iKeyFramesCount);
   xmlAnimationRoot.append_attribute("skeletonBones").set_value(iSkeletonBonesCount);
#endif

   // header (56 bytes)
   fDestinyFile.write("GEMeshAnimation ", 16);
   fDestinyFile.write(sAnimationName, AnimationNameSize);
   fDestinyFile.write(reinterpret_cast<char*>(&iKeyFramesCount), sizeof(uint));
   fDestinyFile.write(reinterpret_cast<char*>(&iSkeletonBonesCount), sizeof(uint));

   // reserved
   for(uint i = 0; i < Animation::FileFormatHeaderReservedBytes; i++)
      fDestinyFile.write(&Zero, 1);

   // animation data
   for(uint i = 0; i < iKeyFramesCount; i++)
   {
#if defined (_DEBUG)
      xml_node xmlAnimationKeyFrame = xmlAnimationRoot.append_child("KeyFrame");
      xmlAnimationKeyFrame.append_attribute("index").set_value(i);
#endif

      for(uint j = 0; j < iSkeletonBonesCount; j++)
      {
         AnimationKeyFrame& cKeyFrame = cAnimation->getKeyFrame(i, j);

         fDestinyFile.write(reinterpret_cast<char*>(&cKeyFrame.TimeInSeconds), sizeof(float));
         fDestinyFile.write(reinterpret_cast<char*>(&cKeyFrame.FramePosition.X), sizeof(Vector3));
         fDestinyFile.write(reinterpret_cast<char*>(&cKeyFrame.FrameRotation.X), sizeof(Vector3));
         fDestinyFile.write(reinterpret_cast<char*>(&cKeyFrame.FrameScale.X), sizeof(Vector3));

#if defined (_DEBUG)
         ObjectName cBoneName;

         cSkeletonBones.iterate([j, &cBoneName](ImportBone* cBone)
         {
            if(cBone->getIndex() == j)
            {
               cBoneName = cBone->getName();
               return false;
            }

            return true;
         });

         xml_node xmlAnimationKeyFrameBone = xmlAnimationKeyFrame.append_child("Bone");
         xmlAnimationKeyFrameBone.append_attribute("name").set_value(cBoneName.getString().c_str());
         char sBuffer[64];
         sprintf(sBuffer, "%.3f", cKeyFrame.TimeInSeconds);
         xmlAnimationKeyFrameBone.append_attribute("time").set_value(sBuffer);

         Parser::writeVector3(cKeyFrame.FramePosition, sBuffer);
         xmlAnimationKeyFrameBone.append_attribute("position").set_value(sBuffer);
         Parser::writeVector3(cKeyFrame.FrameRotation, sBuffer);
         xmlAnimationKeyFrameBone.append_attribute("rotation").set_value(sBuffer);
         Parser::writeVector3(cKeyFrame.FrameScale, sBuffer);
         xmlAnimationKeyFrameBone.append_attribute("scale").set_value(sBuffer);
#endif
      }
   }

#if defined (_DEBUG)
   sprintf(sDestinyFileName, "%s.%s.animation.xml", sSourceFileName, sAnimationName);
   xmlAnimation.save_file(sDestinyFileName, "  ");
#endif

   fDestinyFile.close();
}
