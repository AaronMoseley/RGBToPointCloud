#include "ImageSourceManagementWidget.h"

void ImageSourceManagementWidget::InitializeWidget()
{
	QHBoxLayout* mainLayout = new QHBoxLayout();
	setLayout(mainLayout);

	QScrollArea* scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	scrollArea->setMaximumHeight(100);
	QWidget* scrollContainer = new QWidget();
	scrollArea->setWidget(scrollContainer);
	m_scrollLayout = new QVBoxLayout(scrollContainer);
	mainLayout->addWidget(scrollArea);

	QPushButton* generateCloudButton = new QPushButton("Process Images");
	connect(generateCloudButton, &QPushButton::clicked, this, &ImageSourceManagementWidget::ProcessImagesClicked);
	mainLayout->addWidget(generateCloudButton);
	generateCloudButton->setFixedHeight(100);
	m_scrollLayout->addStretch();
}

void ImageSourceManagementWidget::ChangeCameraName(VulkanCommonFunctions::ObjectHandle cameraObjectHandle, const std::string& newName)
{
	if (m_scrollAreaNameLabels.contains(cameraObjectHandle) == false)
	{
		return;
	}

	m_scrollAreaNameLabels[cameraObjectHandle]->setText(newName.c_str());
}

void ImageSourceManagementWidget::AddImageSource(const ImageSourceSettingsDialog::ImageSourceSettingsData& imageSourceData)
{
	QHBoxLayout* elementLayout = new QHBoxLayout();
	QLabel* nameLabel = new QLabel(imageSourceData.m_cameraName.c_str());
	elementLayout->addWidget(nameLabel);
	m_scrollAreaNameLabels[imageSourceData.m_cameraObjectHandle] = nameLabel;
	m_scrollAreaElements[imageSourceData.m_cameraObjectHandle] = elementLayout;

	QPushButton* editButton = new QPushButton("Edit");
	std::function<void(VulkanCommonFunctions::ObjectHandle)> editFunction = std::bind(&ImageSourceManagementWidget::EditClicked, this, imageSourceData.m_cameraObjectHandle);
	connect(editButton, &QPushButton::clicked, this, editFunction);
	elementLayout->addWidget(editButton);

	QPushButton* removeButton = new QPushButton("Remove");
	std::function<void(VulkanCommonFunctions::ObjectHandle)> removeFunction = std::bind(&ImageSourceManagementWidget::RemoveClicked, this, imageSourceData.m_cameraObjectHandle);
	connect(removeButton, &QPushButton::clicked, this, removeFunction);
	elementLayout->addWidget(removeButton);

	m_scrollLayout->addLayout(elementLayout);
}

void ImageSourceManagementWidget::EditClicked(VulkanCommonFunctions::ObjectHandle cameraObjectHandle)
{
	if (m_editCallback != nullptr)
	{
		m_editCallback(cameraObjectHandle);
	}
}

void ImageSourceManagementWidget::RemoveClicked(VulkanCommonFunctions::ObjectHandle cameraObjectHandle)
{
	if (m_scrollAreaElements.contains(cameraObjectHandle))
	{
		m_scrollLayout->removeItem(m_scrollAreaElements[cameraObjectHandle]);
		QLayoutItem *item;
		while ((item = m_scrollAreaElements[cameraObjectHandle]->takeAt(0)) != nullptr)
		{
			delete item->widget();   // Safe if widget() is nullptr
			delete item;
		}

		delete m_scrollAreaElements[cameraObjectHandle];

		m_scrollAreaElements.erase(cameraObjectHandle);
	}

	m_scrollAreaNameLabels.erase(cameraObjectHandle);

	if (m_removalCallback != nullptr)
	{
		m_removalCallback(cameraObjectHandle);
	}
}

void ImageSourceManagementWidget::ProcessImagesClicked()
{
	if (m_processImagesCallback == nullptr)
	{
		return;
	}

	m_processImagesCallback();
}