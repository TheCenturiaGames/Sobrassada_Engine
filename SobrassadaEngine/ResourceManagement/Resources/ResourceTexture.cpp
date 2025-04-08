#include "ResourceTexture.h"
#include "Geometry/Frustum.h"
#include "glew.h"
#include "Framebuffer.h"

ResourceTexture::ResourceTexture(uint64_t uid, const std::string& name) : Resource(uid, name, ResourceType::Texture)
{
}

ResourceTexture::~ResourceTexture()
{
}

void ResourceTexture::LoadData(const DirectX::TexMetadata& metadata, const DirectX::ScratchImage& scratchImage)
{
    this->metadata = metadata;
    // this->scratchImage = scratchImage;
    ConvertMetadata(this->metadata, openGLMetadata);

    if (IsCubemap())
    {
        assert(metadata.arraySize == 6);

        for (int i = 0; i < 6; ++i)
        {
            glGenTextures(1, &m_CubemapFaceIDs[i]);
            glBindTexture(GL_TEXTURE_2D, m_CubemapFaceIDs[i]);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            const DirectX::Image* faceImg = scratchImage.GetImage(0, i, 0);
            if (faceImg)
            {
                glTexImage2D(
                    GL_TEXTURE_2D, 0, openGLMetadata.internalFormat, static_cast<GLsizei>(faceImg->width),
                    static_cast<GLsizei>(faceImg->height), 0, openGLMetadata.format, openGLMetadata.type,
                    faceImg->pixels
                );
            }
            else
            {
                GLOG("Error loading cubemap face %d", i);
            }
        }
    }
}

void ResourceTexture::ConvertMetadata(const DirectX::TexMetadata& metadata, OpenGLMetadata& outMetadata)
{
    switch (metadata.format)
    {
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_R8G8B8A8_UNORM:
        outMetadata.internalFormat = GL_RGBA8;
        outMetadata.format         = GL_RGBA;
        outMetadata.type           = GL_UNSIGNED_BYTE;
        break;
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
    case DXGI_FORMAT_B8G8R8A8_UNORM:
        outMetadata.internalFormat = GL_RGBA8;
        outMetadata.format         = GL_BGRA;
        outMetadata.type           = GL_UNSIGNED_BYTE;
        break;
    case DXGI_FORMAT_B5G6R5_UNORM:
        outMetadata.internalFormat = GL_RGB8;
        outMetadata.format         = GL_BGR;
        outMetadata.type           = GL_UNSIGNED_BYTE;
        break;
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
        outMetadata.internalFormat = GL_DEPTH24_STENCIL8;
        outMetadata.format         = GL_DEPTH_STENCIL;
        outMetadata.type           = GL_UNSIGNED_INT_24_8;
        break;
    default:
        assert(false && "Unsupported format");
    }
}

unsigned int ResourceTexture::GetCubemapFaceID(int faceIndex) const
{
    assert(faceIndex >= 0 && faceIndex < 6);
    return m_CubemapFaceIDs[faceIndex];
}

void ResourceTexture::CubeMapToTexture(int width, int height)
{
    const float3 front[6] = {float3::unitX,  -float3::unitX, float3::unitY,
                             -float3::unitY, float3::unitZ,  -float3::unitZ};
    float3 up[6] = {-float3::unitY, -float3::unitY, float3::unitZ, -float3::unitZ, -float3::unitY, -float3::unitY};
    Frustum frustum;
    frustum.type              = FrustumType::PerspectiveFrustum;
    frustum.pos               = float3::zero;
    frustum.nearPlaneDistance = 0.1f;
    frustum.farPlaneDistance  = 100.0f;
    frustum.verticalFov       = PI / 2.0f;
    frustum.horizontalFov     = PI / 2.0f;
    // TODO: Create and Bind Frame Buffer and Create Irradiance Cubemap
    Framebuffer framebuffer(width, height, false);
    framebuffer.Bind();
    for (int i = 0; i < 6; ++i)
    {
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, GetCubemapFaceID(i), 0
        );
        frustum.front = front[i];
        frustum.up    = up[i];

        // TODO: Draw 2x2x2 Cube using frustum view and projection matrices
    }
}