#include "Mcustom.h"
#include "MSHWriter.h"
#include "MSHReader.h"

KFbxWriter* CreateMSHWriter(KFbxSdkManager& pManager, KFbxExporter& pExporter, int pSubID, int pPluginID);
void* GetMSHWriterInfo(KFbxWriter::KInfoRequest pRequest, int pId);
void FillMSHWriterIOSettings(KFbxIOSettings& pIOS);

KFbxReader* CreateMSHReader(KFbxSdkManager& pManager, KFbxImporter& pImporter, int pSubID, int pPluginID);
void *GetMSHReaderInfo(KFbxReader::KInfoRequest pRequest, int pId);
void FillMSHReaderIOSettings(KFbxIOSettings& pIOS);

