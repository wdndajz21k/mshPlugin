// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 MSHPLUGIN_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// MSHPLUGIN_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef MSHPLUGIN_EXPORTS
#define MSHPLUGIN_API __declspec(dllexport)
#else
#define MSHPLUGIN_API __declspec(dllimport)
#endif
// 此类是从 mshPlugin.dll 导出的
class MSHPLUGIN_API CmshPlugin 
{
public:
	CmshPlugin(const char*);
	~CmshPlugin();
	void ReadMeshFromFile(HWND);
	void WriteMeshToFile(HWND);
	KFbxSdkManager *GetKFbxSdkManager(){return pSdkManager;}
protected:
	void InitialFbxSdk();
	void GetFileName(HWND,char*,int &, int);
	void GetIOFilters(int,char *);
private:
	KFbxSdkManager *pSdkManager;
	KFbxScene      *lScene;
	char *inputFile;int inputFormat;
	char *outputFile;int outputFormat;
	LOG mylog;

};




