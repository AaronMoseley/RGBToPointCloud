#ifndef POINTCLOUDAPP_CAMERAMANAGER_H
#define POINTCLOUDAPP_CAMERAMANAGER_H

#include <onnxruntime_cxx_api.h>

#include "Objects/ObjectComponent.h"
#include "Widgets/ImageSourceSettingsDialog.h"
#include "Widgets/ImageSourceManagementWidget.h"
#include <filesystem>
#include <mutex>
#include <thread>

#include "Vulkan Interface/VulkanCommonFunctions.h"

class ImageSourceManager : public ObjectComponent {
public:
	using RGBImagePixelData = std::vector<std::vector<std::array<float, 3>>>;
	using ImageDepthData = std::vector<std::vector<double>>;

	ImageSourceManager()
	{
		InitializeOnnxSession();
	}

	void Start() override;
	void Update(float deltaTime) override;

	void TriggerImageSourceEditing(VulkanCommonFunctions::ObjectHandle cameraObjectHandle);
	void TriggerImageSourceRemoval(VulkanCommonFunctions::ObjectHandle cameraObjectHandle);

private:
	//need to be able to pick between multiple models
	const std::filesystem::path kMLModelPath = "ml_models/IndoorModel.onnx";
	const std::string kMLModelInputName = "input";
	const std::string kMLModelOutputName = "output";
	const float kPercentageOfPixelsToKeep = 0.2f;

	std::unique_ptr<Ort::Env> m_onnxEnvironment = nullptr;
	std::unique_ptr<Ort::Session> m_mlModelSession = nullptr;

	std::mutex m_onnxSessionMutex;
	std::mutex m_sceneMutex;

	void InitializeOnnxSession();

	void AddCameraManagementWidget();

	void TriggerImageSourceSettingsDialog(VulkanCommonFunctions::ObjectHandle cameraObjectHandle);
	void TriggerImageSourceSettingsDialog(const ImageSourceSettingsDialog::ImageSourceSettingsData& imageSourceData);

	void ProcessImages();
	void ProcessSingleImage(const ImageSourceSettingsDialog::ImageSourceSettingsData& imageSettings);
	bool ReadImage(const ImageSourceSettingsDialog::ImageSourceSettingsData& imageSourceData, RGBImagePixelData& outImagePixels);
	void PredictDepthOfImage(const RGBImagePixelData& imagePixels, ImageDepthData& outDepthData);
	void CreatePointsFromDepthImage(const ImageSourceSettingsDialog::ImageSourceSettingsData& imageSourceData, const ImageDepthData& depthImage, const RGBImagePixelData& imagePixels);

	void AddCamera();

	std::map<VulkanCommonFunctions::ObjectHandle, ImageSourceSettingsDialog::ImageSourceSettingsData> m_imageSettingsData;
	ImageSourceManagementWidget* m_imageSourceManagementWidget = nullptr;
};



#endif
