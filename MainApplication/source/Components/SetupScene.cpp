#include "SetupScene.h"
#include "Components/Transform.h"
#include "Components/GLTFModel.h"
#include "Management/Scene.h"

void SetupScene::Start()
{
	//add crosshair
	std::shared_ptr<RenderObject> uiImageTexture = std::make_shared<RenderObject>();
	uiImageTexture->SetMaterialName("GenericUIObjectMaterial");
	std::shared_ptr<Transform> imageTextureTransform = uiImageTexture->AddComponent<Transform>();
	imageTextureTransform->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	imageTextureTransform->SetScale(glm::vec3(0.05f, 0.05f, 1.0f));
	std::shared_ptr<UIImage> uiImageComponent = uiImageTexture->AddComponent<UIImage>();
	uiImageComponent->SetTexture("textures/Crosshair.png");
	GetScene()->AddUIObject(uiImageTexture);

	//add origin marker
	std::shared_ptr<RenderObject> originMarkerModel = std::make_shared<RenderObject>();
	originMarkerModel->SetMaterialName("GenericObjectMaterial");
	std::shared_ptr<Transform> gltfModelTransform = originMarkerModel->AddComponent<Transform>();
	std::shared_ptr<GLTFModel> gltfMesh = originMarkerModel->AddComponent<GLTFModel>();
	gltfMesh->SetLit(false);
	gltfModelTransform->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	gltfMesh->SetSourcePath("models/OriginMarker/OriginMarker.gltf");
	gltfMesh->ReverseWindingOrder();
	GetScene()->AddObject(originMarkerModel);

	//add help text
	m_helpTextObject = std::make_shared<RenderObject>();
	m_helpTextObject->SetMaterialName("GenericUIObjectMaterial");
	std::shared_ptr<Transform> uiTextObjectTransform = m_helpTextObject->AddComponent<Transform>();
	uiTextObjectTransform->SetPosition(glm::vec3(-0.95f, 0.95f, 0.0f));
	uiTextObjectTransform->SetScale(glm::vec3(1.0f, 1.0f, 1.0f));
	std::shared_ptr<Text> uiTextComponent = m_helpTextObject->AddComponent<Text>();
	uiTextComponent->SetTextString(GetHelpTextString());
	std::shared_ptr<Font> newFont = GetScene()->AddFont("fonts/jetbrainsmononl-medium.png", "fonts/jetbrainsmononl-medium.fnt");
	uiTextComponent->SetFontName("JetBrains Mono NL Medium");
	GetScene()->AddUIObject(m_helpTextObject);
}

void SetupScene::Update(float deltaTime)
{
	std::shared_ptr<Text> uiTextComponent = m_helpTextObject->GetComponent<Text>();
	uiTextComponent->SetTextString(GetHelpTextString());
}

std::string SetupScene::GetHelpTextString() const
{
	glm::vec3 position = GetOwner()->GetComponent<Transform>()->GetPosition();

	std::string result = kHelpTextCamera;
	result += "\nX: " + std::format("{:.3f}", position.x);
	result += "\nY: " + std::format("{:.3f}", position.y);
	result += "\nZ: " + std::format("{:.3f}", position.z);

	return result;
}
