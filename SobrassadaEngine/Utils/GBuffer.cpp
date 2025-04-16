#include "GBuffer.h"

#include "glew.h"

GBuffer::GBuffer(int width, int height)
{
    colorAttachments.assign(4, 0);
    colorAttachments[0] = GL_COLOR_ATTACHMENT0;
    colorAttachments[1] = GL_COLOR_ATTACHMENT1;
    colorAttachments[2] = GL_COLOR_ATTACHMENT2;
    colorAttachments[3] = GL_COLOR_ATTACHMENT3;
}

GBuffer::~GBuffer()
{
    glDeleteFramebuffers(1, &gBufferObject);

    glDeleteTextures(1, &positionTexture);
    glDeleteTextures(1, &normalTexture);
    glDeleteTextures(1, &diffuseTexture);
    glDeleteTextures(1, &specularTexture);
    glDeleteTextures(1, &depthTexture);
}

void GBuffer::InitializeGBuffer()
{
    if (gBufferObject == 0) glGenFramebuffers(1, &gBufferObject);
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferObject);

    // position texture
    if (positionTexture == 0) glGenTextures(1, &positionTexture);
    glBindTexture(GL_TEXTURE_2D, positionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
}