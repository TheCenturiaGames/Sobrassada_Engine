#include "MetaFile.h"
#include "FileSystem.h"
#include "Libs/rapidjson/stringbuffer.h"
#include "Libs/rapidjson/writer.h"

MetaFile::MetaFile(UID uid, const std::string& assetPath)
    : assetUID(uid), lastModified(FileSystem::GetLastModifiedTime(assetPath))
{
}

void MetaFile::Save(const std::string& assetPath) const
{
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    // Common metadata fields
    doc.AddMember("UID", assetUID, allocator);
    doc.AddMember("lastModifiedDate", static_cast<uint64_t>(lastModified), allocator);

    // Import-specific options (from derived class)
    AddImportOptions(doc, allocator);

    // Serialize JSON
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    // Save to .meta file
    std::string metaFilePath = assetPath + ".meta";
    FileSystem::Save(metaFilePath.c_str(), buffer.GetString(), buffer.GetSize(), true);
}


