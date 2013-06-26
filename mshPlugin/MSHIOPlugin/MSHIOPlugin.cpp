// Create your own writer.
// And your writer will get a pPluginID and pSubID.

#include "stdafx.h"
KFbxWriter* CreateMSHWriter(KFbxSdkManager& pManager, KFbxExporter& pExporter, int pSubID, int pPluginID)
{
    return FbxSdkNew<MSHWriter>(pManager, pPluginID);
}

// Get extension, description or version info about MSHWriter
void* GetMSHWriterInfo(KFbxWriter::KInfoRequest pRequest, int pId)
{
    static char const* sExt[] =
    {
        "MSH",
        0
    };

    static char const* sDesc[] =
    {
        "Dragon Nest Mesh Format (*.MSH)",
        0
    };

    switch (pRequest)
    {
    case KFbxWriter::eInfoExtension:
        return sExt;
    case KFbxWriter::eInfoDescriptions:
        return sDesc;
    case KFbxWriter::eInfoVersions:
        return 0;
    default:
        return 0;
    }
}

void FillMSHWriterIOSettings(KFbxIOSettings& pIOS)
{
    // Here you can write your own KFbxIOSettings and parse them.
}


// Creates a MSHReader in the Sdk Manager
KFbxReader* CreateMSHReader(KFbxSdkManager& pManager, KFbxImporter& pImporter, int pSubID, int pPluginID)
{
    return FbxSdkNew<MSHReader>(pManager, pPluginID);
}

// Get extension, description or version info about MSHReader
void *GetMSHReaderInfo(KFbxReader::KInfoRequest pRequest, int pId)
{
    switch (pRequest)
    {
    case KFbxReader::eInfoExtension:
        return GetMSHWriterInfo(KFbxWriter::eInfoExtension, pId);
    case KFbxReader::eInfoDescriptions:
        return GetMSHWriterInfo(KFbxWriter::eInfoDescriptions, pId);
    default:
        return 0;
    }
}

void FillMSHReaderIOSettings(KFbxIOSettings& pIOS)
{
    // Here you can write your own KFbxIOSettings and parse them.
}

