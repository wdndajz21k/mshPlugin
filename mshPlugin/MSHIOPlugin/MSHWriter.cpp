#include "stdafx.h"
MSHWriter::MSHWriter(KFbxSdkManager &pFbxSdkManager, int pID):
KFbxWriter(pFbxSdkManager, pID),
mFilePointer(NULL),
mManager(&pFbxSdkManager)
{

}

MSHWriter::~MSHWriter()
{
    FileClose();
}

// Create a file stream with pFileName
bool MSHWriter::FileCreate(char* pFileName)
{
    if(mFilePointer != NULL)
	{
        FileClose();
	}

    mFilePointer = fopen(pFileName, "wb");

    if(mFilePointer == NULL)
	{
        return false;
	}

    return true;
}

// Close the file stream
bool MSHWriter::FileClose()
{
    if(mFilePointer != NULL)
    {
        fclose(mFilePointer);
        return true;
    }
    return false;
}

// Check whether the file stream is open.
bool MSHWriter::IsFileOpen()
{
    if(mFilePointer != NULL)
	{
        return true;
	}
    return false;
}

// Get the file stream options
void MSHWriter::GetWriteOptions()
{
}

// Write file with stream options
bool MSHWriter::Write(KFbxDocument* pDocument)
{
    if (!pDocument)
	{
        GetError().SetLastErrorID(eINVALID_DOCUMENT_HANDLE);
        return false;
    }

    KFbxScene* lScene = (KFbxScene*)pDocument;
    bool lIsAScene = (lScene != NULL);
    bool lResult = true;

    if(lScene == NULL)
	{
        return false;
	}

    KFbxNode* lRootNode = lScene->GetRootNode();

    //����0x400�ֽ�,��ΪBoneData��0x400��ʼ
    char empty[0x400] = {0};
    fwrite((char*)empty, 0x400, 1, mFilePointer);

    //д���������
    for(int i = 0; i < lRootNode->GetChildCount(); i++)
	{
        if(lRootNode->GetChild(i)->GetSkeleton())
		{
            WriteBoneData(lScene, lRootNode->GetChild(i));
		}
	}

    //д��Mesh����
    for(int i = 0; i < lRootNode->GetChildCount(); i++)
	{
        if(lRootNode->GetChild(i)->GetMesh())
		{
            lResult = WriteMeshData(lRootNode->GetChild(i));
            if(!lResult)return false;   //����ʱ����
        }
	}

    //д���ļ�ͷ��
    fseek(mFilePointer, 0, SEEK_SET);
    WriteHeader(lScene);

    return lResult;
}


// Pre-process the scene before write it out
bool MSHWriter::PreprocessScene(KFbxScene &pScene)
{
    return true;
}

// Post-process the scene after write it out
bool MSHWriter::PostprocessScene(KFbxScene &pScene)
{
    return true;
}


void MSHWriter::WriteHeader(KFbxScene* pScene)
{
    size_t lSize = 0x130;   //sizeof(MshHeader);
    MshHeader lHeader;
    char* lBuffer = (char*)&lHeader;

    //����BoundingBox
    KFbxNode* pRootNode = pScene->GetRootNode();
    KFbxGeometry* pMeshAttribute;
    KFbxVector4 bbMax,bbMin;
    for(int i = 0; i < 3; i++)
	{
        for(int j = 0; j < pRootNode->GetChildCount(); j++)
		{
            if(pRootNode->GetChild(j)->GetMesh())
			{
                pMeshAttribute = (KFbxGeometry*)pRootNode->GetChild(j)->GetNodeAttribute();
                pMeshAttribute->ComputeBBox();
                if(bbMax.GetAt(i) < pMeshAttribute->BBoxMax.Get()[i])
                    bbMax.SetAt(i, pMeshAttribute->BBoxMax.Get()[i]);
                if(bbMin.GetAt(i) > pMeshAttribute->BBoxMin.Get()[i])
                    bbMin.SetAt(i, pMeshAttribute->BBoxMin.Get()[i]);
            }
		}
	}

    //�༭�ļ�ͷ
    memset(lHeader.name, 0, 256);  //�ַ�����������
    strcpy(lHeader.name, "Eternity Engine Mesh File 0.1");
    lHeader.version = 0xC;
    lHeader.meshCount = pScene->GetGeometryCount();
    lHeader.unknown1 = 1;
    lHeader.unknown2 = 0;
    lHeader.bbMax = (Vec4F)bbMax;
    lHeader.bbMin = (Vec4F)bbMin;
    lHeader.boneCount = pScene->GetNodeCount() - lHeader.meshCount - 1;   //�����ڵ��� = �ڵ��� - Mesh�� - 1
    lHeader.unknown3 = 0x1;

    //д������
    fwrite(lBuffer, lSize, 1, mFilePointer);
}
//�ݹ��ȡ������Ϣ��д����Ϣ
void MSHWriter::WriteBoneData(KFbxScene* pScene, KFbxNode* pSkeletonNode)
{
    BoneData lBoneData;
    char* lBuffer = (char*)&lBoneData;
    size_t lSize = 0x140;   //sizeof(BoneData);
    KFbxXMatrix transformMatrix;
    //��ȡ��ǰ��������
    memset(lBoneData.boneName, 0, 256);  //�ַ�����������
    strcpy(lBoneData.boneName, pSkeletonNode->GetName());
    //�ⲿ����༭��FBX�ļ��Ĺ����������ַ���������
    if(lBoneData.boneName[0] == '_')
	{
        lBoneData.boneName[0] = '~';
	}
    else
	{
        for(int i = 0; lBoneData.boneName[i] != '\0'; i++)
		{
            if(lBoneData.boneName[i] == '_')
			{
                lBoneData.boneName[i] = ' ';
			}
		}
	}
    pSkeletonNode->SetName(lBoneData.boneName); //д�ص��ڵ�,����İ�������Ҫ�õ�
    //��ȡ���ڹ�������任����
    transformMatrix = pScene->GetEvaluator()->GetNodeGlobalTransform(pSkeletonNode);
    transformMatrix = transformMatrix.Inverse();
    for(int i = 0; i < 4; i++)
	{
        lBoneData.transformMatrix[i] = transformMatrix.GetRow(i);
	}

    //д��������ݵ��ļ�
    fwrite(lBuffer, lSize, 1, mFilePointer);
    for(int i = 0; i < pSkeletonNode->GetChildCount(); i++)
	{
        WriteBoneData(pScene, pSkeletonNode->GetChild(i));
    }
}

bool MSHWriter::WriteMeshData(KFbxNode* pMeshNode)
{
    KFbxMesh* pMesh = pMeshNode->GetMesh();
    if(pMesh)
	{

        //��ȡMesh��Ϣ
        MeshInfo lMeshInfo;
        memset(lMeshInfo.sceneName, 0, 256);  //�ַ�����������
        memset(lMeshInfo.meshName, 0, 256);
        memset(lMeshInfo.empty, 0, 512 - 16);
        strcpy(lMeshInfo.sceneName, "Scene Root");
        strcpy(lMeshInfo.meshName, pMeshNode->GetName());
        lMeshInfo.vertexCount = pMesh->GetControlPointsCount(); //������
        lMeshInfo.faceIndexCount = pMesh->GetPolygonVertexCount();   //������������
        lMeshInfo.unknown = 0x1;
        lMeshInfo.renderMode = 0x100;   //GL_TRIANGLES

        //��ȡMesh����
        int boneCount;                  //������,Ҫ��Ϊ��������д�뵽Mesh����β��
        int* originFaceIndex;           //��������
        KFbxVector4* originVertexData;  //��������
        KFbxLayerElementArrayTemplate<KFbxVector4>* originNormalData;  //��������
        KFbxLayerElementArrayTemplate<KFbxVector2>* originUVData;     //UV����
        KFbxLayerElementArrayTemplate<int>* originUVIndex;
        KFbxGeometry* pMeshAttribute;
        KFbxSkin* pSkin;                //��Ƥ,Ȩ������
        originFaceIndex = pMesh->GetPolygonVertices();
        originVertexData = pMesh->GetControlPoints();
        pMesh->ComputeVertexNormals();  //��Ҫ�Ⱥϲ�һ�·���
        for(int i = 0; i < pMesh->GetLayerCount(); i++)//��ȡ����,UV����
		{    
            if(pMesh->GetLayer(i)->GetNormals())
			{
                originNormalData = &pMesh->GetLayer(i)->GetNormals()->GetDirectArray();
			}
            if(pMesh->GetLayer(i)->GetUVs())
			{
                originUVData = &pMesh->GetLayer(i)->GetUVs()->GetDirectArray();
                originUVIndex = &pMesh->GetLayer(i)->GetUVs()->GetIndexArray();
            }
        }
        pMeshAttribute = (KFbxGeometry*)pMeshNode->GetNodeAttribute();
        pSkin = (KFbxSkin*)pMeshAttribute->GetDeformer(0, KFbxDeformer::eSKIN);
        boneCount = pSkin->GetClusterCount();

        //ȷ�����ݿ�(����)��С
        size_t miSize, fiSize, vdSize, ndSize, uvSize, wiSize, wdSize, bnSize;
        miSize = 0x400;
        fiSize = lMeshInfo.faceIndexCount * 0x2;
        vdSize = lMeshInfo.vertexCount * 0xC;
        ndSize = lMeshInfo.vertexCount * 0xC;
        uvSize = lMeshInfo.vertexCount * 0x8;
        wiSize = lMeshInfo.vertexCount * 4 * 0x2;
        wdSize = lMeshInfo.vertexCount * 4 * 0x4;
        bnSize = pSkin->GetClusterCount() * 0x100;
        //ΪMesh���ݴ���������
        unsigned short* faceIndex = (unsigned short*)malloc(fiSize);
        Vec3F* vertexData = (Vec3F*)malloc(vdSize);
        Vec3F* normalData = (Vec3F*)malloc(ndSize);
        Vec2F* uvData = (Vec2F*)malloc(uvSize);
        unsigned short* weightIndex = (unsigned short*)malloc(wiSize);
        float* weightData = (float*)malloc(wdSize);
        char (*boneName)[256] = (char(*)[256])malloc(bnSize);
        //��ջ�����
        memset(faceIndex, 0, fiSize);
        memset(vertexData, 0, vdSize);
        memset(normalData, 0, ndSize);
        memset(uvData, 0, uvSize);
        memset(weightIndex, 0, wiSize);
        memset(weightData, 0, wdSize);
        memset(boneName, 0, bnSize);

        //ת�����ݵ�������
        //��������
        for(int i = 0; i < lMeshInfo.faceIndexCount; i++)
		{
            faceIndex[i] = originFaceIndex[i];
		}
        //UV����,By_Polygon_Vertexģʽ
        for(int i = 0, j = 0; j < pMesh->GetPolygonCount(); j++)
		{
            for(int k = 0 ; k < 3; k++)
			{
                uvData[faceIndex[i]] = (Vec2F)(*originUVData)[(*originUVIndex)[j * 3 + k]];
                uvData[faceIndex[i]].y = 1 - uvData[faceIndex[i]].y;  //��ֱ��תUV
                i++;
            }
		}
        //�������� & ��������
        for(int i = 0; i < lMeshInfo.vertexCount; i++)
		{
            vertexData[i] = (Vec3F)originVertexData[i];
            normalData[i] = (Vec3F)(*originNormalData)[i];
        }
        //�������� & Ȩ��
        for(int i = 0; i < pSkin->GetClusterCount(); i++)
		{
            for(int j = 0; j < pSkin->GetCluster(i)->GetControlPointIndicesCount(); j++)
			{
                int k = pSkin->GetCluster(i)->GetControlPointIndices()[j];
                for(int x = 0; x < 4; x++)
				{
                    if(weightData[4 * k + x] == 0)//�ж������Ƿ�ռ��
					{ 
                        weightIndex[4 * k + x] = i;
                        weightData[4 * k + x] = pSkin->GetCluster(i)->GetControlPointWeights()[j];
                        break;
                    }
				}
            }
            strcpy(boneName[i], pSkin->GetCluster(i)->GetLink()->GetName()); //������������
        }

        //д�뻺�������ݵ��ļ�
        fwrite((char*)&lMeshInfo, miSize, 1, mFilePointer);  //д��Mesh��Ϣ
        fwrite((char*)faceIndex, fiSize, 1, mFilePointer);
        fwrite((char*)vertexData, vdSize, 1, mFilePointer);
        fwrite((char*)normalData, ndSize, 1, mFilePointer);
        fwrite((char*)uvData, uvSize, 1, mFilePointer);
        fwrite((char*)weightIndex, wiSize, 1, mFilePointer);
        fwrite((char*)weightData, wdSize, 1, mFilePointer);
        fwrite((char*)&boneCount, 0x4, 1, mFilePointer);
        fwrite((char*)boneName, bnSize, 1, mFilePointer);

        return true;
    }
    return false;
}