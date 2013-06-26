// mshPlugin.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "mshPlugin.h"
#include <CommDlg.h>







// �й��ඨ�����Ϣ������� mshPlugin.h
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
void CmshPlugin::InitialFbxSdk()//��ʼ������������
{
	if (pSdkManager)
	{
		mylog.Log("�����������Ѵ���,�������ٳ���������");
		pSdkManager->Destroy();
		pSdkManager=NULL;
	}
	//����FBXSDK������
	int lRegisteredCount;
	int lPluginId;
	pSdkManager = KFbxSdkManager::Create();
	mylog.Log("���������������ɹ�");
	KFbxIOSettings* ios = KFbxIOSettings::Create(pSdkManager, IOSROOT);
	mylog.Log("IO���ô����ɹ�");
	pSdkManager->SetIOSettings(ios);
	mylog.Log("IO���ü��سɹ�");

	//ע���Զ������(��֮��ģ�͸�ʽ)
	pSdkManager->GetIOPluginRegistry()->RegisterReader(CreateMSHReader, GetMSHReaderInfo,
		lPluginId, lRegisteredCount, FillMSHReaderIOSettings);
	mylog.Log("ע���ȡ����ɹ�");
	pSdkManager->GetIOPluginRegistry()->RegisterWriter(CreateMSHWriter, GetMSHWriterInfo,
		lPluginId, lRegisteredCount, FillMSHWriterIOSettings);
	mylog.Log("ע�ᱣ�����ɹ�");
}
void CmshPlugin::GetIOFilters(int ioSettings,char *outFilters)//������
{
	
	char str[1024]	= {0};
	int filterCount =  0;
	if(ioSettings == 0)//��ȡ�򿪹�����
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
	else//��ȡ���������
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
	
	//��'\0'�滻'|'
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

	//��ʼ���ļ�ѡ����
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
	mylog.Log("�����������ɹ�");

	switch(ioSettings)
	{
	case 0://���ļ�
		ofn.lpstrFilter     = filter;
		ofn.Flags           = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		ofn.nFilterIndex    = 6;
		GetOpenFileName(&ofn);  //��ʾ�ļ�ѡ���
		formatNum = ofn.nFilterIndex - 1;
		break;
	case 1://�����ļ�
		ofn.lpstrFilter     = filter;
		ofn.Flags           = OFN_EXPLORER | OFN_OVERWRITEPROMPT;
		GetSaveFileName(&ofn);
		formatNum = ofn.nFilterIndex - 1;
	default:
		break;
	}
}


void CmshPlugin::ReadMeshFromFile(HWND hwnd)//ִ������һ�������ڴ����Ѿ�����ģ�������ˣ�������ʽΪFBX
{
	InitialFbxSdk();
	GetFileName(hwnd,inputFile,inputFormat,0);
	bool lResult = false;

	lScene = KFbxScene::Create(pSdkManager,"");
	mylog.Log("���������ɹ�");

	//����Importer
	KFbxImporter* lImporter = KFbxImporter::Create(pSdkManager,"");
	mylog.Log("�����������ɹ�");

	//����
	mylog.Log("���ڳ�ʼ��������");
	lResult = lImporter->Initialize(inputFile, inputFormat, pSdkManager->GetIOSettings());
	
	if(!lResult)
		return;
	mylog.Log("���ڵ��볡��");
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
		mylog.Log("���ڴ�������");
		lScene = KFbxScene::Create(pSdkManager,"");
	}
	

	//����Exporter
	KFbxExporter* lExporter = KFbxExporter::Create(pSdkManager, "");



	//����
	mylog.Log("���ڳ�ʼ��");
	lResult = lExporter->Initialize(outputFile, outputFormat, pSdkManager->GetIOSettings());
	if(!lResult)
		return;
	mylog.Log("���ڵ�������");
	lResult = lExporter->Export(lScene);
	if(!lResult)
		return;
}



