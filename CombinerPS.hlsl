
#include "ShaderInclude.hlsli"


Texture2D SurfaceTexture : register(t0); // "t" registers for textures
Texture2D OverlayTexture : register(t1); // "t" registers for textures
SamplerState BasicSampler : register(s0); // "s" registers for samplers

cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float2 uvOffset;
    float2 uvScale;

}
// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
    input.uv = input.uv * uvScale + uvOffset;
	
	// Adjust the variables below as necessary to work with your own code
    float4 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv);
    float4 overlayColor = OverlayTexture.Sample(BasicSampler, input.uv);

	// Combine Texture
    float4 comhineColor = surfaceColor + overlayColor;
	
	
	// Apply colorTint
    comhineColor *= colorTint;
	
	
	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering 
    return float4(comhineColor.x, comhineColor.y, comhineColor.z, 1);
}