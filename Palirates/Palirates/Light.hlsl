//--------------------------------------------------------------------------------------
#define MAX_LIGHTS			16 
#define MAX_MATERIALS		16 

#define POINT_LIGHT			1
#define SPOT_LIGHT			2
#define DIRECTIONAL_LIGHT	3


//#define _WITH_METALLIC_ROUGHNESS



struct LIGHT
{
	float4					m_cAmbient;
	float4					m_cDiffuse;
	float4					m_cSpecular;
	float3					m_vPosition;
	float 					m_fFalloff;
	float3					m_vDirection;
	float 					m_fTheta; //cos(m_fTheta)
	float3					m_vAttenuation;
	float					m_fPhi; //cos(m_fPhi)
	bool					m_bEnable;
	int 					m_nType;
	float					m_fRange;
	float					padding;
};


struct Light_Material_Info
{
    float gRoughness;
    float gMetallic;
    float padding0;
    float padding1;
    
    float4 gSpecular;
    float4 gEmissive;
};

StructuredBuffer<Light_Material_Info> Light_Material_Info_List : register(t5);

cbuffer cbLights : register(b2)
{
    LIGHT gLights[MAX_LIGHTS];
    float4 gcGlobalAmbientLight;
    int gnLights;
};


//--------------------------------------------------------------------------------------
// Diffuse + Specular 조명 계산
#define _WITH_METALLIC_ROUGHNESS

//--------------------------------------------------------------------------------------
// Diffuse + Specular 조명 계산
float4 ComputeDiffuseSpecular(float3 vToLight, float3 vNormal, float3 vToCamera, float3 albedoColor, Light_Material_Info lightMaterial, int Light_ID, float shadowFactor)
{
    LIGHT light = gLights[Light_ID];
    float fDiffuseFactor = max(dot(vToLight, vNormal), 0.0f);
    float fSpecularFactor = 0.0f;

#ifdef _WITH_METALLIC_ROUGHNESS
    float roughness = saturate(lightMaterial.gRoughness);
    float metallic = saturate(lightMaterial.gMetallic);
    float metalFactor = pow(metallic, 2.2);
    float shininess = lerp(256.0f, 2.0f, roughness);
    float3 dielectricF0 = float3(0.04f, 0.04f, 0.04f);
    float3 specularColor = lerp(dielectricF0, albedoColor, metalFactor);
    float3 diffuseColor = lerp(albedoColor, float3(0, 0, 0), metalFactor);
#else
    float shininess = lightMaterial.gSpecular.w;
    float3 specularColor = lightMaterial.gSpecular.rgb;
    float3 diffuseColor  = albedoColor;
#endif

    if (fDiffuseFactor > 0.0f && shininess > 0.0f)
    {
        float3 vHalf = normalize(vToCamera + vToLight);
        fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), shininess);
    }

    float shadowStrength = 0.8f; // 더 어둡게 - / 더 밝게 +
    float ambientShadow = lerp(1.0f, shadowFactor, shadowStrength);

    float4 ambient = light.m_cAmbient * float4(diffuseColor, 1.0f) * ambientShadow;
    float4 diffuse = light.m_cDiffuse * fDiffuseFactor * float4(diffuseColor, 1.0f) * shadowFactor;
    float4 specular = light.m_cSpecular * fSpecularFactor * float4(specularColor, 1.0f) * shadowFactor;


    return ambient + diffuse + specular;
}

//--------------------------------------------------------------------------------------

float4 DirectionalLight(int Light_ID, float3 vNormal, float3 vToCamera, float3 albedoColor, Light_Material_Info lightMaterial, float shadowFactor)
{
    float3 vToLight = -gLights[Light_ID].m_vDirection;
    return ComputeDiffuseSpecular(vToLight, vNormal, vToCamera, albedoColor, lightMaterial, Light_ID, shadowFactor);
}

//--------------------------------------------------------------------------------------

float4 PointLight(int Light_ID, float3 vPosition, float3 vNormal, float3 vToCamera, float3 albedoColor, Light_Material_Info lightMaterial)
{
    float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float3 vToLight = gLights[Light_ID].m_vPosition - vPosition;
    float fDistance = length(vToLight);

    if (fDistance <= gLights[Light_ID].m_fRange)
    {
        vToLight /= fDistance;
        float fAttenuation = 1.0f / dot(gLights[Light_ID].m_vAttenuation, float3(1.0f, fDistance, fDistance * fDistance));
        result = ComputeDiffuseSpecular(vToLight, vNormal, vToCamera, albedoColor, lightMaterial, Light_ID, 1.0f) * fAttenuation;
    }
    return result;
}

//--------------------------------------------------------------------------------------

float4 SpotLight(int Light_ID, float3 vPosition, float3 vNormal, float3 vToCamera, float3 albedoColor, Light_Material_Info lightMaterial)
{
    float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float3 vToLight = gLights[Light_ID].m_vPosition - vPosition;
    float fDistance = length(vToLight);

    if (fDistance <= gLights[Light_ID].m_fRange)
    {
        vToLight /= fDistance;
        float fAttenuation = 1.0f / dot(gLights[Light_ID].m_vAttenuation, float3(1.0f, fDistance, fDistance * fDistance));
        float fSpotFactor = pow(max(dot(-vToLight, gLights[Light_ID].m_vDirection), 0.0f), gLights[Light_ID].m_fFalloff);
        result = ComputeDiffuseSpecular(vToLight, vNormal, vToCamera, albedoColor, lightMaterial, Light_ID, 1.0f) * fAttenuation * fSpotFactor;
    }
    return result;
}

//--------------------------------------------------------------------------------------

float4 Lighting(float3 wPosition, float3 wNormal, float3 camera_pos, float3 albedoColor, uint materialID, float shadowFactor)
{
    if (materialID == 0)
        return float4(albedoColor, 1.0f);

    Light_Material_Info lightMaterial = Light_Material_Info_List[materialID];
    float3 vToCamera = normalize(camera_pos - wPosition);
    float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

    for (int i = 0; i < gnLights; i++)
    {
        if (!gLights[i].m_bEnable)
            continue;

        if (gLights[i].m_nType == DIRECTIONAL_LIGHT)
            cColor += DirectionalLight(i, wNormal, vToCamera, albedoColor, lightMaterial, shadowFactor);
        else if (gLights[i].m_nType == POINT_LIGHT)
            cColor += PointLight(i, wPosition, wNormal, vToCamera, albedoColor, lightMaterial);
        else if (gLights[i].m_nType == SPOT_LIGHT)
            cColor += SpotLight(i, wPosition, wNormal, vToCamera, albedoColor, lightMaterial);
    }

#ifdef _WITH_METALLIC_ROUGHNESS
    float m = saturate(lightMaterial.gMetallic);
    float metalFactor = pow(m, 2.2);
    float3 ambientDiffuse = lerp(albedoColor, float3(0.0f, 0.0f, 0.0f), metalFactor);
#else
    float3 ambientDiffuse = albedoColor;
#endif

    cColor += gcGlobalAmbientLight * float4(ambientDiffuse, 1.0f);
    cColor += float4(albedoColor, 1.0f) * lightMaterial.gEmissive.w;
    cColor.a = 1.0f;

    return cColor;
}


float3 Lighting_VisualizeLightAmount(float3 wPosition, float3 wNormal, float3 camera_pos, uint materialID)
{
    Light_Material_Info lightMaterial = Light_Material_Info_List[materialID];
    float3 vToCamera = normalize(camera_pos - wPosition);
    float3 totalLight = float3(0.0f, 0.0f, 0.0f);

    for (int i = 0; i < gnLights; i++)
    {
        if (!gLights[i].m_bEnable)
            continue;

        float3 lightColor = float3(0.0f, 0.0f, 0.0f);

        if (gLights[i].m_nType == DIRECTIONAL_LIGHT)
        {
            lightColor = DirectionalLight(i, wNormal, vToCamera, 1.0f.xxx, lightMaterial, 0.0f).rgb;
        }
        else if (gLights[i].m_nType == POINT_LIGHT)
        {
            lightColor = PointLight(i, wPosition, wNormal, vToCamera, 1.0f.xxx, lightMaterial).rgb;
        }
        else if (gLights[i].m_nType == SPOT_LIGHT)
        {
            lightColor = SpotLight(i, wPosition, wNormal, vToCamera, 1.0f.xxx, lightMaterial).rgb;
        }

        totalLight += lightColor;
    }

    // 시각화를 위해 클램핑 (0 ~ 1 범위로)
    return saturate(totalLight);
}
