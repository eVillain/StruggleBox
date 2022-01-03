#pragma once

#include "VertBuffer.h"
#include "VertexData.h"
#include "Timer.h"

class Allocator;
class Material;
class Shader;

template <typename T>
class ArrayBufferVertex
{
public:
	ArrayBufferVertex(const size_t size, const VertexDataType type, Allocator& allocator)
		: m_vao(0)
		, m_vertBuffer(type)
		, m_vertexData(size, type, allocator)
	{
	}

	void initialize()
	{
		glGenVertexArrays(1, &m_vao);
		glBindVertexArray(m_vao);
		m_vertBuffer.initialize();
		m_vertBuffer.bind();
		glBindVertexArray(0);
	}

	void terminate()
	{
		glDeleteVertexArrays(1, &m_vao);
		m_vertBuffer.destroy();
		m_vertexData.clear();
	}

	void bufferData(const T* data, const size_t count)
	{
		m_vertexData.buffer(data, count);
	}

	void upload()
	{
		m_vertBuffer.bind();
		m_vertBuffer.upload(m_vertexData.getData(), m_vertexData.getCount() * sizeof(T), true);
	}

	void render(
		const glm::mat4& view,
		const glm::mat4& projection,
		const glm::mat3& normalMatrix,
		const glm::vec3& cameraPosition,
		uint32_t texID0,
		uint32_t texID1,
		uint32_t texID2,
		uint32_t texID3,
		uint32_t texID4,
		uint32_t texID5,
		Shader& shader)
	{
		if (m_vertexData.getCount() == 0)
		{
			return;
		}

		if (texID0)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texID0);
		}
		if (texID1)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, texID1);
		}
		if (texID2)
		{
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, texID2);
		}
		if (texID3)
		{
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, texID3);
		}
		if (texID4)
		{
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, texID4);
		}
		if (texID5)
		{
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, texID5);
		}
		shader.begin();
		shader.setUniformM4fv("View", view);
		shader.setUniformM4fv("Projection", projection);
		shader.setUniformM3fv("NormalMatrix", normalMatrix);
		shader.setUniform3fv("CameraPosition", cameraPosition);

		if (m_vertBuffer.getType() == VertexDataType::SphereVerts)
		{
			glDisable(GL_CULL_FACE);
			glDepthMask(GL_TRUE);

			shader.setUniform1iv("albedoTexture", 0);
			shader.setUniform1iv("normalTexture", 1);
			shader.setUniform1iv("metalnessTexture", 2);
			shader.setUniform1iv("roughnessTexture", 3);
			shader.setUniform1iv("displacementTexture", 4);
			shader.setUniform1iv("emissiveTexture", 5);
		}
		else if (m_vertBuffer.getType() == VertexDataType::FireballVerts)
		{
			shader.setUniform1iv("textureMap", 0);
			shader.setUniform1fv("time", Timer::RunTimeSeconds());
			//Log::Debug("Rendering %i fireball verts", m_vertexData.getCount());
			//Log::Debug("First vert at: %f, %f, %f", m_vertexData.getData()[0].x, m_vertexData.getData()[0].y, m_vertexData.getData()[0].z);
		}

		glBindVertexArray(m_vao);
		glDrawArrays(GL_POINTS, 0, m_vertexData.getCount());
		shader.end();
		glBindVertexArray(0);
	}

	void clear() { m_vertexData.clear(); }
	size_t getCount() const { return m_vertexData.getCount(); }

protected:
	GLuint m_vao;
	VertBuffer m_vertBuffer;
	VertexData<T> m_vertexData;
};
