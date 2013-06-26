/*--------------------MSH自定义类型定义--------------------*/
class Vec2F
{
    public:
    float x;
    float y;
    Vec2F(){x = 0; y = 0;}
    Vec2F(KFbxVector2 &vector){x = vector.GetAt(0); y = vector.GetAt(1);}
    Vec2F(KFbxVector4 &vector){x = vector.GetAt(0); y = vector.GetAt(1);}
};
class Vec3F : public Vec2F
{
    public:
    float z;
    Vec3F() : Vec2F(){z = 0;}
    Vec3F(KFbxVector4 &vector) : Vec2F(vector){z = vector.GetAt(2);}
};
class Vec4F : public Vec3F
{
    public:
    float w;
    Vec4F() : Vec3F(){w = 0;}
    Vec4F(KFbxVector4 &vector) : Vec3F(vector){w = vector.GetAt(3);}
};




//MSH文件头信息
struct MshHeader
{
    char name[256];	//Eternity Engine Mesh File 0.1
    int version;	//MSH文件版本
    int meshCount;  //Mesh总数
    int unknown1;	//0x0
    int unknown2;	//0x1
    Vec3F bbMax;
    Vec3F bbMin;
    int boneCount;  //骨骼总数
    int unknown3;
};
//Bone数据
struct BoneData
{
    char boneName[256];
    Vec4F transformMatrix[4];
};
//Mesh基本信息
struct MeshInfo
{
	char sceneName[256];    //场景名称
	char meshName[256];     //Mesh名称
	int vertexCount;        //顶点总数
	int faceIndexCount;     //2字节的三角形索引总数
	int unknown;
	int renderMode;         // renderMode & 0x100 ? GL_TRIANGLE_STRIP : GL_TRIANGLES
	char empty[512 - 16];	//empty
};
//Mesh数据指针
struct MeshDataPointer
{
    char* pFaceIndex;   // 0x2(unsigned short) * 三角面数
    char* pVertexData;  // 0x4(float) * 3 * 顶点数
    char* pNormalData;  // 0x4(float) * 3 * 顶点数
    char* pUVData;      // 0x4(float) * 2 * 顶点数
    char* pBoneIndex;   // 0x2(unsigned short) * 4 顶点数
    char* pBoneWeight;  // 0x4(float) * 4 * 顶点数
    int*  pBoneCount;   // 当前Mesh的Bone数
    char* pBoneName;    // 0x100(char [256]) * Mesh的Bone数
};


