dxc -spirv -T vs_6_0 -E VSMain ObjectShaders.hlsl -Fo VertexShader.spv
dxc -spirv -T ps_6_0 -E PSMain ObjectShaders.hlsl -Fo PixelShader.spv

dxc -spirv -T vs_6_0 -E VSMain UIObjectShaders.hlsl -Fo UIVertexShader.spv
dxc -spirv -T ps_6_0 -E PSMain UIObjectShaders.hlsl -Fo UIPixelShader.spv
