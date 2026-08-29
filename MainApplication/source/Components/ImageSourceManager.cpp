#include "ImageSourceManager.h"

#include <qcoreapplication.h>

#include "lasdefinitions.hpp"
#include "laspoint.hpp"
#include "laswriter.hpp"

#include "Components/Transform.h"
#include "Components/Square.h"
#include "Components/GLTFModel.h"
#include "Management/Scene.h"
#include "Widgets/ImageSourceSettingsDialog.h"
#include "stb_image.h"

void ImageSourceManager::Start()
{
	AddCameraManagementWidget();
}

void ImageSourceManager::Update(float deltaTime)
{
	if (GetWindowManager()->KeyPressedThisFrame(Qt::Key::Key_Escape))
	{
		GetWindowManager()->Shutdown();
	}

	if (GetWindowManager()->KeyPressedThisFrame(Qt::Key::Key_F))
	{
		AddCamera();
	}

	if (m_activeThreads == 0 && m_imageSourceManagementWidget->IsProcessImagesButtonEnabled() == false)
	{
		m_imageSourceManagementWidget->SetButtonsEnabled(true);
	}

	m_objectsToAddMutex.lock();
	while (m_objectsToAdd.size() > 0)
	{
		std::shared_ptr<RenderObject> newPoint = m_objectsToAdd.front();
		m_objectsToAdd.pop();
		GetScene()->AddObject(newPoint);
	}
	m_objectsToAddMutex.unlock();

	m_objectsToRemoveMutex.lock();
	while (m_objectsToRemove.size() > 0)
	{
		VulkanCommonFunctions::ObjectHandle handle = m_objectsToRemove.front();
		m_objectsToRemove.pop();
		GetScene()->RemoveObject(handle);
	}
	m_objectsToRemoveMutex.unlock();
}

void ImageSourceManager::ExportLAS(std::string filePath)
{
	size_t totalPointCount = 0;
	glm::vec3 fileOffset = {0.0f, 0.0f, 0.0f};

	for (const auto& [cameraHandle, imageSettings] : m_imageSettingsData)
	{
		std::shared_ptr<RenderObject> cameraObject = GetScene()->GetRenderObject(cameraHandle);

		if (totalPointCount == 0)
		{
			fileOffset = cameraObject->GetComponent<Transform>()->GetWorldPosition();
		}

		totalPointCount += cameraObject->GetComponent<Transform>()->GetChildCount();
	}

	if (totalPointCount == 0)
	{
		m_activeThreads--;
		return;
	}

	LASheader header;

	header.version_major = 1;
	header.version_minor = 4;
	header.point_data_format = 7;
	header.point_data_record_length = 36;
	header.header_size = 375;
	header.offset_to_point_data = 375;

	header.x_scale_factor = 0.001;
	header.y_scale_factor = 0.001;
	header.z_scale_factor = 0.001;

	header.x_offset = fileOffset.x;
	header.y_offset = fileOffset.y;
	header.z_offset = fileOffset.z;

	header.number_of_variable_length_records = 0;
	header.user_data_in_header_size = 0;
	header.user_data_after_header_size = 0;

	std::time_t now = std::time(nullptr);
	std::tm* local = std::localtime(&now);

	header.file_creation_day = static_cast<uint16_t>(local->tm_yday + 1);
	header.file_creation_year = static_cast<uint16_t>(local->tm_year + 1900);

	std::strncpy(
	header.system_identifier,
	"RGBToPointCloud",
	sizeof(header.system_identifier));

	std::strncpy(
		header.generating_software,
		"RGBToPointCloud 1.0",
		sizeof(header.generating_software));

	LASwriteOpener opener;
	opener.set_file_name(filePath.c_str());

	LASwriter* writer = opener.open(&header);

	if (writer == nullptr)
	{
		qDebug() << "Could not open laslib writer";
		m_activeThreads--;
		return;
	}

	LASpoint point;
	point.init(&writer->quantizer,
		header.point_data_format,
		header.point_data_record_length,
		&header);

	std::array<uint16_t, 3> rgbValues = {0, 0, 0};

	for (const auto& [cameraHandle, imageSettings] : m_imageSettingsData)
	{
		std::shared_ptr<Transform> cameraTransform = GetScene()->GetRenderObject(cameraHandle)->GetComponent<Transform>();
		size_t childCount = cameraTransform->GetChildCount();

		for (size_t i = 0; i < childCount; i++)
		{
			std::shared_ptr<Transform> pointTransform = cameraTransform->GetChild(i);
			std::shared_ptr<PointMeshRenderer> pointMesh = pointTransform->GetOwner()->GetComponent<PointMeshRenderer>();

			if (pointMesh == nullptr)
			{
				continue;
			}

			point.set_x(pointTransform->GetWorldPosition().x);
			point.set_y(pointTransform->GetWorldPosition().y);
			point.set_z(pointTransform->GetWorldPosition().z);

			rgbValues[0] = static_cast<uint16_t>(pointMesh->GetColor().r * 65535.0f);
			rgbValues[1] = static_cast<uint16_t>(pointMesh->GetColor().g * 65535.0f);
			rgbValues[2] = static_cast<uint16_t>(pointMesh->GetColor().b * 65535.0f);

			point.set_RGB(rgbValues.data());

			writer->write_point(&point);
			writer->update_inventory(&point);
		}
	}

	writer->update_header(&header, true);
	writer->close();

	m_activeThreads--;
}

void ImageSourceManager::TriggerExportLAS(std::string filePath)
{
	m_imageSourceManagementWidget->SetButtonsEnabled(false);
	m_activeThreads++;
	std::thread exportThread(&ImageSourceManager::ExportLAS, this, filePath);
	exportThread.detach();
}

void ImageSourceManager::AddCamera()
{
	std::shared_ptr<Transform> currentTransform = GetOwner()->GetComponent<Transform>();
	glm::quat currentRotation = currentTransform->GetRotationQuaternion();

	glm::vec3 currentPosition = currentTransform->GetPosition();
	glm::vec3 cameraPosition = currentPosition;
	cameraPosition += 0.5f * currentTransform->Right();
	cameraPosition -= 0.25f * currentTransform->Up();
	cameraPosition += 2.0f * currentTransform->Forward();

	std::shared_ptr<RenderObject> newCameraModel = std::make_shared<RenderObject>();
	newCameraModel->SetMaterialName("GenericObjectMaterial");
	std::shared_ptr<Transform> gltfModelTransform = newCameraModel->AddComponent<Transform>();
	std::shared_ptr<GLTFModel> gltfMesh = newCameraModel->AddComponent<GLTFModel>();
	gltfMesh->SetLit(false);
	gltfModelTransform->SetRotationQuaternion(currentRotation);
	gltfModelTransform->SetPosition(cameraPosition);
	gltfModelTransform->SetScale(glm::vec3(1.0f, 1.0f, 1.0f));

	gltfMesh->SetSourcePath("models/Camera/CameraModel.gltf");
	gltfMesh->ReverseWindingOrder();

	VulkanCommonFunctions::ObjectHandle cameraHandle = GetScene()->AddObject(newCameraModel);

	TriggerImageSourceSettingsDialog(cameraHandle);
}

void ImageSourceManager::AddCameraManagementWidget()
{
	m_imageSourceManagementWidget = new ImageSourceManagementWidget();

	std::function<void(VulkanCommonFunctions::ObjectHandle)> editCallback = std::bind(&ImageSourceManager::TriggerImageSourceEditing, this, std::placeholders::_1);
	std::function<void(VulkanCommonFunctions::ObjectHandle)> removeCallback = std::bind(&ImageSourceManager::TriggerImageSourceRemoval, this, std::placeholders::_1);
	std::function<void(std::string)> exportCallback = std::bind(&ImageSourceManager::TriggerExportLAS, this, std::placeholders::_1);

	m_imageSourceManagementWidget->SetEditCallback(editCallback);
	m_imageSourceManagementWidget->SetRemovalCallback(removeCallback);
	m_imageSourceManagementWidget->SetLASExportCallback(exportCallback);

	std::function<void()> processImagesCallback = std::bind(&ImageSourceManager::ProcessImages, this);

	m_imageSourceManagementWidget->SetProcessImagesCallback(processImagesCallback);

	GetWindowManager()->AddWidgetToMenu("ImageSourceManager", m_imageSourceManagementWidget);
}

void ImageSourceManager::InitializeOnnxSession()
{
	m_onnxEnvironment = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "inference");

	Ort::SessionOptions sessionOptions;
	sessionOptions.SetIntraOpNumThreads(1);
	sessionOptions.SetGraphOptimizationLevel(
		GraphOptimizationLevel::ORT_ENABLE_ALL);

	m_mlModelSessionIndoor = std::make_unique<Ort::Session>(*m_onnxEnvironment, kMLModelPathIndoor.c_str(), sessionOptions);
	m_mlModelSessionOutdoor = std::make_unique<Ort::Session>(*m_onnxEnvironment, kMLModelPathOutdoor.c_str(), sessionOptions);
}

void ImageSourceManager::RemovePointsRelatedToCamera(VulkanCommonFunctions::ObjectHandle cameraObjectHandle)
{
	std::shared_ptr<Transform> cameraTransform = GetScene()->GetRenderObject(cameraObjectHandle)->GetComponent<Transform>();

	while (cameraTransform->GetChildCount() > 0)
	{
		std::shared_ptr<Transform> childTransform = cameraTransform->GetChild(0);
		cameraTransform->RemoveChild(0);
		VulkanCommonFunctions::ObjectHandle childHandle = childTransform->GetOwner()->GetObjectHandle();

		m_objectsToRemoveMutex.lock();
		m_objectsToRemove.push(childHandle);
		m_objectsToRemoveMutex.unlock();
	}

	m_objectsToRemoveMutex.lock();
	m_objectsToRemove.push(cameraObjectHandle);
	m_objectsToRemoveMutex.unlock();
}

void ImageSourceManager::ProcessSingleImage(const ImageSourceSettingsDialog::ImageSourceSettingsData& imageSettings,
	std::shared_ptr<Transform> cameraTransform)
{
	RGBImagePixelData pixelData;
	bool validImage = ReadImage(imageSettings, pixelData);

	if (validImage == false || pixelData.size() == 0)
	{
		return;;
	}

	ImageDepthData depthData;
	PredictDepthOfImage(pixelData, depthData, imageSettings.m_imageSetting);

	CreatePointsFromDepthImage(imageSettings, depthData, pixelData, cameraTransform);

	m_activeThreads--;
}

void ImageSourceManager::ProcessImages()
{
	m_imageSourceManagementWidget->SetButtonsEnabled(false);

	for (const auto&[objectHandle, imageSourceData] : m_imageSettingsData)
	{
		std::shared_ptr<Transform> cameraTransform = GetScene()->GetRenderObject(objectHandle)->GetComponent<Transform>();

		std::thread processingThread(&ImageSourceManager::ProcessSingleImage, this, std::cref(imageSourceData), cameraTransform);
		m_activeThreads++;
		processingThread.detach();
	}
}

bool ImageSourceManager::ReadImage(const ImageSourceSettingsDialog::ImageSourceSettingsData& imageSourceData, RGBImagePixelData& outImagePixels)
{
	int imageWidth, imageHeight, imageChannels;

	stbi_uc* pixels = stbi_load(imageSourceData.m_imageSourcePath.string().c_str(),
		&imageWidth, &imageHeight, &imageChannels, STBI_rgb);

	if (!pixels)
	{
		return false;
	}

	size_t croppedHeight = (imageHeight / 32) * 32;
	size_t croppedWidth = (imageWidth / 32) * 32;

	size_t heightOffset = (imageHeight - croppedHeight) / 2;
	size_t widthOffset = (imageWidth - croppedWidth) / 2;

	outImagePixels.resize(croppedHeight);

	for (size_t y = 0; y < croppedHeight; y++)
	{
		size_t sourceY = y + heightOffset;

		outImagePixels[y].resize(croppedWidth);

		for (size_t x = 0; x < croppedWidth; x++)
		{
			size_t sourceX = x + widthOffset;

			size_t index = ((sourceY * imageWidth) + sourceX) * 3;

			outImagePixels[y][x] =
			{
				pixels[index] / 255.0f,
				pixels[index + 1] / 255.0f,
				pixels[index + 2] / 255.0f
			};
		}
	}

	return true;
}

void ImageSourceManager::PredictDepthOfImage(const RGBImagePixelData& imagePixels, ImageDepthData& outDepthData, ImageSourceSettingsDialog::ImageSetting imageSetting)
{
	size_t imageHeight = imagePixels.size();
	size_t imageWidth = imagePixels[0].size();

	std::vector<float> inputTensor(3 * imageHeight * imageWidth);

	for (size_t i = 0; i < imageHeight; i++)
	{
		size_t rowOffset = i * imageWidth;

		for (size_t j = 0; j < imageWidth; j++)
		{
			size_t indexWithinChannel = rowOffset + j;

			size_t redIndex = indexWithinChannel;
			size_t greenIndex = (imageHeight * imageWidth) + indexWithinChannel;
			size_t blueIndex = (2 * imageHeight * imageWidth) + indexWithinChannel;

			inputTensor[redIndex] = imagePixels[i][j][0];
			inputTensor[greenIndex] = imagePixels[i][j][1];
			inputTensor[blueIndex] = imagePixels[i][j][2];
		}
	}

	std::array<int64_t, 4> inputShape = { 1, 3, static_cast<int64_t>(imageHeight), static_cast<int64_t>(imageWidth) };

	Ort::MemoryInfo memory_info =
	Ort::MemoryInfo::CreateCpu(
		OrtArenaAllocator,
		OrtMemTypeDefault);

	Ort::Value input_tensor =
		Ort::Value::CreateTensor<float>(
			memory_info,
			inputTensor.data(),
			inputTensor.size(),
			inputShape.data(),
			inputShape.size());

	const char* input_names[] = { kMLModelInputName.c_str() };
	const char* output_names[] = { kMLModelOutputName.c_str() };

	std::vector<Ort::Value> outputs;
	if (imageSetting == ImageSourceSettingsDialog::ImageSetting::Indoor)
	{
		m_indoorOnnxSessionMutex.lock();
		outputs = m_mlModelSessionIndoor->Run(
			Ort::RunOptions{nullptr},
			input_names,
			&input_tensor,
			1,
			output_names,
			1);
		m_indoorOnnxSessionMutex.unlock();
	} else if (imageSetting == ImageSourceSettingsDialog::ImageSetting::Outdoor)
	{
		m_outdoorOnnxSessionMutex.lock();
		outputs = m_mlModelSessionOutdoor->Run(
			Ort::RunOptions{nullptr},
			input_names,
			&input_tensor,
			1,
			output_names,
			1);
		m_outdoorOnnxSessionMutex.unlock();
	}

	if (outputs.size() == 0)
	{
		return;
	}

	float* result = outputs[0].GetTensorMutableData<float>();

	outDepthData.resize(imageHeight);
	for (size_t y = 0; y < imageHeight; y++)
	{
		outDepthData[y].resize(imageWidth);

		size_t rowOffset = y * imageWidth;

		for (size_t x = 0; x < imageWidth; x++)
		{
			outDepthData[y][x] = glm::exp(result[rowOffset + x]);
		}
	}
}

void ImageSourceManager::CreatePointsFromDepthImage(const ImageSourceSettingsDialog::ImageSourceSettingsData& imageSourceData,
	const ImageDepthData& depthImage, const RGBImagePixelData& imagePixels,
	std::shared_ptr<Transform> cameraTransform)
{
	size_t imageHeight = depthImage.size();
	size_t imageWidth = depthImage[0].size();

	size_t existingPointCount = cameraTransform->GetChildCount();
	size_t currentChildIndex = 0;

	for (size_t y = 0; y < imageHeight; y++)
	{
		float normalizedYCoordinate = 1.0f - (((static_cast<float>(y) + 0.5f) / imageHeight) * 2.0f);

		for (size_t x = 0; x < imageWidth; x++)
		{
			float random = ((double) rand() / (RAND_MAX));

			if (random > kPercentageOfPixelsToKeep)
			{
				continue;
			}

			float currentDepth = depthImage[y][x] * imageSourceData.m_imageGlobalScale;
			float normalizedXCoordinate = ((static_cast<float>(x) + 0.5f) / imageWidth) * 2.0f - 1.0f;

			glm::vec3 cameraRay;
			glm::vec3 sourcePosition;

			if (imageSourceData.m_imageType == ImageSourceSettingsDialog::ImageType::Perspective)
			{
				cameraRay = cameraTransform->Forward() +
					(cameraTransform->Right() * (normalizedXCoordinate * glm::tan(glm::radians(imageSourceData.m_horizontalFOV) * 0.5f))) +
					(cameraTransform->Up() * (normalizedYCoordinate * glm::tan(glm::radians(imageSourceData.m_verticalFOV) * 0.5f)));
				cameraRay = glm::normalize(cameraRay);

				sourcePosition = cameraTransform->GetPosition();
			}
			else
			{
				sourcePosition = glm::vec3(0.0f);
				sourcePosition += cameraTransform->Right() * (normalizedXCoordinate * (imageSourceData.m_worldImageWidth / 2.0f));
				sourcePosition += cameraTransform->Up() * (normalizedYCoordinate * (imageSourceData.m_worldImageHeight / 2.0f));

				cameraRay = cameraTransform->Forward();
			}

			glm::vec3 pointPosition = sourcePosition + (cameraRay * currentDepth);

			std::shared_ptr<Transform> pointTransform = nullptr;
			std::shared_ptr<Square> newPointMesh = nullptr;

			if (currentChildIndex < existingPointCount)
			{
				pointTransform = cameraTransform->GetChild(currentChildIndex);
				newPointMesh = pointTransform->GetOwner()->GetComponent<Square>();

				currentChildIndex++;
			} else
			{
				std::shared_ptr<RenderObject> newPoint = std::make_shared<RenderObject>();
				pointTransform = newPoint->AddComponent<Transform>();
				newPointMesh = newPoint->AddComponent<Square>();
				pointTransform->SetParent(cameraTransform);

				m_objectsToAddMutex.lock();
				m_objectsToAdd.push(newPoint);
				m_objectsToAddMutex.unlock();
			}

			pointTransform->SetPosition(pointPosition);
			pointTransform->SetScale(glm::vec3(0.2f));
			newPointMesh->SetColor(glm::vec3(imagePixels[y][x][0], imagePixels[y][x][1], imagePixels[y][x][2]));
		}
	}

	if (currentChildIndex >= existingPointCount)
	{
		return;
	}

	while (currentChildIndex < cameraTransform->GetChildCount())
	{
		std::shared_ptr<Transform> childToRemove = cameraTransform->GetChild(currentChildIndex);
		cameraTransform->RemoveChild(currentChildIndex);

		VulkanCommonFunctions::ObjectHandle pointHandle = childToRemove->GetOwner()->GetObjectHandle();

		m_objectsToRemoveMutex.lock();
		m_objectsToRemove.push(pointHandle);
		m_objectsToRemoveMutex.unlock();
	}
}

void ImageSourceManager::TriggerImageSourceEditing(VulkanCommonFunctions::ObjectHandle cameraObjectHandle)
{
	if (m_imageSettingsData.contains(cameraObjectHandle) == false)
	{
		return;
	}

	TriggerImageSourceSettingsDialog(m_imageSettingsData[cameraObjectHandle]);
}

void ImageSourceManager::TriggerImageSourceRemoval(VulkanCommonFunctions::ObjectHandle cameraObjectHandle)
{
	m_imageSettingsData.erase(cameraObjectHandle);

	std::thread removalThread(&ImageSourceManager::RemovePointsRelatedToCamera, this, cameraObjectHandle);
	removalThread.detach();
}

void ImageSourceManager::TriggerImageSourceSettingsDialog(VulkanCommonFunctions::ObjectHandle cameraObjectHandle)
{
	std::shared_ptr<RenderObject> cameraObject = GetScene()->GetRenderObject(cameraObjectHandle);

	std::shared_ptr<Transform> cameraTransform = cameraObject->GetComponent<Transform>();

	std::string cameraName = "Camera_" + std::to_string(m_imageSettingsData.size());
	ImageSourceSettingsDialog* dialog = new ImageSourceSettingsDialog(cameraName, cameraTransform->GetWorldPosition(), cameraTransform->GetRotation());
	dialog->show();

	while (dialog->isVisible())
	{
		QCoreApplication::processEvents();
	}

	if (dialog->GetSavePressed())
	{
		ImageSourceSettingsDialog::ImageSourceSettingsData imageSourceData;
		dialog->LoadImageSourceData(imageSourceData);
		imageSourceData.m_cameraObjectHandle = cameraObjectHandle;

		cameraTransform->SetPosition(imageSourceData.m_position);
		cameraTransform->SetRotation(imageSourceData.m_rotation);
		m_imageSourceManagementWidget->AddImageSource(imageSourceData);
		m_imageSettingsData[cameraObjectHandle] = imageSourceData;
	} else
	{
		GetScene()->RemoveObject(cameraObjectHandle);

		m_imageSettingsData.erase(cameraObjectHandle);
	}
}

void ImageSourceManager::TriggerImageSourceSettingsDialog(const ImageSourceSettingsDialog::ImageSourceSettingsData& imageSourceData)
{
	ImageSourceSettingsDialog* dialog = new ImageSourceSettingsDialog(imageSourceData);
	dialog->show();

	std::shared_ptr<Transform> cameraTransform = GetScene()->GetRenderObject(imageSourceData.m_cameraObjectHandle)->GetComponent<Transform>();

	while (dialog->isVisible())
	{
		QCoreApplication::processEvents();
	}

	if (dialog->GetSavePressed())
	{
		ImageSourceSettingsDialog::ImageSourceSettingsData newImageSourceData;
		dialog->LoadImageSourceData(newImageSourceData);
		newImageSourceData.m_cameraObjectHandle = imageSourceData.m_cameraObjectHandle;

		cameraTransform->SetPosition(imageSourceData.m_position);
		cameraTransform->SetRotation(imageSourceData.m_rotation);
		m_imageSourceManagementWidget->ChangeCameraName(imageSourceData.m_cameraObjectHandle, newImageSourceData.m_cameraName);
		m_imageSettingsData[imageSourceData.m_cameraObjectHandle] = newImageSourceData;
	}
}