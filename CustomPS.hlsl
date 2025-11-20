#include "ShaderInclude.hlsli"


cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float2 uvOffset;
    float2 uvScale;
    float TotalTime; // TIme in seconds

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
    // Use UV coordinates to make a simple gradient
    float2 uvEffect = input.uv * 2 ; // how many times to repeat the pattern using input.uv
    
    float gradient = (sin(uvEffect.x + TotalTime) + cos(uvEffect.y + TotalTime)) + 0.5f;

    // Combine pulse, gradient, and base color
    float3 finalColor = colorTint.rgb * gradient;

    return float4(finalColor, 1.0f);
}