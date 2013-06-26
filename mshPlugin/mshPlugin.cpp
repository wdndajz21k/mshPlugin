// mshPlugin.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "mshPlugin.h"
#include <CommDlg.h>







// 有关类定义的信息，请参阅 mshPlugin.h
CmshPlugin::CmshPlugin(const char* logFileName):mylog(logFileName)
{
	pSdkManager = NULL;
	lScene		= NULL;
	inputFile	= new char[1024];
	outputFile	= new char[1024];
}
CmshPlugin::~CmshPlugin()
{
	if (inputFile)
	{
		delete inputFile;
		inputFile = NULL;
	}
	if (outputFile)
	{
		delete outputFile;
		outputFile = NULL;
	}
	if (pSdkManager)
	{
		pSdkManager->Destroy();
		pSdkManager = NULL;
	}
}
void CmshPlugin::InitialFbxSdk()//初始化场景管理器
{
	if (pSdkManager)
	{
		mylog.Log("场景管理器已存在,正在销毁场景管理器");
		pSdkManager->Destroy();
		pSdkManager=NULL;
	}
	//创建FBXSDK管理器
	int lRegisteredCount;
	int lPluginId;
	pSdkManager = KFbxSdkManager::Create();
	mylog.Log("场景管理器创建成功");
	KFbxIOSettings* ios = KFbxIOSettings::Create(pSdkManager, IOSROOT);
	mylog.Log("IO设置创建成功");
	pSdkManager->SetIOSettings(ios);
	mylog.Log("IO设置加载成功");

	//注册自定义组件(龙之谷模型格式)
	pSdkManager->GetIOPluginRegistry()->RegisterReader(CreateMSHReader, GetMSHReaderInfo,
		lPluginId, lRegisteredCount, FillMSHReaderIOSettings);
	mylog.Log("注册读取插件成功");
	pSdkManager->GetIOPluginRegistry()->RegisterWriter(CreateMSHWriter, GetMSHWriterInfo,
		lPluginId, lRegisteredCount, FillMSHWriterIOSettings);
	mylog.Log("注册保存插件成功");
}
void CmshPlugin::GetIOFilters(int ioSettings,char *outFilters)//过滤器
{
	
	char str[1024]	= {0};
	int filterCount =  0;
	if(ioSettings == 0)//获取打开过滤器
	{ 
		
		filterCount = pSdkManager->GetIOPluginRegistry()->GetReaderFormatCount();
		for(int i = 0; i < filterCount; i++)
		{   
			strcat(str,pSdkManager->GetIOPluginRegistry()->GetReaderFormatDescription(i));
			strcat(str,"|*.");
			strcat(str,pSdkManager->GetIOPluginRegistry()->GetReaderFormatExtension(i));
			strcat(str,"|");
		}
	}
	else//获取保存过滤器
	{
		filterCount = pSdkManager->GetIOPluginRegistry()->GetWriterFormatCount();
		for(int i = 0; i < filterCount; i++)
		{
			strcat(str,pSdkManager->GetIOPluginRegistry()->GetWriterFormatDescription(i)); 
			strcat(str,"|*.");
			strcat(str,pSdkManager->GetIOPluginRegistry()->GetWriterFormatExtension(i));
			strcat(str,"|");
		}
	}
	
	//用'\0'替换'|'
	int nbChar   = strlen(str)+1;
	strcpy(outFilters,str);
	for(int i = 0; i < nbChar; i++)
	{
		if(outFilters[i] == '|')
		{
			outFilters[i] = '\0';
		}
	}

}

//ioSettings:
//0 - inputFile
//1 - outputFile
void CmshPlugin::GetFileName(HWND hwnd,
				 char* szFile,
				 int &formatNum,
				 int ioSettings)
{
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ZeroMemory(szFile, MAX_PATH);

	//初始化文件选区框
	ofn.lStructSize     = sizeof(ofn);
	ofn.hwndOwner       = hwnd;
	ofn.lpstrFile       = szFile;
	ofn.nMaxFile        = MAX_PATH;
	ofn.lpstrFileTitle  = NULL;
	ofn.nMaxFileTitle   = 0;
	ofn.nFilterIndex    = 1;
	ofn.lpstrInitialDir = NULL;

	char filter[1024];
	GetIOFilters(ioSettings,filter);
	mylog.Log("过滤器创建成功");

	switch(ioSettings)
	{
	case 0://读文件
		ofn.lpstrFilter     = filter;
		ofn.Flags           = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		ofn.nFilterIndex    = 6;
		GetOpenFileName(&ofn);  //显示文件选择框
		formatNum = ofn.nFilterIndex - 1;
		break;
	case 1://保存文件
		ofn.lpstrFilter     = filter;
		ofn.Flags           = OFN_EXPLORER | OFN_OVERWRITEPROMPT;
		GetSaveFileName(&ofn);
		formatNum = ofn.nFilterIndex - 1;
	default:
		break;
	}
}


void CmshPlugin::ReadMeshFromFile(HWND hwnd)//执行完这一步，在内存中已经存在模型数据了，保存形式为FBX
{
	InitialFbxSdk();
	GetFileName(hwnd,inputFile,inputFormat,0);
	bool lResult = false;

	lScene = KFbxScene::Create(pSdkManager,"");
	mylog.Log("场景创建成功");

	//创建Importer
	KFbxImporter* lImporter = KFbxImporter::Create(pSdkManager,"");
	mylog.Log("创建导入器成功");

	//导入
	mylog.Log("正在初始化导入器");
	lResult = lImporter->Initialize(inputFile, inputFormat, pSdkManager->GetIOSettings());
	
	if(!lResult)
		return;
	mylog.Log("正在导入场景");
	lResult = lImporter->Import(lScene);
	if(!lResult)
		return;

}
void CmshPlugin::WriteMeshToFile(HWND hwnd)
{
	GetFileName(hwnd,outputFile,outputFormat,1);
	bool lResult = false;

	if (!lScene)
	{
		mylog.Log("正在创建场景");
		lScene = KFbxScene::Create(pSdkManager,"");
	}
	

	//创建Exporter
	KFbxExporter* lExporter = KFbxExporter::Create(pSdkManager, "");



	//导出
	mylog.Log("正在初始化");
	lResult = lExporter->Initialize(outputFile, outputFormat, pSdkManager->GetIOSettings());
	if(!lResult)
		return;
	mylog.Log("正在导出场景");
	lResult = lExporter->Export(lScene);
	if(!lResult)
		return;
}



