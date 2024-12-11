#include "parametric_shapes.hpp"
#include "core/Log.h"

#include <glm/glm.hpp>

#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

bonobo::mesh_data parametric_shapes::createQuad(float const width,
	float const height,
	unsigned int const horizontal_split_count,
	unsigned int const vertical_split_count)
{
	auto const horizontal_edges_count = horizontal_split_count + 1u;
	auto const vertical_edges_count = vertical_split_count + 1u;
	auto const vertices_nb = horizontal_edges_count * vertical_edges_count; // 总顶点数量

	// 定义顶点、法线和纹理坐标
	auto vertices = std::vector<glm::vec3>(vertices_nb);
	auto normals = std::vector<glm::vec3>(vertices_nb, glm::vec3(0.0f, 1.0f, 0.0f)); // 假设法线指向上方
	auto texcoords = std::vector<glm::vec2>(vertices_nb); 

	// 生成顶点和纹理坐标
	size_t index = 0u;
	for (unsigned int z = 0u; z < vertical_edges_count; ++z) {
		for (unsigned int x = 0u; x < horizontal_edges_count; ++x) {
			// 顶点位置
			vertices[index] = glm::vec3(
				(static_cast<float>(x) / horizontal_split_count * width) - (width / 2.0f), // x
				0.0f, // y
				(static_cast<float>(z) / vertical_split_count * height) - (height / 2.0f) // z
			);

			// 纹理坐标
			texcoords[index] = glm::vec2(
				static_cast<float>(x) / horizontal_split_count, // u
				static_cast<float>(z) / vertical_split_count   // v
			);

			++index;
		}
	}

	// 创建索引数组
	auto index_sets = std::vector<glm::uvec3>(2u * horizontal_split_count * vertical_split_count);

	// 生成索引
	index = 0u;
	for (unsigned int z = 0u; z < vertical_split_count; ++z) {
		for (unsigned int x = 0u; x < horizontal_split_count; ++x) {
			index_sets[index++] = glm::uvec3(
				vertical_edges_count * z + x,
				vertical_edges_count * z + x + 1,
				vertical_edges_count * (z + 1) + x + 1
			);
			index_sets[index++] = glm::uvec3(
				vertical_edges_count * z + x,
				vertical_edges_count * (z + 1) + x + 1,
				vertical_edges_count * (z + 1) + x
			);
		}
	}

	// 创建 mesh_data
	bonobo::mesh_data data;
	glGenVertexArrays(1, &data.vao);
	assert(data.vao != 0u);
	glBindVertexArray(data.vao);

	// 计算缓冲区大小
	auto const vertices_size = static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec3));
	auto const normals_size = static_cast<GLsizeiptr>(normals.size() * sizeof(glm::vec3));
	auto const texcoords_size = static_cast<GLsizeiptr>(texcoords.size() * sizeof(glm::vec2));
	auto const bo_size = vertices_size + normals_size + texcoords_size;

	// 设置 VBO
	glGenBuffers(1, &data.bo);
	assert(data.bo != 0u);
	glBindBuffer(GL_ARRAY_BUFFER, data.bo);
	glBufferData(GL_ARRAY_BUFFER, bo_size, nullptr, GL_STATIC_DRAW);

	// 顶点数据
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices_size, vertices.data());
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(0));

	// 法线数据
	glBufferSubData(GL_ARRAY_BUFFER, vertices_size, normals_size, normals.data());
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::normals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::normals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(vertices_size));

	// 纹理坐标数据
	glBufferSubData(GL_ARRAY_BUFFER, vertices_size + normals_size, texcoords_size, texcoords.data());
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(vertices_size + normals_size));

	// 设置索引缓冲区
	data.indices_nb = static_cast<GLsizei>(index_sets.size() * 3u);
	glGenBuffers(1, &data.ibo);
	assert(data.ibo != 0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(index_sets.size() * sizeof(glm::uvec3)), index_sets.data(), GL_STATIC_DRAW);

	// 解绑
	glBindVertexArray(0u);
	glBindBuffer(GL_ARRAY_BUFFER, 0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

	return data;
}





bonobo::mesh_data
parametric_shapes::createSphere(float const radius,
                                unsigned int const longitude_split_count,
                                unsigned int const latitude_split_count)
{

	//! \todo Implement this function
	//! return bonobo::mesh_data();
	auto const longitude_edges_count = longitude_split_count + 1u;
	auto const latitude_edges_count = latitude_split_count + 1u;
	auto const longitude_vertices_count = longitude_edges_count + 1u;
	auto const latitude_vertices_count = latitude_edges_count + 1u;
	auto const vertices_nb = longitude_vertices_count * latitude_vertices_count;//整个球体网格的总顶点数量

	auto vertices = std::vector<glm::vec3>(vertices_nb);
	auto normals = std::vector<glm::vec3>(vertices_nb);
	auto texcoords = std::vector<glm::vec3>(vertices_nb);
	auto tangents = std::vector<glm::vec3>(vertices_nb);
	auto binormals = std::vector<glm::vec3>(vertices_nb);

	float const d_theta = glm::two_pi<float>() / (static_cast<float>(longitude_edges_count));
	float const d_phi = glm::pi<float>() / (static_cast<float>(latitude_edges_count));

	// generate vertices iteratively
	size_t index = 0u;
	for (unsigned int lat = 0u; lat < latitude_vertices_count; ++lat) {
		float const phi = lat * d_phi;

		for (unsigned int lon = 0u; lon < longitude_vertices_count; ++lon) {
			float const theta = lon * d_theta;

			//// vertex position
			glm::vec3 position = glm::vec3(
				radius * std::sin(theta) * std::sin(phi),
				-radius * std::cos(phi),
				radius * std::cos(theta) * std::sin(phi)
			);
			vertices[index] = position;

			// normal
			glm::vec3 normal = glm::normalize(position);
			normals[index] = normal;

			//tangent
			glm::vec3 tangent = glm::vec3(
				radius * std::cos(theta),
				0.0f,
				-radius * std::sin(theta)
			);

			tangents[index] = glm::normalize(tangent);

			// binormal
			glm::vec3 binormal = glm::vec3(
				radius * std::sin(theta) * std::cos(phi),
				radius * std::sin(phi),
				radius * std::cos(theta) * std::cos(phi)
			);
		
			binormals[index] = glm::normalize(binormal);

			// texture coordinates
			texcoords[index] = glm::vec3(
				static_cast<float>(lon) / static_cast<float>(longitude_split_count),
				static_cast<float>(lat) / static_cast<float>(latitude_split_count),
				0.0f); 


			++index;
		}
	}

	// create index array
	auto index_sets = std::vector<glm::uvec3>(2u * longitude_edges_count * latitude_edges_count);

	// generate indices iteratively
	index = 0u;
	for (unsigned int lat = 0u; lat < latitude_edges_count; ++lat)
	{
		for (unsigned int lon = 0u; lon < longitude_edges_count; ++lon)
		{
			index_sets[index] = glm::uvec3(latitude_vertices_count * (lat + 0u) + (lon + 0u),
				latitude_vertices_count * (lat + 0u) + (lon + 1u),
				latitude_vertices_count * (lat + 1u) + (lon + 1u));
			++index;

			index_sets[index] = glm::uvec3(latitude_vertices_count * (lat + 0u) + (lon + 0u),
				latitude_vertices_count * (lat + 1u) + (lon + 1u),
				latitude_vertices_count * (lat + 1u) + (lon + 0u));
			++index;
		}
	}

	bonobo::mesh_data data;
	glGenVertexArrays(1, &data.vao);
	assert(data.vao != 0u);
	glBindVertexArray(data.vao);

	auto const vertices_offset = 0u;
	auto const vertices_size = static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec3));
	auto const normals_offset = vertices_size;
	auto const normals_size = static_cast<GLsizeiptr>(normals.size() * sizeof(glm::vec3));
	auto const texcoords_offset = normals_offset + normals_size;
	auto const texcoords_size = static_cast<GLsizeiptr>(texcoords.size() * sizeof(glm::vec3));
	auto const tangents_offset = texcoords_offset + texcoords_size;
	auto const tangents_size = static_cast<GLsizeiptr>(tangents.size() * sizeof(glm::vec3));
	auto const binormals_offset = tangents_offset + tangents_size;
	auto const binormals_size = static_cast<GLsizeiptr>(binormals.size() * sizeof(glm::vec3));
	auto const bo_size = static_cast<GLsizeiptr>(vertices_size
		+ normals_size
		+ texcoords_size
		+ tangents_size
		+ binormals_size
		);
	glGenBuffers(1, &data.bo);
	assert(data.bo != 0u);
	glBindBuffer(GL_ARRAY_BUFFER, data.bo);
	glBufferData(GL_ARRAY_BUFFER, bo_size, nullptr, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, vertices_offset, vertices_size, static_cast<GLvoid const*>(vertices.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(0x0));

	glBufferSubData(GL_ARRAY_BUFFER, normals_offset, normals_size, static_cast<GLvoid const*>(normals.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::normals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::normals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(normals_offset));

	glBufferSubData(GL_ARRAY_BUFFER, texcoords_offset, texcoords_size, static_cast<GLvoid const*>(texcoords.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(texcoords_offset));

	glBufferSubData(GL_ARRAY_BUFFER, tangents_offset, tangents_size, static_cast<GLvoid const*>(tangents.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::tangents));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::tangents), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(tangents_offset));

	glBufferSubData(GL_ARRAY_BUFFER, binormals_offset, binormals_size, static_cast<GLvoid const*>(binormals.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::binormals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::binormals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(binormals_offset));

	glBindBuffer(GL_ARRAY_BUFFER, 0u);

	data.indices_nb = static_cast<GLsizei>(index_sets.size() * 3u);
	glGenBuffers(1, &data.ibo);
	assert(data.ibo != 0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(index_sets.size() * sizeof(glm::uvec3)), reinterpret_cast<GLvoid const*>(index_sets.data()), GL_STATIC_DRAW);

	glBindVertexArray(0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

	return data;
}

bonobo::mesh_data
parametric_shapes::createTorus(float const major_radius,
                               float const minor_radius,
                               unsigned int const major_split_count,
                               unsigned int const minor_split_count)
{
	//! \todo (Optional) Implement this function
	
	//return bonobo::mesh_data();
	auto const major_edges_count = major_split_count + 1u;
	auto const minor_edges_count = minor_split_count + 1u;
	auto const major_vertices_count = major_edges_count + 1u;
	auto const minor_vertices_count = minor_edges_count + 1u;
	auto const vertices_nb = major_vertices_count * minor_vertices_count; // 总顶点数

	auto vertices = std::vector<glm::vec3>(vertices_nb);
	auto normals = std::vector<glm::vec3>(vertices_nb, glm::vec3(0.0f, 1.0f, 0.0f)); ;
	auto texcoords = std::vector<glm::vec2>(vertices_nb);
	auto tangents = std::vector<glm::vec3>(vertices_nb);
	auto binormals = std::vector<glm::vec3>(vertices_nb);

	float const d_theta = glm::two_pi<float>() / static_cast<float>(major_edges_count);
	float const d_phi = glm::two_pi<float>() / static_cast<float>(minor_edges_count);

	// generate vertices iteratively
	size_t index = 0u;
	for (unsigned int i = 0; i < major_vertices_count; ++i) {
		float const theta = i * d_phi;

		for (unsigned int j = 0; j < minor_vertices_count; ++j) {
			float const phi = j * d_theta;

			// 顶点位置
			glm::vec3 position = glm::vec3(
				(major_radius + minor_radius * std::cos(theta)) * std::cos(phi),
				//-minor_radius * std::sin(theta),
				(major_radius + minor_radius * std::cos(theta)) * std::sin(phi),
				-minor_radius * std::sin(theta)
			);
			vertices[index] = position;

			// 法线
			glm::vec3 normal = glm::normalize(position);
			normals[index] = normal;

			// 切线
			glm::vec3 tangent = glm::vec3(
				-minor_radius * std::sin(theta) * std::cos(phi),
				//-minor_radius * std::cos(theta),
				-minor_radius * std::sin(theta) * std::sin(phi),
				-minor_radius * std::cos(theta)

			);

			// 副切线
			glm::vec3 binormal = glm::vec3(
				-(major_radius + minor_radius * std::cos(theta)) * std::sin(phi),
				//0.0f,
				(major_radius + minor_radius * std::cos(theta)) * std::cos(phi),
				0.0f
			);

			tangents[index] = glm::normalize(tangent);
			binormals[index] = glm::normalize(binormal);

			// 纹理坐标
			texcoords[index] = glm::vec2(
				static_cast<float>(i) / static_cast<float>(major_split_count),
				static_cast<float>(j) / static_cast<float>(minor_split_count)
			);

			++index;
		}
	}

	// create index array
	auto index_sets = std::vector<glm::uvec3>(2u * major_edges_count * minor_edges_count);

	// generate indices iteratively
	index = 0u;
	for (unsigned int i = 0u; i < major_edges_count; ++i)
	{
		for (unsigned int j = 0u; j < minor_edges_count; ++j)
		{
			index_sets[index] = glm::uvec3(minor_vertices_count * (i + 0u) + (j + 0u),
				minor_vertices_count * (i + 0u) + (j + 1u),
				minor_vertices_count * (i + 1u) + (j + 1u));
			++index;

			index_sets[index] = glm::uvec3(minor_vertices_count * (i + 0u) + (j + 0u),
				minor_vertices_count * (i + 1u) + (j + 1u),
				minor_vertices_count * (i + 1u) + (j + 0u));
			++index;
		}
	}

	bonobo::mesh_data data;
	glGenVertexArrays(1, &data.vao);
	assert(data.vao != 0u);
	glBindVertexArray(data.vao);

	auto const vertices_offset = 0u;
	auto const vertices_size = static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec3));
	auto const normals_offset = vertices_size;
	auto const normals_size = static_cast<GLsizeiptr>(normals.size() * sizeof(glm::vec3));
	auto const texcoords_offset = normals_offset + normals_size;
	auto const texcoords_size = static_cast<GLsizeiptr>(texcoords.size() * sizeof(glm::vec2));
	auto const tangents_offset = texcoords_offset + texcoords_size;
	auto const tangents_size = static_cast<GLsizeiptr>(tangents.size() * sizeof(glm::vec3));
	auto const binormals_offset = tangents_offset + tangents_size;
	auto const binormals_size = static_cast<GLsizeiptr>(binormals.size() * sizeof(glm::vec3));
	auto const bo_size = static_cast<GLsizeiptr>(vertices_size
		+ normals_size
		+ texcoords_size
		+ tangents_size
		+ binormals_size
		);
	glGenBuffers(1, &data.bo);
	assert(data.bo != 0u);
	glBindBuffer(GL_ARRAY_BUFFER, data.bo);
	glBufferData(GL_ARRAY_BUFFER, bo_size, nullptr, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, vertices_offset, vertices_size, static_cast<GLvoid const*>(vertices.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(0x0));

	glBufferSubData(GL_ARRAY_BUFFER, normals_offset, normals_size, static_cast<GLvoid const*>(normals.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::normals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::normals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(normals_offset));

	glBufferSubData(GL_ARRAY_BUFFER, texcoords_offset, texcoords_size, static_cast<GLvoid const*>(texcoords.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(texcoords_offset));

	glBufferSubData(GL_ARRAY_BUFFER, tangents_offset, tangents_size, static_cast<GLvoid const*>(tangents.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::tangents));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::tangents), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(tangents_offset));

	glBufferSubData(GL_ARRAY_BUFFER, binormals_offset, binormals_size, static_cast<GLvoid const*>(binormals.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::binormals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::binormals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(binormals_offset));

	glBindBuffer(GL_ARRAY_BUFFER, 0u);

	data.indices_nb = static_cast<GLsizei>(index_sets.size() * 3u);
	glGenBuffers(1, &data.ibo);
	assert(data.ibo != 0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(index_sets.size() * sizeof(glm::uvec3)), reinterpret_cast<GLvoid const*>(index_sets.data()), GL_STATIC_DRAW);

	glBindVertexArray(0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

	return data;
}

bonobo::mesh_data
parametric_shapes::createCircleRing(float const radius,
                                    float const spread_length,
                                    unsigned int const circle_split_count,
                                    unsigned int const spread_split_count)
{
	auto const circle_slice_edges_count = circle_split_count + 1u;
	auto const spread_slice_edges_count = spread_split_count + 1u;
	auto const circle_slice_vertices_count = circle_slice_edges_count + 1u;
	auto const spread_slice_vertices_count = spread_slice_edges_count + 1u;
	auto const vertices_nb = circle_slice_vertices_count * spread_slice_vertices_count;

	auto vertices  = std::vector<glm::vec3>(vertices_nb);
	auto normals   = std::vector<glm::vec3>(vertices_nb);
	auto texcoords = std::vector<glm::vec3>(vertices_nb);
	auto tangents  = std::vector<glm::vec3>(vertices_nb);
	auto binormals = std::vector<glm::vec3>(vertices_nb);

	float const spread_start = radius - 0.5f * spread_length;
	float const d_theta = glm::two_pi<float>() / (static_cast<float>(circle_slice_edges_count));
	float const d_spread = spread_length / (static_cast<float>(spread_slice_edges_count));

	// generate vertices iteratively
	size_t index = 0u;
	float theta = 0.0f;
	for (unsigned int i = 0u; i < circle_slice_vertices_count; ++i) {
		float const cos_theta = std::cos(theta);
		float const sin_theta = std::sin(theta);

		float distance_to_centre = spread_start;
		for (unsigned int j = 0u; j < spread_slice_vertices_count; ++j) {
			// vertex
			vertices[index] = glm::vec3(distance_to_centre * cos_theta,
			                            distance_to_centre * sin_theta,
			                            0.0f);

			// texture coordinates
			texcoords[index] = glm::vec3(static_cast<float>(j) / (static_cast<float>(spread_slice_vertices_count)),
			                             static_cast<float>(i) / (static_cast<float>(circle_slice_vertices_count)),
			                             0.0f);

			// tangent
			auto const t = glm::vec3(cos_theta, sin_theta, 0.0f);
			tangents[index] = t;

			// binormal
			auto const b = glm::vec3(-sin_theta, cos_theta, 0.0f);
			binormals[index] = b;

			// normal
			auto const n = glm::cross(t, b);
			normals[index] = n;

			distance_to_centre += d_spread;
			++index;
		}

		theta += d_theta;
	}

	// create index array
	auto index_sets = std::vector<glm::uvec3>(2u * circle_slice_edges_count * spread_slice_edges_count);

	// generate indices iteratively
	index = 0u;
	for (unsigned int i = 0u; i < circle_slice_edges_count; ++i)
	{
		for (unsigned int j = 0u; j < spread_slice_edges_count; ++j)
		{
			index_sets[index] = glm::uvec3(spread_slice_vertices_count * (i + 0u) + (j + 0u),
			                               spread_slice_vertices_count * (i + 0u) + (j + 1u),
			                               spread_slice_vertices_count * (i + 1u) + (j + 1u));
			++index;

			index_sets[index] = glm::uvec3(spread_slice_vertices_count * (i + 0u) + (j + 0u),
			                               spread_slice_vertices_count * (i + 1u) + (j + 1u),
			                               spread_slice_vertices_count * (i + 1u) + (j + 0u));
			++index;
		}
	}

	bonobo::mesh_data data;
	glGenVertexArrays(1, &data.vao);
	assert(data.vao != 0u);
	glBindVertexArray(data.vao);

	auto const vertices_offset = 0u;
	auto const vertices_size = static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec3));
	auto const normals_offset = vertices_size;
	auto const normals_size = static_cast<GLsizeiptr>(normals.size() * sizeof(glm::vec3));
	auto const texcoords_offset = normals_offset + normals_size;
	auto const texcoords_size = static_cast<GLsizeiptr>(texcoords.size() * sizeof(glm::vec3));
	auto const tangents_offset = texcoords_offset + texcoords_size;
	auto const tangents_size = static_cast<GLsizeiptr>(tangents.size() * sizeof(glm::vec3));
	auto const binormals_offset = tangents_offset + tangents_size;
	auto const binormals_size = static_cast<GLsizeiptr>(binormals.size() * sizeof(glm::vec3));
	auto const bo_size = static_cast<GLsizeiptr>(vertices_size
	                                            +normals_size
	                                            +texcoords_size
	                                            +tangents_size
	                                            +binormals_size
	                                            );
	glGenBuffers(1, &data.bo);
	assert(data.bo != 0u);
	glBindBuffer(GL_ARRAY_BUFFER, data.bo);
	glBufferData(GL_ARRAY_BUFFER, bo_size, nullptr, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, vertices_offset, vertices_size, static_cast<GLvoid const*>(vertices.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(0x0));

	glBufferSubData(GL_ARRAY_BUFFER, normals_offset, normals_size, static_cast<GLvoid const*>(normals.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::normals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::normals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(normals_offset));

	glBufferSubData(GL_ARRAY_BUFFER, texcoords_offset, texcoords_size, static_cast<GLvoid const*>(texcoords.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(texcoords_offset));

	glBufferSubData(GL_ARRAY_BUFFER, tangents_offset, tangents_size, static_cast<GLvoid const*>(tangents.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::tangents));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::tangents), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(tangents_offset));

	glBufferSubData(GL_ARRAY_BUFFER, binormals_offset, binormals_size, static_cast<GLvoid const*>(binormals.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::binormals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::binormals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(binormals_offset));

	glBindBuffer(GL_ARRAY_BUFFER, 0u);

	data.indices_nb = static_cast<GLsizei>(index_sets.size() * 3u);
	glGenBuffers(1, &data.ibo);
	assert(data.ibo != 0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(index_sets.size() * sizeof(glm::uvec3)), reinterpret_cast<GLvoid const*>(index_sets.data()), GL_STATIC_DRAW);

	glBindVertexArray(0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

	return data;

}

bonobo::mesh_data parametric_shapes::createWall(float const height,
	float const depth,
	unsigned int const vertical_split_count,
	unsigned int const depth_split_count)
{
	// 计算每个方向上的边数
	auto const vertical_edges_count = vertical_split_count + 1u;
	auto const depth_edges_count = depth_split_count + 1u;

	auto const vertices_nb = vertical_edges_count * depth_edges_count; // 总顶点数

	// 定义顶点、法线和纹理坐标
	auto vertices = std::vector<glm::vec3>(vertices_nb);
	auto normals = std::vector<glm::vec3>(vertices_nb, glm::vec3(1.0f, 0.0f, 0.0f)); // 默认法线指向 x 轴
	auto texcoords = std::vector<glm::vec2>(vertices_nb);

	// 生成顶点和纹理坐标
	size_t index = 0u;
	for (unsigned int z = 0u; z < depth_edges_count; ++z) {
		for (unsigned int y = 0u; y < vertical_edges_count; ++y) {
			// 顶点位置
			vertices[index] = glm::vec3(
				0.0f, // x 固定为 0
				(static_cast<float>(y) / vertical_split_count * height) - (height / 2.0f), // y
				(static_cast<float>(z) / depth_split_count * depth) - (depth / 2.0f) // z
			);

			// 纹理坐标
			texcoords[index] = glm::vec2(
				static_cast<float>(y) / vertical_split_count, // u
				static_cast<float>(z) / depth_split_count    // v
			);

			++index;
		}
	}

	// 创建索引数组
	auto index_sets = std::vector<glm::uvec3>(2u * vertical_split_count * depth_split_count);

	// 生成索引
	index = 0u;
	for (unsigned int z = 0u; z < depth_split_count; ++z) {
		for (unsigned int y = 0u; y < vertical_split_count; ++y) {
			index_sets[index++] = glm::uvec3(
				depth_edges_count * y + z,
				depth_edges_count * y + z + 1,
				depth_edges_count * (y + 1) + z + 1
			);
			index_sets[index++] = glm::uvec3(
				depth_edges_count * y + z,
				depth_edges_count * (y + 1) + z + 1,
				depth_edges_count * (y + 1) + z
			);
		}
	}

	// 创建 mesh_data
	bonobo::mesh_data data;
	glGenVertexArrays(1, &data.vao);
	assert(data.vao != 0u);
	glBindVertexArray(data.vao);

	// 计算缓冲区大小
	auto const vertices_size = static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec3));
	auto const normals_size = static_cast<GLsizeiptr>(normals.size() * sizeof(glm::vec3));
	auto const texcoords_size = static_cast<GLsizeiptr>(texcoords.size() * sizeof(glm::vec2));
	auto const bo_size = vertices_size + normals_size + texcoords_size;

	// 设置 VBO
	glGenBuffers(1, &data.bo);
	assert(data.bo != 0u);
	glBindBuffer(GL_ARRAY_BUFFER, data.bo);
	glBufferData(GL_ARRAY_BUFFER, bo_size, nullptr, GL_STATIC_DRAW);

	// 顶点数据
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices_size, vertices.data());
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(0));

	// 法线数据
	glBufferSubData(GL_ARRAY_BUFFER, vertices_size, normals_size, normals.data());
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::normals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::normals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(vertices_size));

	// 纹理坐标数据
	glBufferSubData(GL_ARRAY_BUFFER, vertices_size + normals_size, texcoords_size, texcoords.data());
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(vertices_size + normals_size));

	// 设置索引缓冲区
	data.indices_nb = static_cast<GLsizei>(index_sets.size() * 3u);
	glGenBuffers(1, &data.ibo);
	assert(data.ibo != 0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(index_sets.size() * sizeof(glm::uvec3)), index_sets.data(), GL_STATIC_DRAW);

	// 解绑
	glBindVertexArray(0u);
	glBindBuffer(GL_ARRAY_BUFFER, 0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

	return data;
}

bonobo::mesh_data parametric_shapes::createFront(float const width,
	float const height,
	unsigned int const horizontal_split_count,
	unsigned int const vertical_split_count)
{
	auto const horizontal_edges_count = horizontal_split_count + 1u;
	auto const vertical_edges_count = vertical_split_count + 1u;
	auto const vertices_nb = horizontal_edges_count * vertical_edges_count; // Total number of vertices

	// Define vertices, normals, and texture coordinates
	auto vertices = std::vector<glm::vec3>(vertices_nb);
	auto normals = std::vector<glm::vec3>(vertices_nb, glm::vec3(0.0f, 0.0f, 1.0f)); // Assume normals point in the z direction
	auto texcoords = std::vector<glm::vec2>(vertices_nb);

	// Generate vertices and texture coordinates
	size_t index = 0u;
	for (unsigned int y = 0u; y < vertical_edges_count; ++y) {
		for (unsigned int x = 0u; x < horizontal_edges_count; ++x) {
			// Vertex position in the xy-plane
			vertices[index] = glm::vec3(
				(static_cast<float>(x) / horizontal_split_count * width) - (width / 2.0f), // x
				(static_cast<float>(y) / vertical_split_count * height) - (height / 2.0f), // y
				0.0f // z (fixed at 0 for xy-plane)
			);

			// Texture coordinates
			texcoords[index] = glm::vec2(
				static_cast<float>(x) / horizontal_split_count, // u
				static_cast<float>(y) / vertical_split_count   // v
			);

			++index;
		}
	}

	// Create index array
	auto index_sets = std::vector<glm::uvec3>(2u * horizontal_split_count * vertical_split_count);

	// Generate indices
	index = 0u;
	for (unsigned int y = 0u; y < vertical_split_count; ++y) {
		for (unsigned int x = 0u; x < horizontal_split_count; ++x) {
			index_sets[index++] = glm::uvec3(
				vertical_edges_count * y + x,
				vertical_edges_count * y + x + 1,
				vertical_edges_count * (y + 1) + x + 1
			);
			index_sets[index++] = glm::uvec3(
				vertical_edges_count * y + x,
				vertical_edges_count * (y + 1) + x + 1,
				vertical_edges_count * (y + 1) + x
			);
		}
	}

	// Create mesh_data
	bonobo::mesh_data data;
	glGenVertexArrays(1, &data.vao);
	assert(data.vao != 0u);
	glBindVertexArray(data.vao);

	// Calculate buffer sizes
	auto const vertices_size = static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec3));
	auto const normals_size = static_cast<GLsizeiptr>(normals.size() * sizeof(glm::vec3));
	auto const texcoords_size = static_cast<GLsizeiptr>(texcoords.size() * sizeof(glm::vec2));
	auto const bo_size = vertices_size + normals_size + texcoords_size;

	// Set up VBO
	glGenBuffers(1, &data.bo);
	assert(data.bo != 0u);
	glBindBuffer(GL_ARRAY_BUFFER, data.bo);
	glBufferData(GL_ARRAY_BUFFER, bo_size, nullptr, GL_STATIC_DRAW);

	// Vertex data
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices_size, vertices.data());
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(0));

	// Normal data
	glBufferSubData(GL_ARRAY_BUFFER, vertices_size, normals_size, normals.data());
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::normals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::normals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(vertices_size));

	// Texture coordinate data
	glBufferSubData(GL_ARRAY_BUFFER, vertices_size + normals_size, texcoords_size, texcoords.data());
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(vertices_size + normals_size));

	// Set up index buffer
	data.indices_nb = static_cast<GLsizei>(index_sets.size() * 3u);
	glGenBuffers(1, &data.ibo);
	assert(data.ibo != 0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(index_sets.size() * sizeof(glm::uvec3)), index_sets.data(), GL_STATIC_DRAW);

	// Unbind
	glBindVertexArray(0u);
	glBindBuffer(GL_ARRAY_BUFFER, 0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

	return data;
}
