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

class ImageSourceSettingsDialog : public QWidget {
	Q_OBJECT
public:
	struct ImageSourceSettingsData
	{
		std::filesystem::path m_imageSourcePath;
		float m_horizontalFOV;
		float m_verticalFOV;
		float m_imageGlobalScale;
	};

	ImageSourceSettingsDialog() : QWidget(nullptr)
	{
		InitializeDialog();
	}

	void LoadImageSourceData(ImageSourceSettingsData& outData) const;
	bool GetSavePressed() const { return m_savePressed; }

private:
	void InitializeDialog();

	void SelectImageSourceFile();

	void SavePressed();
	void CancelPressed();

	bool m_savePressed = false;

	QLineEdit* m_fileSourceLineEdit = nullptr;
	QLineEdit* m_verticalFOVLineEdit = nullptr;
	QLineEdit* m_horizontalFOVLineEdit = nullptr;
	QLineEdit* m_imageGlobalScaleLineEdit = nullptr;
};



#endif
