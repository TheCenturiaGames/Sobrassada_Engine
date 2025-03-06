#include "MetaFile.h"

#include "FileSystem.h"
#include "Globals.h"

#include "Libs/rapidjson/prettywriter.h"
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

    rapidjson::Value pathValue;
    pathValue.SetString(assetPath.c_str(), static_cast<rapidjson::SizeType>(assetPath.length()), allocator);

    // Common metadata fields
    doc.AddMember("UID", assetUID, allocator);
    doc.AddMember("lastModifiedDate", static_cast<uint64_t>(lastModified), allocator);
    doc.AddMember("originalPath", pathValue, allocator);
    AddImportOptions(doc, allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    std::string savePath     = FileSystem::GetFilePath(assetPath);
    std::string name         = FileSystem::GetFileNameWithoutExtension(assetPath);
    // Save to .meta file
    std::string metaFilePath = savePath + name + META_EXTENSION;
    unsigned int bytesWritten =
        (unsigned int)FileSystem::Save(metaFilePath.c_str(), buffer.GetString(), (unsigned int)buffer.GetSize(), false);

    if (bytesWritten == 0) GLOG("Failed to save meta file: %s", assetPath.c_str());
}
