#include "ImageSourceManager.h"
#include "Components/Transform.h"
#include "Components/GLTFModel.h"
#include "Management/Scene.h"
#include "Widgets/AddImageSourceDialog.h"

void ImageSourceManager::Start()
{
	AddCameraManagementWidget();
}

void ImageSourceManager::Update(float deltaTime)
{
	if (GetWindowManager()->KeyPressedThisFrame(Qt::Key::Key_Escape))
	{
		GetWindowManager()->Shutdown();
	}

	if (GetWindowManager()->KeyPressedThisFrame(Qt::Key::Key_F))
	{
		AddCamera();
	}
}

void ImageSourceManager::AddCamera()
{
	std::shared_ptr<Transform> currentTransform = GetOwner()->GetComponent<Transform>();
	glm::quat currentRotation = currentTransform->GetRotationQuaternion();

	glm::vec3 currentPosition = currentTransform->GetPosition();
	glm::vec3 cameraPosition = currentPosition;
	cameraPosition += 0.5f * currentTransform->Right();
	cameraPosition -= 0.25f * currentTransform->Up();
	cameraPosition += 2.0f * currentTransform->Forward();

	std::shared_ptr<RenderObject> newCameraModel = std::make_shared<RenderObject>();
	newCameraModel->SetMaterialName("GenericObjectMaterial");
	std::shared_ptr<Transform> gltfModelTransform = newCameraModel->AddComponent<Transform>();
	std::shared_ptr<GLTFModel> gltfMesh = newCameraModel->AddComponent<GLTFModel>();
	gltfMesh->SetLit(false);
	gltfModelTransform->SetRotationQuaternion(currentRotation);
	gltfModelTransform->SetPosition(cameraPosition);
	gltfModelTransform->SetScale(glm::vec3(1.0f, 1.0f, 1.0f));

	gltfMesh->SetSourcePath("models/Camera/CameraModel.gltf");
	gltfMesh->ReverseWindingOrder();
	GetScene()->AddObject(newCameraModel);

	m_cameraObjects.push_back(newCameraModel);
	TriggerAddCameraDialog();
}

void ImageSourceManager::AddCameraManagementWidget()
{

}

void ImageSourceManager::TriggerAddCameraDialog()
{
	AddImageSourceDialog* dialog = new AddImageSourceDialog();
	dialog->show();
}