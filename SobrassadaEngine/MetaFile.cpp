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


MetaMesh::MetaMesh(UID uid, const std::string& assetPath, bool generateTangents, bool optimizeMesh)
    : MetaFile(uid, assetPath), generateTangents(generateTangents), optimizeMesh(optimizeMesh)
{
    importerType = "Mesh";
}

void MetaMesh::AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const
{
    rapidjson::Value importOptions(rapidjson::kObjectType);
    importOptions.AddMember("generateTangents", generateTangents, allocator);
    importOptions.AddMember("optimizeMesh", optimizeMesh, allocator);
    doc.AddMember("importOptions", importOptions, allocator);
}


MetaMaterial::MetaMaterial(UID uid, const std::string& assetPath, const std::string& shader, bool useAlpha)
    : MetaFile(uid, assetPath), shader(shader), useAlpha(useAlpha)
{
    importerType = "Material";
}

void MetaMaterial::AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const
{
    rapidjson::Value importOptions(rapidjson::kObjectType);
    importOptions.AddMember("shader", rapidjson::Value(shader.c_str(), allocator), allocator);
    importOptions.AddMember("useAlpha", useAlpha, allocator);
    doc.AddMember("importOptions", importOptions, allocator);
}


MetaTexture::MetaTexture(UID uid, const std::string& assetPath, bool mipmaps, const std::string& compression)
    : MetaFile(uid, assetPath), mipmaps(mipmaps), compression(compression)
{
    importerType = "Texture";
}

void MetaTexture::AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const
{
    rapidjson::Value importOptions(rapidjson::kObjectType);
    importOptions.AddMember("mipmaps", mipmaps, allocator);
    importOptions.AddMember("compression", rapidjson::Value(compression.c_str(), allocator), allocator);
    doc.AddMember("importOptions", importOptions, allocator);
}