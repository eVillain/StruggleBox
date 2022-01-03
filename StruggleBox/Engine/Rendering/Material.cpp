#include "Material.h"

#include "Allocator.h"
#include "Renderer.h"
#include "Texture2D.h"
#include "FileUtil.h"
#include "Timer.h"

Material::Material()
: m_albedo(nullptr)
, m_normal(nullptr)
, m_metalness(nullptr)
, m_roughness(nullptr)
, m_displacement(nullptr)
, m_emissive(nullptr)
{
}

void Material::initialize(Renderer& renderer, bool multithreaded)
{
	const std::string path = FileUtil::GetPath().append("Data/Materials/");
	const std::string albedoFile = path + "Materials-Albedo.png";
	const std::string normalFile = path + "Materials-Normal.png";
	const std::string metalFile = path + "Materials-Metalness.png";
	const std::string roughnessFile = path + "Materials-Roughness.png";
	const std::string displacementFile = path + "Materials-Displacement.png";
	const std::string emissiveFile = path + "Materials-Emissive.png";

	const double timeStart = Timer::Seconds();

	if (!multithreaded)
	{
		const TextureID albedoID = renderer.getTextureID(albedoFile, true);
		m_albedo = renderer.getTextureByID(albedoID);
		const TextureID normalID = renderer.getTextureID(normalFile, true);
		m_normal = renderer.getTextureByID(normalID);
		const TextureID metalID = renderer.getTextureID(metalFile, true);
		m_metalness = renderer.getTextureByID(metalID);
		const TextureID roughnessID = renderer.getTextureID(roughnessFile, true);
		m_roughness = renderer.getTextureByID(roughnessID);
		const TextureID displacementID = renderer.getTextureID(displacementFile, true);
		m_displacement = renderer.getTextureByID(displacementID);
		const TextureID emissiveID = renderer.getTextureID(emissiveFile, true);
		m_emissive = renderer.getTextureByID(emissiveID);

		const double timeEnd = Timer::Seconds();
		Log::Debug("Material::initialize took %f seconds to load", timeEnd - timeStart);
	}
	else
	{
		renderer.getTextureIDAsync(albedoFile, [this, &renderer, timeStart](TextureID albedoID)
			{
				m_albedo = renderer.getTextureByID(albedoID);
				const double timeEnd = Timer::Seconds();
				Log::Debug("Material::initialize took %f seconds to load albedo", timeEnd - timeStart);
				if (allLoaded())
				{
					Log::Debug("Material::initialize took %f seconds to load", timeEnd - timeStart);
				}
			});
		renderer.getTextureIDAsync(normalFile, [this, &renderer, timeStart](TextureID normalID)
			{
				m_normal = renderer.getTextureByID(normalID);
				const double timeEnd = Timer::Seconds();
				Log::Debug("Material::initialize took %f seconds to load normal", timeEnd - timeStart);
				if (allLoaded())
				{
					Log::Debug("Material::initialize took %f seconds to load", timeEnd - timeStart);
				}
			});
		renderer.getTextureIDAsync(metalFile, [this, &renderer, timeStart](TextureID metalID)
			{
				m_metalness = renderer.getTextureByID(metalID);
				const double timeEnd = Timer::Seconds();
				Log::Debug("Material::initialize took %f seconds to load metalness", timeEnd - timeStart);
				if (allLoaded())
				{
					Log::Debug("Material::initialize took %f seconds to load", timeEnd - timeStart);
				}
			});
		renderer.getTextureIDAsync(roughnessFile, [this, &renderer, timeStart](TextureID roughnessID)
			{
				m_roughness = renderer.getTextureByID(roughnessID);
				const double timeEnd = Timer::Seconds();
				Log::Debug("Material::initialize took %f seconds to load roughness", timeEnd - timeStart);
				if (allLoaded())
				{
					Log::Debug("Material::initialize took %f seconds to load", timeEnd - timeStart);
				}
			});
		renderer.getTextureIDAsync(displacementFile, [this, &renderer, timeStart](TextureID displacementID)
			{
				m_displacement = renderer.getTextureByID(displacementID);
				const double timeEnd = Timer::Seconds();
				Log::Debug("Material::initialize took %f seconds to load displacement", timeEnd - timeStart);
				if (allLoaded())
				{
					Log::Debug("Material::initialize took %f seconds to load", timeEnd - timeStart);
				}
			});
		renderer.getTextureIDAsync(emissiveFile, [this, &renderer, timeStart](TextureID emissiveID)
			{
				m_emissive = renderer.getTextureByID(emissiveID);
				const double timeEnd = Timer::Seconds();
				Log::Debug("Material::initialize took %f seconds to load emissive", timeEnd - timeStart);
				if (allLoaded())
				{
					Log::Debug("Material::initialize took %f seconds to load", timeEnd - timeStart);
				}
			});
	}
}
