#ifndef POINTCLOUDAPP_SETUPSCENE_H
#define POINTCLOUDAPP_SETUPSCENE_H

#include "Objects/ObjectComponent.h"

class SetupScene : public ObjectComponent {
public:
	SetupScene() {};

	void Start() override;
	void Update(float deltaTime) override;

private:
	const std::string kHelpTextCamera = "Press F to Add a New Image";

	std::string GetHelpTextString() const;

	std::shared_ptr<RenderObject> m_helpTextObject;
};



#endif
