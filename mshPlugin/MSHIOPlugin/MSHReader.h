
/*
MSH读取插件for FBX SDK
*/
class MSHReader : public KFbxReader
{
public:
    MSHReader(KFbxSdkManager &pFbxSdkManager, int pID);
    virtual ~MSHReader();
    virtual void GetVersion(int& pMajor, int& pMinor, int& pRevision) const;
    virtual bool FileOpen(char* pFileName);
    virtual bool FileClose();
    virtual bool IsFileOpen();
    virtual bool GetReadOptions(bool pParseFileAsNeeded = true);
    virtual bool Read(KFbxDocument* pDocument);

    //自定义函数声明
    virtual char* MSHReader::InitialPointer(char*, char*&, MshHeader*&);
    virtual char* MSHReader::InitialMeshPointer(char*, MeshInfo*&, MeshDataPointer*);
    virtual KFbxNode* MSHReader::CreateMesh(KFbxScene*, MeshInfo*, MeshDataPointer*);
    virtual void MSHReader::CreateSkeleton(KFbxScene*, MshHeader*, BoneData*);
    virtual KFbxCluster** MSHReader::LinkSkeleton(KFbxNode*, KFbxNode*, MeshDataPointer*);
    virtual void MSHReader::AddWeight(KFbxCluster**, MeshInfo*, MeshDataPointer*);
    virtual void MSHReader::StoreBindPose(KFbxScene*, KFbxNode*, KFbxCluster**, MeshDataPointer*);
private:
    FILE *mFilePointer;
    KFbxSdkManager *mManager;
};



