#ifndef POINTCLOUDAPP_IMAGESOURCESETTINGDIALOG_H
#define POINTCLOUDAPP_IMAGESOURCESETTINGDIALOG_H

#include <filesystem>
#include "QWidget"
#include "QVBoxLayout"
#include "QLabel"
#include "QHBoxLayout"
#include "QPushButton"
#include "QLineEdit"
#include "QFileDialog"
#include "QDoubleValidator"
#include "QButtonGroup"
#include "QRadioButton"
#include "Vulkan Interface/VulkanCommonFunctions.h"

class ImageSourceSettingsDialog : public QWidget {
	Q_OBJECT
public:
	enum ImageType
	{
		None,
		Orthographic,
		Perspective
	};

	enum ImageSetting
	{
		Unknown,
		Indoor,
		Outdoor
	};

	struct ImageSourceSettingsData
	{
		std::string m_cameraName;
		std::filesystem::path m_imageSourcePath;

		ImageType m_imageType = ImageType::None;
		ImageSetting m_imageSetting = ImageSetting::Unknown;

		float m_worldImageWidth;
		float m_worldImageHeight;

		float m_horizontalFOV;
		float m_verticalFOV;

		float m_imageGlobalScale;
		VulkanCommonFunctions::ObjectHandle m_cameraObjectHandle;

		glm::vec3 m_position;
		glm::vec3 m_rotation;
	};

	ImageSourceSettingsDialog(const std::string& cameraName, const glm::vec3& position, const glm::vec3& rotation) : QWidget(nullptr)
	{
		InitializeDialog();
		m_cameraNameLineEdit->setText(cameraName.c_str());
		m_xPositionLineEdit->setText(std::to_string(position.x).c_str());
		m_yPositionLineEdit->setText(std::to_string(position.y).c_str());
		m_zPositionLineEdit->setText(std::to_string(position.z).c_str());

		m_pitchLineEdit->setText(std::to_string(rotation.x).c_str());
		m_yawLineEdit->setText(std::to_string(rotation.y).c_str());
		m_rollLineEdit->setText(std::to_string(rotation.z).c_str());
	}

	ImageSourceSettingsDialog(const ImageSourceSettingsData& cameraData) : QWidget(nullptr)
	{
		InitializeDialog();
		m_cameraNameLineEdit->setText(cameraData.m_cameraName.c_str());
		m_fileSourceLineEdit->setText(cameraData.m_imageSourcePath.c_str());
		m_verticalFOVLineEdit->setText(std::to_string(cameraData.m_verticalFOV).c_str());

		m_horizontalFOVLineEdit->setText(std::to_string(cameraData.m_horizontalFOV).c_str());
		m_imageGlobalScaleLineEdit->setText(std::to_string(cameraData.m_imageGlobalScale).c_str());

		m_imageWorldHeightLineEdit->setText(std::to_string(cameraData.m_worldImageHeight).c_str());
		m_imageWorldWidthLineEdit->setText(std::to_string(cameraData.m_worldImageWidth).c_str());

		m_xPositionLineEdit->setText(std::to_string(cameraData.m_position.x).c_str());
		m_yPositionLineEdit->setText(std::to_string(cameraData.m_position.y).c_str());
		m_zPositionLineEdit->setText(std::to_string(cameraData.m_position.z).c_str());

		m_pitchLineEdit->setText(std::to_string(cameraData.m_rotation.x).c_str());
		m_yawLineEdit->setText(std::to_string(cameraData.m_rotation.y).c_str());
		m_rollLineEdit->setText(std::to_string(cameraData.m_rotation.z).c_str());

		m_imageSettingButtons[cameraData.m_imageSetting]->setChecked(true);
		m_imageTypeButtons[cameraData.m_imageType]->setChecked(true);
		ImageTypeChanged(static_cast<int>(cameraData.m_imageType));
	}

	void LoadImageSourceData(ImageSourceSettingsData& outData) const;
	bool GetSavePressed() const { return m_savePressed; }

private:
	void InitializeDialog();

	void SelectImageSourceFile();

	void SavePressed();
	void CancelPressed();

	void ImageTypeChanged(int imageTypeRaw);

	bool m_savePressed = false;

	QButtonGroup* m_imageTypeButtonGroup = nullptr;
	QButtonGroup* m_imageSettingButtonGroup = nullptr;

	QLineEdit* m_cameraNameLineEdit = nullptr;
	QLineEdit* m_fileSourceLineEdit = nullptr;

	QLineEdit* m_imageWorldHeightLineEdit = nullptr;
	QLineEdit* m_imageWorldWidthLineEdit = nullptr;

	QLineEdit* m_verticalFOVLineEdit = nullptr;
	QLineEdit* m_horizontalFOVLineEdit = nullptr;

	QLineEdit* m_imageGlobalScaleLineEdit = nullptr;

	QLineEdit* m_xPositionLineEdit = nullptr;
	QLineEdit* m_yPositionLineEdit = nullptr;
	QLineEdit* m_zPositionLineEdit = nullptr;

	QLineEdit* m_pitchLineEdit = nullptr;
	QLineEdit* m_yawLineEdit = nullptr;
	QLineEdit* m_rollLineEdit = nullptr;

	std::map<ImageType, QRadioButton*> m_imageTypeButtons;
	std::map<ImageSetting, QRadioButton*> m_imageSettingButtons;
};



#endif
