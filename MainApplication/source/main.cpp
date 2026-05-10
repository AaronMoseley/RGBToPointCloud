#include "ThirdParty/ThirdPartyDeclarations.h"
#include <memory>

#include <QApplication>
#include <QVulkanInstance>

#include "Management/VoltEngine.h"

bool DebugFilter(QVulkanInstance::DebugMessageSeverityFlags severity, QVulkanInstance::DebugMessageTypeFlags type, const void* message)
{
    auto* data = static_cast<const VkDebugUtilsMessengerCallbackDataEXT*>(message);

    qDebug().noquote()
        << "[VK]"
        << "Severity:" << severity
        << "Type:" << type
        << data->pMessage;

    return true;   // return true to stop Qt from printing it as well
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QVulkanInstance instance;
    instance.setLayers({ "VK_LAYER_KHRONOS_validation" });
    instance.setApiVersion(QVersionNumber(1, 3, 0));
    instance.setExtensions({
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
#ifdef _DEBUG
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
    });
    instance.installDebugOutputFilter(DebugFilter);

    if (bool success = instance.create(); !success)
    {
        qDebug() << "Failed to create Vulkan instance";
        return -1;
    }

    auto screenRect = QRect(0, 0, 915, 400);
    qDebug() << "Screen Size: " << screenRect.width() << " x " << screenRect.height();

    VoltEngine renderingApp(nullptr, &instance, screenRect.width(), screenRect.height());

	std::shared_ptr<Scene> sceneManager = renderingApp.GetCurrentScene();

    std::shared_ptr<RenderObject> cameraObject = std::make_shared<RenderObject>();

    std::shared_ptr<Transform> cameraTransform = cameraObject->AddComponent<Transform>();
    cameraTransform->SetPosition(glm::vec3(0.0f, 0.0f, 5.0f));
    cameraTransform->SetRotation(glm::vec3(0.0f, -90.0f, 0.0f));
    cameraTransform->SetScale(glm::vec3(1.0f));
    cameraObject->AddComponent<Camera>();
    cameraObject->AddComponent<FirstPersonController>();
    cameraObject->AddComponent<DemoBehavior>();
    cameraObject->SetTag("Player");
    sceneManager->AddObject(cameraObject);

	renderingApp.BeginRendering();

    renderingApp.show();

    return app.exec();
}