#include "ImageSourceSettingsDialog.h"

#include <QPushButton>

void ImageSourceSettingsDialog::InitializeDialog()
{
	QVBoxLayout* mainLayout = new QVBoxLayout();
	setLayout(mainLayout);

	mainLayout->addWidget(new QLabel("Camera Settings:"));

	QHBoxLayout* imageSourceLayout = new QHBoxLayout();
	mainLayout->addLayout(imageSourceLayout);
	QPushButton* fileSourceSelectorButton = new QPushButton("Image File:");
	connect(fileSourceSelectorButton, &QPushButton::clicked, this, &ImageSourceSettingsDialog::SelectImageSourceFile);
	imageSourceLayout->addWidget(fileSourceSelectorButton);
	m_fileSourceLineEdit = new QLineEdit();
	imageSourceLayout->addWidget(m_fileSourceLineEdit);

	QDoubleValidator* validator = new QDoubleValidator(0.01, 999.99, 2, this);

	QHBoxLayout* verticalFOVLayout = new QHBoxLayout();
	mainLayout->addLayout(verticalFOVLayout);
	verticalFOVLayout->addWidget(new QLabel("Vertical FOV (Degrees): "));
	m_verticalFOVLineEdit = new QLineEdit("90");
	m_verticalFOVLineEdit->setValidator(validator);
	verticalFOVLayout->addWidget(m_verticalFOVLineEdit);

	QHBoxLayout* horizontalFOVLayout = new QHBoxLayout();
	mainLayout->addLayout(horizontalFOVLayout);
	horizontalFOVLayout->addWidget(new QLabel("Horizontal FOV (Degrees): "));
	m_horizontalFOVLineEdit = new QLineEdit("90");
	m_horizontalFOVLineEdit->setValidator(validator);
	horizontalFOVLayout->addWidget(m_horizontalFOVLineEdit);

	QHBoxLayout* imageGlobalScaleLayout = new QHBoxLayout();
	mainLayout->addLayout(imageGlobalScaleLayout);
	imageGlobalScaleLayout->addWidget(new QLabel("Image Distance Scale: "));
	m_imageGlobalScaleLineEdit = new QLineEdit("1.0");
	m_imageGlobalScaleLineEdit->setValidator(validator);
	imageGlobalScaleLayout->addWidget(m_imageGlobalScaleLineEdit);

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
	outData.m_imageSourcePath = std::filesystem::path(m_fileSourceLineEdit->text().toStdWString());
	outData.m_verticalFOV = std::stof(m_verticalFOVLineEdit->text().toStdString());
	outData.m_horizontalFOV = std::stof(m_horizontalFOVLineEdit->text().toStdString());
	outData.m_imageGlobalScale = std::stof(m_imageGlobalScaleLineEdit->text().toStdString());
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

	m_savePressed = true;
	this->hide();
}
