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

	m_imageTypeButtonGroup = new QButtonGroup();
	connect(m_imageTypeButtonGroup, &QButtonGroup::idClicked, this, &ImageSourceSettingsDialog::ImageTypeChanged);

	QHBoxLayout* imageTypeLayout = new QHBoxLayout();
	mainLayout->addLayout(imageTypeLayout);

	QVBoxLayout* perspectiveImageLayout = new QVBoxLayout();
	imageTypeLayout->addLayout(perspectiveImageLayout);

	QRadioButton* perspectiveButton = new QRadioButton("Perspective Image");
	m_imageTypeButtons[ImageType::Perspective] = perspectiveButton;
	perspectiveImageLayout->addWidget(perspectiveButton);
	perspectiveButton->setChecked(true);
	m_imageTypeButtonGroup->addButton(perspectiveButton, static_cast<int>(ImageType::Perspective));

	QHBoxLayout* verticalFOVLayout = new QHBoxLayout();
	perspectiveImageLayout->addLayout(verticalFOVLayout);
	verticalFOVLayout->addWidget(new QLabel("Vertical FOV (Degrees): "));
	m_verticalFOVLineEdit = new QLineEdit("90");
	m_verticalFOVLineEdit->setValidator(fovValidator);
	verticalFOVLayout->addWidget(m_verticalFOVLineEdit);

	QHBoxLayout* horizontalFOVLayout = new QHBoxLayout();
	perspectiveImageLayout->addLayout(horizontalFOVLayout);
	horizontalFOVLayout->addWidget(new QLabel("Horizontal FOV (Degrees): "));
	m_horizontalFOVLineEdit = new QLineEdit("90");
	m_horizontalFOVLineEdit->setValidator(fovValidator);
	horizontalFOVLayout->addWidget(m_horizontalFOVLineEdit);

	QVBoxLayout* orthographicImageLayout = new QVBoxLayout();
	imageTypeLayout->addLayout(orthographicImageLayout);

	QRadioButton* orthographicButton = new QRadioButton("Orthographic Image");
	m_imageTypeButtons[ImageType::Orthographic] = orthographicButton;
	orthographicImageLayout->addWidget(orthographicButton);
	orthographicButton->setChecked(false);
	m_imageTypeButtonGroup->addButton(orthographicButton, static_cast<int>(ImageType::Orthographic));

	QHBoxLayout* imageWorldHeightLayout = new QHBoxLayout();
	orthographicImageLayout->addLayout(imageWorldHeightLayout);
	imageWorldHeightLayout->addWidget(new QLabel("Image World Height: "));
	m_imageWorldHeightLineEdit = new QLineEdit("10.0");
	m_imageWorldHeightLineEdit->setValidator(validator);
	m_imageWorldHeightLineEdit->setEnabled(false);
	imageWorldHeightLayout->addWidget(m_imageWorldHeightLineEdit);

	QHBoxLayout* imageWorldWidthLayout = new QHBoxLayout();
	orthographicImageLayout->addLayout(imageWorldWidthLayout);
	imageWorldWidthLayout->addWidget(new QLabel("Image World Width: "));
	m_imageWorldWidthLineEdit = new QLineEdit("10.0");
	m_imageWorldWidthLineEdit->setValidator(validator);
	m_imageWorldWidthLineEdit->setEnabled(false);
	imageWorldWidthLayout->addWidget(m_imageWorldWidthLineEdit);

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

void ImageSourceSettingsDialog::ImageTypeChanged(int imageTypeRaw)
{
	ImageType imageType = ImageType(imageTypeRaw);

	switch (imageType)
	{
	case ImageType::Perspective:
		m_imageWorldHeightLineEdit->setEnabled(false);
		m_imageWorldWidthLineEdit->setEnabled(false);

		m_verticalFOVLineEdit->setEnabled(true);
		m_horizontalFOVLineEdit->setEnabled(true);
		break;
	case ImageType::Orthographic:
		m_imageWorldHeightLineEdit->setEnabled(true);
		m_imageWorldWidthLineEdit->setEnabled(true);

		m_verticalFOVLineEdit->setEnabled(false);
		m_horizontalFOVLineEdit->setEnabled(false);
		break;
	}
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

	outData.m_verticalFOV = m_verticalFOVLineEdit->text().toFloat();
	outData.m_horizontalFOV = m_horizontalFOVLineEdit->text().toFloat();

	outData.m_worldImageHeight = m_imageWorldHeightLineEdit->text().toFloat();
	outData.m_worldImageWidth = m_imageWorldWidthLineEdit->text().toFloat();

	outData.m_imageType = ImageType(m_imageTypeButtonGroup->checkedId());

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
