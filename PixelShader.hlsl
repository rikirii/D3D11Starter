
#include "ShaderInclude.hlsli"

Texture2D SurfaceTexture : register(t0); // "t" registers for textures
Texture2D NormalMap : register(t1);
SamplerState BasicSampler : register(s0); // "s" registers for samplers

cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float2 uvOffset;
    float2 uvScale;
    float3 cameraPosition;
    float roughness;
    float3 ambientLight;
    float time;
    Light lights[5]; // array of extaclty 5 lights

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
    input.normal = normalize(input.normal);
    input.tangent = normalize(input.tangent);
    input.uv = input.uv * uvScale + uvOffset;
    
    input.normal = NormalMapping(NormalMap, BasicSampler, input.uv, input.normal, input.tangent);
	
	// Adjust the variables below as necessary to work with your own code
	float4 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv);
	
    surfaceColor *=colorTint;
    
    //starting off with ambient light
    float3 totalColor = ambientLight;

    
    for (int i = 0; i < 5; i++)
    {
        Light light = lights[i];
        light.direction = normalize(lights[i].direction);
        
        if (lights[i].type == LIGHT_TYPE_DIRECTIONAL)
        {
            // Process directional light
            float3 dirLight = CalcDirectionalLight(light, input.normal, input.worldPosition, cameraPosition, surfaceColor.xyz, roughness);
            
            totalColor += dirLight;
        }
        else if (lights[i].type == LIGHT_TYPE_POINT)
        {
            // Process point light
            float3 pointLight = CalcPointLight(light, input.normal, input.worldPosition, cameraPosition, surfaceColor.xyz, roughness);
            totalColor += pointLight;
        }
        else if (lights[i].type == LIGHT_TYPE_SPOT)
        {
            // Process spot light
            float3 spotLight = CalcSpotLight(light, input.normal, input.worldPosition, cameraPosition, surfaceColor.xyz, roughness);
            totalColor += spotLight;
        }
    }
    
    
    
	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering 
    
    return float4(totalColor, 1);
    //float3 tangent = normalize(input.tangent);
    //return float4(tangent * 0.5f + 0.5f, 1.0f);

}