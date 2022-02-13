#ifndef PBR_H_
#define PBR_H_

static const float PI = 3.14159265f;

float3 fresnel_schlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom       = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float3 pbr_equation(float3 albedo, float3 rmao, float3 light_color, float3 light_pos, float3 frag_pos, float3 normal, float3 cam_pos)
{
    float3 N = normalize(normal);
    float3 V = normalize(cam_pos - frag_pos);

    float roughness = rmao.r;
    float metallic  = rmao.g;
    float ao        = rmao.b;
    
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0        = lerp(F0, albedo, metallic);

    // reflectance equation
    float3 Lo = float3(0, 0, 0);

    // calculate per-light radiance
    float3 L           = normalize(light_pos - frag_pos);
    float3 H           = normalize(V + L);
    float  distance    = length(light_pos - frag_pos);
    float  attenuation = 1.0 / (distance * distance);
    float3 radiance    = light_color * attenuation;

    // cook-torrance brdf
    float  NDF = DistributionGGX(N, H, roughness);
    float  G   = GeometrySmith(N, V, L, roughness);
    float3 F   = fresnel_schlick(max(dot(H, V), 0.0), F0);

    float3 kS = F;
    float3 kD = float3(1, 1, 1) - kS;
    kD *= 1.0 - metallic;

    float3 numerator   = NDF * G * F;
    float  denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    float3 specular    = numerator / denominator;

    // add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;

    float3 ambient = float3(0.03, 0.03, 0.03) * albedo * ao;
    float3 color   = ambient + Lo;

    color = color / (color + float3(1, 1, 1));

    float gamma = 1.0 / 2.2;
    color       = pow(color, float3(gamma, gamma, gamma));

    return color;
}

#endif // PBR_H_