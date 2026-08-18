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

	void SetProcessImagesCallback(const std::function<void()>& callback) { m_processImagesCallback = callback; }
	void SetLASExportCallback(const std::function<void(std::string)>& callback) { m_exportLASCallback = callback; }

	void SetButtonsEnabled(bool enabled) const
	{
		m_generateCloudButton->setEnabled(enabled);
		m_exportLASButton->setEnabled(enabled);
	}

	bool IsProcessImagesButtonEnabled() const { return m_generateCloudButton->isEnabled() && m_exportLASButton->isEnabled(); }

private:
	void InitializeWidget();

	void EditClicked(VulkanCommonFunctions::ObjectHandle cameraObjectHandle);
	void RemoveClicked(VulkanCommonFunctions::ObjectHandle cameraObjectHandle);

	void ProcessImagesClicked();
	void ProcessLASExportClicked();

	std::map<VulkanCommonFunctions::ObjectHandle, QHBoxLayout*> m_scrollAreaElements;
	std::map<VulkanCommonFunctions::ObjectHandle, QLabel*> m_scrollAreaNameLabels;

	std::function<void(VulkanCommonFunctions::ObjectHandle)> m_editCallback = nullptr;
	std::function<void(VulkanCommonFunctions::ObjectHandle)> m_removalCallback = nullptr;

	std::function<void()> m_processImagesCallback = nullptr;
	std::function<void(const std::string&)> m_exportLASCallback = nullptr;

	QPushButton* m_generateCloudButton = nullptr;
	QPushButton* m_exportLASButton = nullptr;

	QVBoxLayout* m_scrollLayout = nullptr;
};



#endif
