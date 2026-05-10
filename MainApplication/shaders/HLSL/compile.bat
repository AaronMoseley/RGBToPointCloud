C:/VulkanSDK/1.4.309.0/Bin/dxc.exe -spirv -T vs_6_0 -E VSMain ObjectShaders.hlsl -Fo VertexShader.spv
C:/VulkanSDK/1.4.309.0/Bin/dxc.exe -spirv -T ps_6_0 -E PSMain ObjectShaders.hlsl -Fo PixelShader.spv

C:/VulkanSDK/1.4.309.0/Bin/dxc.exe -spirv -T vs_6_0 -E VSMain UIObjectShaders.hlsl -Fo UIVertexShader.spv
C:/VulkanSDK/1.4.309.0/Bin/dxc.exe -spirv -T ps_6_0 -E PSMain UIObjectShaders.hlsl -Fo UIPixelShader.spv

pause