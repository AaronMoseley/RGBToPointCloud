#ifndef POINTCLOUDAPP_CAMERAMANAGER_H
#define POINTCLOUDAPP_CAMERAMANAGER_H

#include "Objects/ObjectComponent.h"
#include "Widgets/ImageSourceSettingsDialog.h"
#include "Widgets/ImageSourceManagementWidget.h"
#include <filesystem>

#include "Vulkan Interface/VulkanCommonFunctions.h"

class ImageSourceManager : public ObjectComponent {
public:
	ImageSourceManager() {}

	void Start() override;
	void Update(float deltaTime) override;

	void TriggerImageSourceEditing(VulkanCommonFunctions::ObjectHandle cameraObjectHandle);
	void TriggerImageSourceRemoval(VulkanCommonFunctions::ObjectHandle cameraObjectHandle);

private:
	void AddCameraManagementWidget();

	void TriggerImageSourceSettingsDialog(VulkanCommonFunctions::ObjectHandle cameraObjectHandle);
	void TriggerImageSourceSettingsDialog(const ImageSourceSettingsDialog::ImageSourceSettingsData& imageSourceData);

	void AddCamera();

	std::map<VulkanCommonFunctions::ObjectHandle, ImageSourceSettingsDialog::ImageSourceSettingsData> m_imageSettingsData;
	ImageSourceManagementWidget* m_imageSourceManagementWidget = nullptr;
};



#endif
