#include "ImageSourceManager.h"

#include <qcoreapplication.h>

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

	m_imageSourceManagementWidget->SetEditCallback(editCallback);
	m_imageSourceManagementWidget->SetRemovalCallback(removeCallback);

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

	m_mlModelSession = std::make_unique<Ort::Session>(*m_onnxEnvironment, kMLModelPath.c_str(), sessionOptions);
}

void ImageSourceManager::ProcessImages()
{
	for (const auto&[objectHandle, imageSourceData] : m_imageSettingsData)
	{
		RGBImagePixelData pixelData;
		bool validImage = ReadImage(imageSourceData, pixelData);

		if (validImage == false || pixelData.size() == 0)
		{
			continue;
		}

		ImageDepthData depthData;
		PredictDepthOfImage(pixelData, depthData);

		CreatePointsFromDepthImage(imageSourceData, depthData, pixelData);
	}
}

bool ImageSourceManager::ReadImage(const ImageSourceSettingsDialog::ImageSourceSettingsData& imageSourceData, RGBImagePixelData& outImagePixels)
{
	int imageWidth, imageHeight, imageChannels;

	stbi_uc* pixels = stbi_load(imageSourceData.m_imageSourcePath.c_str(),
		&imageWidth, &imageHeight, &imageChannels, STBI_rgb);

	if (!pixels)
	{
		return false;
	}

	size_t croppedHeight = (imageHeight / 16) * 16;
	size_t croppedWidth = (imageWidth / 16) * 16;

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

void ImageSourceManager::PredictDepthOfImage(const RGBImagePixelData& imagePixels, ImageDepthData& outDepthData)
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

	auto outputs = m_mlModelSession->Run(
		Ort::RunOptions{nullptr},
		input_names,
		&input_tensor,
		1,
		output_names,
		1);

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

void ImageSourceManager::CreatePointsFromDepthImage(const ImageSourceSettingsDialog::ImageSourceSettingsData& imageSourceData, const ImageDepthData& depthImage, const RGBImagePixelData& imagePixels)
{
	size_t imageHeight = depthImage.size();
	size_t imageWidth = depthImage[0].size();

	std::shared_ptr<RenderObject> cameraObject = GetScene()->GetRenderObject(imageSourceData.m_cameraObjectHandle);
	std::shared_ptr<Transform> cameraTransform = cameraObject->GetComponent<Transform>();

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
				sourcePosition = cameraTransform->GetPosition();
				sourcePosition += cameraTransform->Right() * (normalizedXCoordinate * (imageSourceData.m_worldImageWidth / 2.0f));
				sourcePosition += cameraTransform->Up() * (normalizedYCoordinate * (imageSourceData.m_worldImageHeight / 2.0f));

				cameraRay = cameraTransform->Forward();
			}

			glm::vec3 pointPosition = sourcePosition + (cameraRay * currentDepth);

			std::shared_ptr<RenderObject> newPoint = std::make_shared<RenderObject>();
			std::shared_ptr<Transform> pointTransform = newPoint->AddComponent<Transform>();
			pointTransform->SetPosition(pointPosition);
			pointTransform->SetScale(glm::vec3(0.2f));
			std::shared_ptr<Square> newPointMesh = newPoint->AddComponent<Square>();
			newPointMesh->SetColor(glm::vec3(imagePixels[y][x][0], imagePixels[y][x][1], imagePixels[y][x][2]));
			GetScene()->AddObject(newPoint);
		}
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
	GetScene()->RemoveObject(cameraObjectHandle);
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

		cameraTransform->SetPosition(imageSourceData.m_position);
		cameraTransform->SetRotation(imageSourceData.m_rotation);
		m_imageSourceManagementWidget->ChangeCameraName(imageSourceData.m_cameraObjectHandle, newImageSourceData.m_cameraName);
		m_imageSettingsData[imageSourceData.m_cameraObjectHandle] = newImageSourceData;
	}
}