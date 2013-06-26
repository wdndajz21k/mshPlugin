/*
MSH写入插件for FBX SDK
*/
#include "translate.h"
class MSHWriter : public KFbxWriter
{
public:

    MSHWriter(KFbxSdkManager &pFbxSdkManager, int pID);
    virtual ~MSHWriter();
    virtual bool FileCreate(char* pFileName);
    virtual bool FileClose();
    virtual bool IsFileOpen();
    virtual void GetWriteOptions();
    virtual bool Write(KFbxDocument* pDocument);
    virtual bool PreprocessScene(KFbxScene &pScene);
    virtual bool PostprocessScene(KFbxScene &pScene);
    //自定义函数声明
    virtual void WriteHeader(KFbxScene*);
    virtual void WriteBoneData(KFbxScene*, KFbxNode*);
    virtual bool WriteMeshData(KFbxNode*);

private:
    FILE *mFilePointer;
    KFbxSdkManager *mManager;
};


