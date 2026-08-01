#ifndef POINTCLOUDAPP_CAMERAMANAGER_H
#define POINTCLOUDAPP_CAMERAMANAGER_H

#include "Objects/ObjectComponent.h"
#include "Widgets/ImageSourceSettingsDialog.h"
#include <filesystem>

#include "Vulkan Interface/VulkanCommonFunctions.h"

class ImageSourceManager : public ObjectComponent {
public:
	ImageSourceManager() {}

	void Start() override;
	void Update(float deltaTime) override;

private:
	void AddCameraManagementWidget();
	void TriggerImageSourceSettingsDialog(VulkanCommonFunctions::ObjectHandle cameraObjectHandle, bool deleteOnCancel);

	void AddCamera();

	std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>> m_cameraObjects;
	std::map<VulkanCommonFunctions::ObjectHandle, ImageSourceSettingsDialog::ImageSourceSettingsData> m_imageSettingsData;
};



#endif
