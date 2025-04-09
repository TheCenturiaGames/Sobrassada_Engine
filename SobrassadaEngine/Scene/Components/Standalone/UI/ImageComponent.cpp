#include "ImageComponent.h"

ImageComponent::ImageComponent(UID uid, GameObject* parent) : Component(uid, parent, "Image", COMPONENT_IMAGE)
{

}

ImageComponent::ImageComponent(const rapidjson::Value& initialState, GameObject* parent) : Component(initialState, parent)
{

}

ImageComponent::~ImageComponent()
{

}

void ImageComponent::Init() 
{

}
void ImageComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{


}
void ImageComponent::Clone(const Component* other)
{

}

void ImageComponent::Render(float deltaTime)
{
}

void ImageComponent::RenderEditorInspector()
{

}

void ImageComponent::RenderUI() const
{

}