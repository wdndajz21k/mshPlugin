/*--------------------MSH����ඨ��--------------------*/
#include <MSHReader.h>
#include "stdafx.h"
MSHReader::MSHReader(KFbxSdkManager &pFbxSdkManager, int pID):
KFbxReader(pFbxSdkManager, pID),
mFilePointer(NULL),
mManager(&pFbxSdkManager)
{
}

MSHReader::~MSHReader()
{
    FileClose();
}

void MSHReader::GetVersion(int& pMajor, int& pMinor, int& pRevision) const
{
    pMajor = 1;
    pMinor = 0;
    pRevision = 0;
}

bool MSHReader::FileOpen(char* pFileName)
{
    if(mFilePointer != NULL)
	{
        FileClose();
	}
    mFilePointer = fopen(pFileName, "rb");  //�����Ʒ�ʽ��ȡ
    if(mFilePointer == NULL)
	{
        return false;
	}
    return true;
}

bool MSHReader::FileClose()
{
    if(mFilePointer!=NULL)
	{
        fclose(mFilePointer);
	}
    return true;

}
bool MSHReader::IsFileOpen()
{
    if(mFilePointer != NULL)
	{
        return true;
	}
    return false;
}

bool MSHReader::GetReadOptions(bool pParseFileAsNeeded)
{
    return true;
}

/*--------------------��ȡMSH��Ϣ--------------------*/
bool MSHReader::Read(KFbxDocument* pDocument)
{
    if (!pDocument)
	{
        GetError().SetLastErrorID(eINVALID_DOCUMENT_HANDLE);
        return false;
    }
    //����������
    KFbxScene*      lScene = (KFbxScene*)pDocument;
    bool            lResult = false;

    if(lScene == NULL)
	{
        return false;
	}

    //��ʼ�������
    KFbxNode* lRootNode = lScene->GetRootNode();
    KFbxNodeAttribute * lRootNodeAttribute = KFbxNull::Create(lScene,"Scene Root");
    lRootNode->SetNodeAttribute(lRootNodeAttribute);

    if(mFilePointer == NULL)
	{
        return true;
	}

    //��ȡ�ļ���С
    unsigned lSize;  //�ļ���С
    char* lBuffer = NULL;
    fseek(mFilePointer, 0, SEEK_END);
    lSize = ftell(mFilePointer);
    rewind(mFilePointer);
    //�����ļ���lBuffer
    lBuffer = (char*) malloc (sizeof(char)*lSize + 1);
    size_t lRead = fread(lBuffer, 1, lSize, mFilePointer);
    lBuffer[lRead] = NULL;

    //��������ָ��
    char *pBone, *pReader;
    MshHeader* pHeader;
    MeshInfo* pMeshInfo;
    MeshDataPointer meshData;
    //��ʼ����ָ��
    pReader = InitialPointer(lBuffer, pBone, pHeader);

    //��������
    if(pHeader->boneCount > 0)
	{
        BoneData* pBoneData = (BoneData*)pBone;
        CreateSkeleton(lScene, pHeader, pBoneData);

        //����meshCount��meshģ�Ͳ��������
        for(int i = 0; i < pHeader->meshCount; i++)
		{
             //��ʼ��Mesh��ָ��
            pReader = InitialMeshPointer(pReader, pMeshInfo, &meshData);

            //��ȡ������Mesh
            KFbxNode* pMesh = CreateMesh(lScene, pMeshInfo, &meshData);
            lRootNode->AddChild(pMesh);

            //���ӹ�����Mesh
            KFbxCluster** pClusterIndex = LinkSkeleton(lRootNode, pMesh, &meshData);
            //��ӹ���Ȩ��
            AddWeight(pClusterIndex, pMeshInfo, &meshData);
            //���BindPose
            StoreBindPose(lScene, pMesh, pClusterIndex, &meshData);

        }
    }
    //�ͷŻ���
    free(lBuffer);

    lResult = true;
    return lResult;
}

/*--------------------�Զ��庯������--------------------*/
char* MSHReader::InitialPointer(char* pReader,
                                char* &pBone,
                                MshHeader* &pHeader)
{
    //MSHͷ��Ϣ��ʼ��
    pHeader = (MshHeader*)pReader;
    //�������ݿ�ʼ��
    pReader += 0x400;
    pBone = pReader;
    //Mesh��Ϣ��ʼ��
    pReader += pHeader->boneCount * 0x140;

    //���ع���ָ��λ��
    return pReader;
}

char* MSHReader::InitialMeshPointer(char* pReader,
                                    MeshInfo* &pMeshInfo,
                                    MeshDataPointer* pMeshData)
{
    //Mesh��Ϣ��ʼ��
    pMeshInfo = (MeshInfo*)pReader;
    //Mesh���ݿ�ʼ��
    pReader += 0x400;
    pMeshData->pFaceIndex = pReader;
    pReader += pMeshInfo->faceIndexCount * 0x2;
    pMeshData->pVertexData = pReader;
    pReader += pMeshInfo->vertexCount * 0xC;
    pMeshData->pNormalData = pReader;
    pReader += pMeshInfo->vertexCount * 0xC;
    pMeshData->pUVData = pReader;
    pReader += pMeshInfo->vertexCount * 0x8;
    pMeshData->pBoneIndex = pReader;
    pReader += pMeshInfo->vertexCount * 0x8;
    pMeshData->pBoneWeight = pReader;
    pReader += pMeshInfo->vertexCount * 0x10;
    pMeshData->pBoneCount = (int*)pReader;
    pReader += 0x4;
    pMeshData->pBoneName = pReader;

    //���ع���ָ��λ��
    pReader += *pMeshData->pBoneCount * 0x100;  //��n��Mesh����β��
    return pReader;
}

KFbxNode* MSHReader::CreateMesh(KFbxScene* pScene,
                                MeshInfo* pMeshInfo,
                                MeshDataPointer* pMeshData)
{
    //����mesh���
    KFbxMesh* lMesh = KFbxMesh::Create(pScene, pMeshInfo->meshName);
    //��mesh�����뵽����
    KFbxNode* lNode = KFbxNode::Create(pScene, pMeshInfo->meshName);
    lNode->SetNodeAttribute(lMesh);
    lNode->SetShadingMode(KFbxNode::eTEXTURE_SHADING);
    //����Layer Container
    lMesh->CreateLayer();
    KFbxLayer* layer = lMesh->GetLayer(0);
    //���ӷ��߲�
    K_ASSERT(layer != NULL);
    KFbxLayerElementNormal* normLayer;
    normLayer = KFbxLayerElementNormal::Create((KFbxLayerContainer*)lMesh, "");
    normLayer->SetMappingMode(KFbxLayerElement::eBY_POLYGON_VERTEX);
    normLayer->SetReferenceMode(KFbxLayerElement::eINDEX_TO_DIRECT);
    layer->SetNormals(normLayer);
    //����UV��
    KFbxLayerElementUV* uvLayer;
    uvLayer = KFbxLayerElementUV::Create((KFbxLayerContainer*)lMesh, "map1");
    uvLayer->SetMappingMode(KFbxLayerElement::eBY_POLYGON_VERTEX);
    uvLayer->SetReferenceMode(KFbxLayerElement::eINDEX_TO_DIRECT);
    layer->SetUVs(uvLayer);

    //��ȡ������Ϣ
    Vec3F* tVertex = (Vec3F*)pMeshData->pVertexData;   //����ָ��,xyz����float
    lMesh->InitControlPoints(pMeshInfo->vertexCount);
    KFbxVector4* lControlPoints = lMesh->GetControlPoints();
    for(int i = 0; i < pMeshInfo->vertexCount; i++)
	{
        lControlPoints[i] = KFbxVector4(tVertex->x, tVertex->y, tVertex->z);
        tVertex++;
    }

    //��ȡ������Ϣ
    Vec3F* tNormal = (Vec3F*)pMeshData->pNormalData;
    for(int i = 0; i < pMeshInfo->vertexCount; i++)
	{
        normLayer->GetDirectArray().Add(KFbxVector4(tNormal->x, tNormal->y, tNormal->z));
        tNormal++;
    }

    //��ȡUV��Ϣ
    Vec2F* pUV = (Vec2F*)pMeshData->pUVData;
    for(int i = 0; i < pMeshInfo->vertexCount; i++)
	{
        uvLayer->GetDirectArray().Add(KFbxVector2(pUV->x, 1 - pUV->y)); //��ֱ��ת
        pUV++;
    }

    //���ݶ���������Ϣ����Mesh
    short* pFace = (short*)pMeshData->pFaceIndex;
    //GL_TRIANGLES ģʽ
    if(pMeshInfo->renderMode == 0x100)
	{
        for(; pFace + 2 <= (short*)pMeshData->pVertexData;)
		{
            lMesh->BeginPolygon();
            for(int j = 0; j < 3; j++)
			{
                lMesh->AddPolygon(*pFace);
                uvLayer->GetIndexArray().Add(*pFace);
                normLayer->GetIndexArray().Add(*pFace);
                pFace++;
            }
            lMesh->EndPolygon();
        }
	}
    //GL_TRIANGLE_STRIP ģʽ
    else
	{
        for(unsigned i = 2; i < pMeshInfo->faceIndexCount; i++)
		{
            lMesh->BeginPolygon();
            if(i & 1)
			{
                lMesh->AddPolygon(pFace[0]);
                lMesh->AddPolygon(pFace[2]);
                lMesh->AddPolygon(pFace[1]);
                uvLayer->GetIndexArray().Add(pFace[0]);
                uvLayer->GetIndexArray().Add(pFace[2]);
                uvLayer->GetIndexArray().Add(pFace[1]);
                normLayer->GetIndexArray().Add(pFace[0]);
                normLayer->GetIndexArray().Add(pFace[2]);
                normLayer->GetIndexArray().Add(pFace[1]);
            }
            else
			{
                lMesh->AddPolygon(pFace[0]);
                lMesh->AddPolygon(pFace[1]);
                lMesh->AddPolygon(pFace[2]);
                uvLayer->GetIndexArray().Add(pFace[0]);
                uvLayer->GetIndexArray().Add(pFace[1]);
                uvLayer->GetIndexArray().Add(pFace[2]);
                normLayer->GetIndexArray().Add(pFace[0]);
                normLayer->GetIndexArray().Add(pFace[1]);
                normLayer->GetIndexArray().Add(pFace[2]);
            }
            lMesh->EndPolygon();
            pFace++;
        }
    }

    lMesh->RemoveBadPolygons();

    return lNode;
}

//��������
void MSHReader::CreateSkeleton(KFbxScene* pScene,
                               MshHeader* pHeader,
                               BoneData* pBoneData)
{
    for(int i = 0; i < pHeader->boneCount; i++)
	{
        KFbxSkeleton* lSkeleton = KFbxSkeleton::Create(pScene, "");
        lSkeleton->SetSkeletonType(KFbxSkeleton::eLIMB_NODE);
        lSkeleton->Size.Set(50.0);
        KFbxNode* lNode = KFbxNode::Create(pScene, pBoneData->boneName);
        lNode->SetNodeAttribute(lSkeleton);

        //��ӱ任���󵽹���
        //��Ҫע��ԭ�����ǹ�������任����������
        double tMatrix[4][4];
        for(int i = 0; i < 4; i++)//�����ȵ�˫���ȵ�ת��
		{ 
            tMatrix[i][0] = pBoneData->transformMatrix[i].x;
            tMatrix[i][1] = pBoneData->transformMatrix[i].y;
            tMatrix[i][2] = pBoneData->transformMatrix[i].z;
            tMatrix[i][3] = pBoneData->transformMatrix[i].w;
        }
        KFbxXMatrix pMatrix = *(KFbxXMatrix*)&tMatrix;
        pMatrix = pMatrix.Inverse();

        //��λ����λ��
        lNode->LclTranslation.Set(pMatrix.GetT());
        lNode->LclRotation.Set(pMatrix.GetR());
        lNode->LclScaling.Set(pMatrix.GetS());
        //�趨�����任������Ϣ
        lNode->SetGeometricTranslation(KFbxNode::eSOURCE_SET, pMatrix.GetT());
        lNode->SetGeometricRotation(KFbxNode::eSOURCE_SET, pMatrix.GetR());
        lNode->SetGeometricScaling(KFbxNode::eSOURCE_SET, pMatrix.GetS());

        //��ӵ��������ڵ�
        pScene->GetRootNode()->AddChild(lNode);

        //��һ����������
        pBoneData++;
    }
}

//������Ӧ������Mesh,����ֵ��һ�����Լ����ָ�������
KFbxCluster** MSHReader::LinkSkeleton(KFbxNode* pScene,
                                      KFbxNode* pMesh,
                                      MeshDataPointer* pMeshData)
{
    int meshBoneCount = *pMeshData->pBoneCount;
    char* pBoneName = pMeshData->pBoneName;
    KFbxGeometry* lMeshAttribute = (KFbxGeometry*)pMesh->GetNodeAttribute();
    KFbxSkin* lSkin = KFbxSkin::Create(pScene, "");
    //�����������ƶ�Ӧ��n����,pCluster��ָ��Լ����ָ���ָ��
    KFbxCluster** pCluster = (KFbxCluster**)malloc(sizeof(KFbxCluster*) * meshBoneCount);

    //���ӹ����ڵ���Լ����,׷����Ƥ
    for(int i = 0; i < meshBoneCount; i++)
	{
        pCluster[i] = KFbxCluster::Create(pScene,"");
        pCluster[i]->SetLink(pScene->FindChild(pBoneName));
        pCluster[i]->SetLinkMode(KFbxCluster::eTOTAL1);

        //ΪԼ�����趨����任����
        KFbxNode* pChildNode = pScene->FindChild(pBoneName);
        KFbxXMatrix childMatrix(pChildNode->GetGeometricTranslation(KFbxNode::eSOURCE_SET),
                                pChildNode->GetGeometricRotation(KFbxNode::eSOURCE_SET),
                                pChildNode->GetGeometricScaling(KFbxNode::eSOURCE_SET));
        pCluster[i]->SetTransformLinkMatrix(childMatrix);

        //���Լ��������Ƥ
        lSkin->AddCluster(pCluster[i]);
        pBoneName += 0x100;
    }
    //�����Ƥ��Mesh
    lMeshAttribute->AddDeformer(lSkin);
    return pCluster;
}

//���Ȩ��
void MSHReader::AddWeight(KFbxCluster** pClusterIndex,
                          MeshInfo* pMeshInfo,
                          MeshDataPointer* pMeshData)
{
    float* boneWeight = (float*)pMeshData->pBoneWeight;
    short* pBoneIndex = (short*)pMeshData->pBoneIndex;
    for(int i = 0; i < pMeshInfo->vertexCount; i++)
	{
        for(int j = 0; j < 4; j++)//ÿ���������4������Ȩ��
		{  
            if(*(boneWeight + i * 4 + j) != 0)  //��i�������j��Ȩ������
			{
                pClusterIndex[*pBoneIndex]->AddControlPointIndex(i, *(boneWeight + i * 4 + j));
			}
            pBoneIndex++;
        }
	}
}

//���浱ǰPose
void MSHReader::StoreBindPose(KFbxScene* pScene,
                              KFbxNode* pMesh,
                              KFbxCluster** pClusterIndex,
                              MeshDataPointer* pMeshData)
{
    KFbxPose* lPose = KFbxPose::Create(pScene,pMesh->GetName());
    lPose->SetIsBindPose(true);

    KFbxMatrix lBindMeshMatrix = pScene->GetEvaluator()->GetNodeGlobalTransform(pMesh);
    lPose->Add(pMesh, lBindMeshMatrix);
    for(int i=0; i < *pMeshData->pBoneCount; i++)
	{
        KFbxNode* lKFbxNode = pClusterIndex[i]->GetLink();
        KFbxMatrix lBindMatrix = pScene->GetEvaluator()->GetNodeGlobalTransform(lKFbxNode);
        lBindMatrix = lBindMatrix.Inverse();
        lPose->Add(lKFbxNode, lBindMatrix);
    }
    pScene->AddPose(lPose);
}