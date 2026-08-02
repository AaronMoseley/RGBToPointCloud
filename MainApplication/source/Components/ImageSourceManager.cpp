#include "ImageSourceManager.h"

#include <qcoreapplication.h>

#include "Components/Transform.h"
#include "Components/GLTFModel.h"
#include "Management/Scene.h"
#include "Widgets/ImageSourceSettingsDialog.h"

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
	VulkanCommonFunctions::ObjectHandle cameraHandle = GetScene()->AddObject(newCameraModel);

	TriggerImageSourceSettingsDialog(cameraHandle);
}

void ImageSourceManager::AddCameraManagementWidget()
{
	m_imageSourceManagementWidget = new ImageSourceManagementWidget();

	std::function<void(VulkanCommonFunctions::ObjectHandle)> editCallback = std::bind(&ImageSourceManager::TriggerImageSourceEditing, this, std::placeholders::_1);
	std::function<void(VulkanCommonFunctions::ObjectHandle)> removeCallback = std::bind(&ImageSourceManager::TriggerImageSourceRemoval, this, std::placeholders::_1);

	m_imageSourceManagementWidget->SetEditCallback(editCallback);
	m_imageSourceManagementWidget->SetRemovalCallback(removeCallback);

	GetWindowManager()->AddWidgetToMenu("ImageSourceManager", m_imageSourceManagementWidget);
}

void ImageSourceManager::TriggerImageSourceEditing(VulkanCommonFunctions::ObjectHandle cameraObjectHandle)
{
	if (m_imageSettingsData.contains(cameraObjectHandle) == false)
	{
		return;
	}

	TriggerImageSourceSettingsDialog(m_imageSettingsData[cameraObjectHandle]);
}

void ImageSourceManager::TriggerImageSourceRemoval(VulkanCommonFunctions::ObjectHandle cameraObjectHandle)
{
	m_imageSettingsData.erase(cameraObjectHandle);
	GetScene()->RemoveObject(cameraObjectHandle);
}

void ImageSourceManager::TriggerImageSourceSettingsDialog(VulkanCommonFunctions::ObjectHandle cameraObjectHandle)
{
	std::shared_ptr<RenderObject> cameraObject = GetScene()->GetRenderObject(cameraObjectHandle);
	std::shared_ptr<Transform> cameraTransform = cameraObject->GetComponent<Transform>();

	std::string cameraName = "Camera_" + std::to_string(m_imageSettingsData.size());
	ImageSourceSettingsDialog* dialog = new ImageSourceSettingsDialog(cameraName, cameraTransform->GetWorldPosition(), cameraTransform->GetRotation());
	dialog->show();

	while (dialog->isVisible())
	{
		QCoreApplication::processEvents();
	}

	if (dialog->GetSavePressed())
	{
		ImageSourceSettingsDialog::ImageSourceSettingsData imageSourceData;
		dialog->LoadImageSourceData(imageSourceData);
		imageSourceData.m_cameraObjectHandle = cameraObjectHandle;

		cameraTransform->SetPosition(imageSourceData.m_position);
		cameraTransform->SetRotation(imageSourceData.m_rotation);
		m_imageSourceManagementWidget->AddImageSource(imageSourceData);
		m_imageSettingsData[cameraObjectHandle] = imageSourceData;
	} else
	{
		GetScene()->RemoveObject(cameraObjectHandle);
		m_imageSettingsData.erase(cameraObjectHandle);
	}
}

void ImageSourceManager::TriggerImageSourceSettingsDialog(const ImageSourceSettingsDialog::ImageSourceSettingsData& imageSourceData)
{
	ImageSourceSettingsDialog* dialog = new ImageSourceSettingsDialog(imageSourceData);
	dialog->show();

	std::shared_ptr<Transform> cameraTransform = GetScene()->GetRenderObject(imageSourceData.m_cameraObjectHandle)->GetComponent<Transform>();

	while (dialog->isVisible())
	{
		QCoreApplication::processEvents();
	}

	if (dialog->GetSavePressed())
	{
		ImageSourceSettingsDialog::ImageSourceSettingsData newImageSourceData;
		dialog->LoadImageSourceData(newImageSourceData);

		cameraTransform->SetPosition(imageSourceData.m_position);
		cameraTransform->SetRotation(imageSourceData.m_rotation);
		m_imageSourceManagementWidget->ChangeCameraName(imageSourceData.m_cameraObjectHandle, newImageSourceData.m_cameraName);
		m_imageSettingsData[imageSourceData.m_cameraObjectHandle] = newImageSourceData;
	}
}