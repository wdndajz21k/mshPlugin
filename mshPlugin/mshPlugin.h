// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� MSHPLUGIN_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// MSHPLUGIN_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef MSHPLUGIN_EXPORTS
#define MSHPLUGIN_API __declspec(dllexport)
#else
#define MSHPLUGIN_API __declspec(dllimport)
#endif
// �����Ǵ� mshPlugin.dll ������
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




