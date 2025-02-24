#include "MetaFileCreator.h"

#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include "FileSystem.h"  // Assuming you have a file system utility

#include <Libs/rapidjson/document.h>
#include <Libs/rapidjson/writer.h>
#include <Libs/rapidjson/stringbuffer.h>

void CreateMetaFile(const std::string& assetPath, UID assetUID)
{
    // Get last modified time
    time_t lastModified = FileSystem::GetLastModifiedTime(assetPath);


    std::string uidStr = std::to_string(assetUID);

    // Create a RapidJSON document
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    // Add metadata fields
    doc.AddMember("UID", assetUID, allocator);
    doc.AddMember("lastModifiedDate", static_cast<uint64_t>(lastModified), allocator);


    if (uidStr.starts_with("13")) importerType = "Mesh";
    else if (uidStr.starts_with("12")) importerType = "Material";
    else if (uidStr.starts_with("11")) importerType = "Texture";
    else importerType = "Invalid type";

    // Create importer-specific options
    rapidjson::Value importOptions(rapidjson::kObjectType);

    if (importerType == "Mesh")
    {
        importOptions.AddMember("generateTangents", true, allocator);
        importOptions.AddMember("optimizeMesh", true, allocator);
    }
    else if (importerType == "Texture")
    {
        importOptions.AddMember("mipmaps", true, allocator);
        importOptions.AddMember("compression", rapidjson::Value("BC7", allocator), allocator);
    }
    else if (importerType == "Material")
    {
        importOptions.AddMember("shader", rapidjson::Value("DefaultLit", allocator), allocator);
        importOptions.AddMember("useAlpha", false, allocator);
    }

    // Add import options to the document
    doc.AddMember("importOptions", importOptions, allocator);

    // Convert document to string
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    // Save JSON to .meta file
    std::string metaFilePath = assetPath + ".meta";
    if (FileSystem::Save(metaFilePath.c_str(), buffer.GetString(), buffer.GetSize(), true))
    {
        printf("Meta file created: %s\n", metaFilePath.c_str());
    }
    else
    {
        printf("Failed to create meta file: %s\n", metaFilePath.c_str());
    }
}