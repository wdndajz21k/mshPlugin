#pragma once
#include <fbxsdk.h>
using namespace std;
_declspec(dllimport) class CmshPlugin 
{
public:
	CmshPlugin(const char*);
	~CmshPlugin();
	void ReadMeshFromFile(HWND);
	void WriteMeshToFile(HWND);
	KFbxSdkManager *GetKFbxSdkManager();
};