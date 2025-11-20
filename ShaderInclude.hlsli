#ifndef __GGP_SHADER_INCLUDES__// Each .hlsli file needs a unique identifier! 
#define __GGP_SHADER_INCLUDES__


#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2
#define MAX_SPECULAR_EXPONENT 256.0f
// ALL of your code pieces (structs, functions, etc.) go here!


// Struct representing a single vertex worth of data
// - This should match the vertex definition in our C++ code
// - By "match", I mean the size, order and number of members
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
// 10-16/2025 assignment 7, now after the change, this no longer is a cbuffer
struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float3 localPosition : POSITION; // XYZ position
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};


// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float4 screenPosition : SV_POSITION;
    float2 uv : TEXCOORD; // UV coordinates
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 worldPosition : POSITION;
};



struct VertexToPixel_Sky
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float4 position : SV_Position;
    float3 sampleDir : Direction;
};



struct Light
{
    int type; // 0 = directional, 1 = point, 2 = 
    float3 direction; // For directional and spot lights
    float range; // Effective range for point and spot lights
    float3 position; // For point and spot lights
    float intensity; // Brightness of the light
    float3 color; // RGBA color
    float spotInnerAngle; // Inner cone angle (in radians) – Inside this, full light!
    float spotOuterAngle; // Outer cone angle (in radians) – Between inner and outer, light falls off to zero
    float2 padding; // Padding to align to 16-byte boundary
	
};


float3 NormalMapping(Texture2D normalMap, SamplerState basicSampler,float2 uv, float3 normalFromVS, float3 tangentFromVS )
{
    float3 normalFromTexture = normalMap.Sample(basicSampler, uv).xyz;
    
    // Unpack normal from texture sample – ensure normalization!
    float3 unpackedNormal = normalize(normalFromTexture * 2.0f - 1.0f);
    // Create TBN matrix
    float3 N = normalize(normalFromVS);
    float3 T = normalize(tangentFromVS - dot(tangentFromVS, N) * N); // Orthonormalize!
    float3 B = cross(T, N);
    float3x3 TBN = float3x3(T, B, N);
    // Transform normal from map
    float3 finalNormal = mul(unpackedNormal, TBN);
    return finalNormal;
}


float CalcAttenuate(Light light, float3 worldPos)
{
    float dist = distance(light.position, worldPos);
    float att = saturate(1.0f - (dist * dist / (light.range * light.range)));
    return att * att;
}

float3 CalcDiffuse(float3 normal, float3 direToLight, float3 lightColor, float lightIntensity, float3 surfaceColor)
{
    

    return saturate(dot(normal, direToLight)) * lightColor * lightIntensity * surfaceColor;
    
} 


float3 CalcSpecularPhong(float3 camPos, float3 worldPos, float3 lightDir, float3 normal, float roughness)
{
    //float3 viewDir = normalize(camPos - worldPos); // Light direction is opposite to light vector
    
    float3 dirToCam = normalize(camPos - worldPos);
    
    
    float3 reflectDir = reflect(lightDir, normal); // Direction of reflected light
    
    
    float rdotV = saturate(dot(reflectDir, dirToCam));
    
    float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
    
    float specFactor = pow(rdotV, specExponent); // Specular factor
    
    return specFactor;
    
}


float3 CalcDirectionalLight(Light lights, float3 normal, float3 worldPos, float3 camPos, float3 surfaceColor, float roughness)
{
    
    float3 direToLight = normalize(-lights.direction); // Light direction is opposite to light vector
    
    
    float3 diffuse = CalcDiffuse(
        normal,
        direToLight,
        lights.color,
        lights.intensity,
        surfaceColor
    );
    
    float3 specular = CalcSpecularPhong(camPos, worldPos, lights.direction, normal, roughness) * lights.color * lights.intensity * surfaceColor;
    
    //float3 result = surfaceColor * (diffuse + specular); // Tint specular?
    float3 result = surfaceColor * diffuse + specular; // Don’t tint specular?

    //return result * lights.color * lights.intensity; // Light’s overall color
    return result;
}


float3 CalcPointLight(Light light, float3 normal, float3 worldPos, float3 camPos, float3 surfaceColor, float roughness)
{
    float3 pixelToLight = normalize(light.position - worldPos);
    
    
    float3 direToLight = normalize(-light.direction); // Light direction is opposite to light vector
    
    float attenuation = CalcAttenuate(light,worldPos); // Linear falloff
    
    float3 diffuse = CalcDiffuse(
        normal,
        direToLight,
        light.color,
        light.intensity * attenuation,
        surfaceColor
    );
    
    float3 specular = CalcSpecularPhong(camPos, worldPos, light.direction, normal, roughness) * light.color * light.intensity * attenuation * surfaceColor;
    
    float3 result = surfaceColor * diffuse + specular; // Don’t tint specular?
    return result;
}

float3 CalcSpotLight(Light light, float3 normal, float3 worldPos, float3 camPos, float3 surfaceColor, float roughness)
{
    float3 lightToPixel = normalize(light.position - worldPos);
    //float3 lightDir = normalize(light.direction);
    
    
    // Get cos(angle) between pixel and light direction
    float pixelAngle = saturate(dot(-lightToPixel, light.direction));
    // Get cosines of angles and calculate range
    float cosOuter = cos(light.spotOuterAngle);
    float cosInner = cos(light.spotInnerAngle);
    float falloffRange = cosOuter - cosInner;
    // Linear falloff over the range, clamp 0-1, apply to light calc
    float spotTerm = saturate((cosOuter - pixelAngle) / falloffRange);
    float3 color = CalcPointLight(light, normal, worldPos, camPos, surfaceColor, roughness) * spotTerm ;
    
    return color;

}

#endif