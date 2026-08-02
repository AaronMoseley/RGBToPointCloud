#ifndef POINTCLOUDAPP_IMAGESOURCEMANAGEMENTWIDGET_H
#define POINTCLOUDAPP_IMAGESOURCEMANAGEMENTWIDGET_H

#include "QWidget"
#include "QScrollArea"
#include "QPushButton"
#include "QLabel"
#include "QHBoxLayout"
#include "QVBoxLayout"
#include "ImageSourceSettingsDialog.h"

class ImageSourceManagementWidget : public QWidget {
	Q_OBJECT
public:
	ImageSourceManagementWidget() : QWidget(nullptr)
	{
		InitializeWidget();
	}

	void AddImageSource(const ImageSourceSettingsDialog::ImageSourceSettingsData& imageSourceData);
	void ChangeCameraName(VulkanCommonFunctions::ObjectHandle cameraObjectHandle, const std::string& newName);

	void SetEditCallback(const std::function<void(VulkanCommonFunctions::ObjectHandle)>& callback) { m_editCallback = callback; }
	void SetRemovalCallback(const std::function<void(VulkanCommonFunctions::ObjectHandle)>& callback) { m_removalCallback = callback; }

private:
	void InitializeWidget();

	void EditClicked(VulkanCommonFunctions::ObjectHandle cameraObjectHandle);
	void RemoveClicked(VulkanCommonFunctions::ObjectHandle cameraObjectHandle);

	std::map<VulkanCommonFunctions::ObjectHandle, QHBoxLayout*> m_scrollAreaElements;
	std::map<VulkanCommonFunctions::ObjectHandle, QLabel*> m_scrollAreaNameLabels;

	std::function<void(VulkanCommonFunctions::ObjectHandle)> m_editCallback;
	std::function<void(VulkanCommonFunctions::ObjectHandle)> m_removalCallback;

	QVBoxLayout* m_scrollLayout = nullptr;
};



#endif
