#pragma once

#include "VertBuffer.h"
#include "VertexData.h"

class Allocator;
class Material;
class Shader;

template <typename T>
class ArrayBufferInstance
{
public:
	ArrayBufferInstance(
		const size_t size,
		const VertexDataType type,
		const VertexDataType instanceType,
		Allocator& allocator)
		: m_vertBuffer(type)
		, m_instanceVertBuffer(instanceType)
		, m_instanceVertexData(size, instanceType, allocator)
	{
	}

	void initialize()
	{
		glGenVertexArrays(1, &m_vao);
		glBindVertexArray(m_vao);
		m_vertBuffer.initialize();
		m_vertBuffer.bind();
		m_instanceVertBuffer.initialize();
		m_instanceVertBuffer.bind();
		glBindVertexArray(0);
	}

	void terminate()
	{
		m_vertBuffer.destroy();
		m_instanceVertBuffer.destroy();
		m_instanceVertexData.clear();
	}

	void bufferData(const T* data, const size_t count)
	{
		m_instanceVertexData.buffer(data, count);
	}

	void upload()
	{
		m_instanceVertBuffer.bind();
		m_instanceVertBuffer.upload(m_instanceVertexData.getData(), m_instanceVertexData.getCount() * sizeof(T), true);
	}

	void render(const glm::mat4& view, const glm::mat4& projection, Material& material, Shader& shader, const glm::vec3& camPos)
	{
		if (m_instanceVertexData.getCount() == 0)
		{
			return;
		}
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glDisable(GL_BLEND);

		shader.begin();


		if (m_instanceVertBuffer.getType() == VertexDataType::InstancedCubeVerts)
		{
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, material.getEmissive()->getGLTextureID());
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, material.getDisplacement()->getGLTextureID());
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, material.getRoughness()->getGLTextureID());
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, material.getMetalness()->getGLTextureID());
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, material.getNormal()->getGLTextureID());
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, material.getAlbedo()->getGLTextureID());

			shader.setUniform1iv("albedoTexture", 0);
			shader.setUniform1iv("normalTexture", 1);
			shader.setUniform1iv("metalnessTexture", 2);
			shader.setUniform1iv("roughnessTexture", 3);
			shader.setUniform1iv("displacementTexture", 4);
			shader.setUniform1iv("emissiveTexture", 5);
		}
		else
		{

		}

		shader.setUniformM4fv("projectionMatrix", projection);
		shader.setUniformM4fv("viewMatrix", view);
		shader.setUniform3fv("camPos", camPos);

		glBindVertexArray(m_vao);
		//m_cubeVertBuffer.bind();
		glDrawArraysInstanced(GL_TRIANGLES, 0, 36, m_instanceVertexData.getCount());
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);

		shader.end();
	}

	void clear() { m_instanceVertexData.clear(); }
	size_t getCount() const { return m_instanceVertexData.getCount(); }

private:
	GLuint m_vao;
	VertBuffer m_vertBuffer;
	VertBuffer m_instanceVertBuffer;
	VertexData<T> m_instanceVertexData;
};