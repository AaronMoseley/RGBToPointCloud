#include "ImageSourceSettingsDialog.h"

#include <QPushButton>

void ImageSourceSettingsDialog::InitializeDialog()
{
	QVBoxLayout* mainLayout = new QVBoxLayout();
	setLayout(mainLayout);

	mainLayout->addWidget(new QLabel("Camera Settings:"));

	QHBoxLayout* cameraNameLayout = new QHBoxLayout();
	mainLayout->addLayout(cameraNameLayout);
	cameraNameLayout->addWidget(new QLabel("Camera Name: "));
	m_cameraNameLineEdit = new QLineEdit();
	cameraNameLayout->addWidget(m_cameraNameLineEdit);
	m_cameraNameLineEdit->setReadOnly(false);

	QHBoxLayout* imageSourceLayout = new QHBoxLayout();
	mainLayout->addLayout(imageSourceLayout);
	QPushButton* fileSourceSelectorButton = new QPushButton("Image File:");
	connect(fileSourceSelectorButton, &QPushButton::clicked, this, &ImageSourceSettingsDialog::SelectImageSourceFile);
	imageSourceLayout->addWidget(fileSourceSelectorButton);
	m_fileSourceLineEdit = new QLineEdit();
	imageSourceLayout->addWidget(m_fileSourceLineEdit);

	QDoubleValidator* validator = new QDoubleValidator(0.01, 999.99, 2);
	QDoubleValidator* positionValidator = new QDoubleValidator(-10000.0, 10000.0, 2);
	QDoubleValidator* rotationValidator = new QDoubleValidator(0.0, 360.0, 2);
	QDoubleValidator* fovValidator = new QDoubleValidator(0.01, 180.0, 2);

	QHBoxLayout* verticalFOVLayout = new QHBoxLayout();
	mainLayout->addLayout(verticalFOVLayout);
	verticalFOVLayout->addWidget(new QLabel("Vertical FOV (Degrees): "));
	m_verticalFOVLineEdit = new QLineEdit("90");
	m_verticalFOVLineEdit->setValidator(fovValidator);
	verticalFOVLayout->addWidget(m_verticalFOVLineEdit);

	QHBoxLayout* horizontalFOVLayout = new QHBoxLayout();
	mainLayout->addLayout(horizontalFOVLayout);
	horizontalFOVLayout->addWidget(new QLabel("Horizontal FOV (Degrees): "));
	m_horizontalFOVLineEdit = new QLineEdit("90");
	m_horizontalFOVLineEdit->setValidator(fovValidator);
	horizontalFOVLayout->addWidget(m_horizontalFOVLineEdit);

	QHBoxLayout* imageGlobalScaleLayout = new QHBoxLayout();
	mainLayout->addLayout(imageGlobalScaleLayout);
	imageGlobalScaleLayout->addWidget(new QLabel("Image Distance Scale: "));
	m_imageGlobalScaleLineEdit = new QLineEdit("1.0");
	m_imageGlobalScaleLineEdit->setValidator(validator);
	imageGlobalScaleLayout->addWidget(m_imageGlobalScaleLineEdit);

	QHBoxLayout* positionLayout = new QHBoxLayout();
	mainLayout->addLayout(positionLayout);
	positionLayout->addWidget(new QLabel("X: "));
	m_xPositionLineEdit = new QLineEdit();
	m_xPositionLineEdit->setValidator(positionValidator);
	positionLayout->addWidget(m_xPositionLineEdit);

	positionLayout->addWidget(new QLabel("Y: "));
	m_yPositionLineEdit = new QLineEdit();
	m_yPositionLineEdit->setValidator(positionValidator);
	positionLayout->addWidget(m_yPositionLineEdit);

	positionLayout->addWidget(new QLabel("Z: "));
	m_zPositionLineEdit = new QLineEdit();
	m_zPositionLineEdit->setValidator(positionValidator);
	positionLayout->addWidget(m_zPositionLineEdit);

	QHBoxLayout* rotationLayout = new QHBoxLayout();
	mainLayout->addLayout(rotationLayout);
	rotationLayout->addWidget(new QLabel("Pitch: "));
	m_pitchLineEdit = new QLineEdit();
	m_pitchLineEdit->setValidator(rotationValidator);
	rotationLayout->addWidget(m_pitchLineEdit);

	rotationLayout->addWidget(new QLabel("Yaw: "));
	m_yawLineEdit = new QLineEdit();
	m_yawLineEdit->setValidator(rotationValidator);
	rotationLayout->addWidget(m_yawLineEdit);

	rotationLayout->addWidget(new QLabel("Roll: "));
	m_rollLineEdit = new QLineEdit();
	m_rollLineEdit->setValidator(rotationValidator);
	rotationLayout->addWidget(m_rollLineEdit);

	QHBoxLayout* buttonLayout = new QHBoxLayout();
	mainLayout->addLayout(buttonLayout);
	QPushButton* cancelButton = new QPushButton("Cancel");
	buttonLayout->addWidget(cancelButton);
	connect(cancelButton, &QPushButton::clicked, this, &ImageSourceSettingsDialog::CancelPressed);

	QPushButton* saveButton = new QPushButton("Save");
	buttonLayout->addWidget(saveButton);
	connect(saveButton, &QPushButton::clicked, this, &ImageSourceSettingsDialog::SavePressed);
}

void ImageSourceSettingsDialog::SelectImageSourceFile()
{
	QString filePath = QFileDialog::getOpenFileName(
		this,
		"Open File",
		QDir::homePath(),                // Initial directory
		"All Files (*)"
	);

	if (filePath.isEmpty())
	{
		return;
	}

	m_fileSourceLineEdit->setText(filePath);
}

void ImageSourceSettingsDialog::LoadImageSourceData(ImageSourceSettingsData& outData) const
{
	outData.m_cameraName = m_cameraNameLineEdit->text().toStdString();
	outData.m_imageSourcePath = std::filesystem::path(m_fileSourceLineEdit->text().toStdWString());
	outData.m_verticalFOV = std::stof(m_verticalFOVLineEdit->text().toStdString());
	outData.m_horizontalFOV = std::stof(m_horizontalFOVLineEdit->text().toStdString());
	outData.m_imageGlobalScale = std::stof(m_imageGlobalScaleLineEdit->text().toStdString());

	glm::vec3 outputPosition =
	{
		m_xPositionLineEdit->text().toFloat(),
		m_yPositionLineEdit->text().toFloat(),
		m_zPositionLineEdit->text().toFloat()
	};
	outData.m_position = outputPosition;

	glm::vec3 outputRotation =
	{
		m_pitchLineEdit->text().toFloat(),
		m_yawLineEdit->text().toFloat(),
		m_rollLineEdit->text().toFloat()
	};
	outData.m_rotation = outputRotation;
}

void ImageSourceSettingsDialog::CancelPressed()
{
	m_savePressed = false;
	this->hide();
}

void ImageSourceSettingsDialog::SavePressed()
{
	if (std::filesystem::exists(m_fileSourceLineEdit->text().toStdString()) == false)
	{
		return;
	}

	if (m_cameraNameLineEdit->text().isEmpty())
	{
		return;
	}

	m_savePressed = true;
	this->hide();
}
