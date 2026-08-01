#ifndef POINTCLOUDAPP_CAMERAMANAGER_H
#define POINTCLOUDAPP_CAMERAMANAGER_H

#include "Objects/ObjectComponent.h"

class ImageSourceManager : public ObjectComponent {
public:
	ImageSourceManager() {}

	void Start() override;
	void Update(float deltaTime) override;

private:
	void AddCameraManagementWidget();
	void TriggerAddCameraDialog();

	void AddCamera();

	std::vector<std::shared_ptr<RenderObject>> m_cameraObjects;
};



#endif
