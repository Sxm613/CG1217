#include "CelestialBody.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

#include "core/helpers.hpp"
#include "core/Log.h"



CelestialBody::CelestialBody(bonobo::mesh_data const& shape,
                             GLuint const* program,
                             GLuint diffuse_texture_id)
{
	_body.node.set_geometry(shape);
	_body.node.add_texture("diffuse_texture", diffuse_texture_id, GL_TEXTURE_2D);
	_body.node.set_program(program);
}

std::pair<glm::mat4, glm::mat4> CelestialBody::render(std::chrono::microseconds elapsed_time,
                                glm::mat4 const& view_projection,
                                glm::mat4 const& parent_transform,
                                bool show_basis)
{
	// Convert the duration from microseconds to seconds.
	auto const elapsed_time_s = std::chrono::duration<float>(elapsed_time).count();
	// If a different ratio was needed, for example a duration in
	// milliseconds, the following would have been used:
	// auto const elapsed_time_ms = std::chrono::duration<float, std::milli>(elapsed_time).count();
	glm::mat4 identity_matrix = glm::mat4(1.0f);

	glm::mat4 scaling_matrix = computeScalingMatrix(identity_matrix, _body.scale);//
	//glm::mat4 world = parent_transform * scaling_matrix;

	//_body.spin.rotation_angle = -glm::half_pi<float>() / 2.0f;


	_body.spin.rotation_angle += _body.spin.speed * elapsed_time_s;

	_body.orbit.rotation_angle += _body.orbit.speed * elapsed_time_s;

	glm::mat4 orbit_tilt_matrix = glm::rotate(identity_matrix, _body.orbit.inclination, glm::vec3(1.0f, 0.0f, 0.0f));

	glm::mat4 orbit_matrix = glm::rotate(orbit_tilt_matrix, _body.orbit.rotation_angle, glm::vec3(0.0f, 1.0f, 0.0f));
	//glm::mat4 orbit_matrix = glm::rotate(identity_matrix, _body.orbit.rotation_angle, glm::vec3(0.0f, 1.0f, 0.0f));

		//new position
	float x = _body.orbit.radius * cos(_body.orbit.rotation_angle);
	float y = _body.orbit.radius * sin(_body.orbit.rotation_angle);

	// 使用平移矩阵设置物体的位置
	//glm::mat4 orbit_matrix = glm::translate(identity_matrix, glm::vec3(x, y, 0.0f));

	glm::mat4 tilt_matrix = glm::rotate(identity_matrix, _body.spin.axial_tilt, glm::vec3(1.0f, 0.0f, 0.0f));

	glm::mat4 spin_matrix = glm::rotate(tilt_matrix, _body.spin.rotation_angle, glm::vec3(0.0f, 1.0f, 0.0f));
	//glm::mat4 spin_matrix = glm::rotate(identity_matrix, _body.spin.rotation_angle, glm::vec3(0.0f, 1.0f, 0.0f));


	// Translate along X axis.
	glm::mat4 moveX_matrix = glm::translate(identity_matrix, glm::vec3(_body.orbit.radius, 0.0f, 0.0f));

	// Combine all transformations.
	glm::mat4 orbit_transform = orbit_matrix *moveX_matrix; // First, apply orbit rotation and translation
	//glm::mat4 orbit_transform =  orbit_tilt_matrix * orbit_matrix;
	glm::mat4 spin_transform = spin_matrix * scaling_matrix; // Then, apply spin (with tilt) and scaling

	glm::mat4 world = parent_transform * orbit_transform * spin_transform;

	glm::mat4 parent = parent_transform * orbit_transform * tilt_matrix;

	if (show_basis)
	{
		bonobo::renderBasis(1.0f, 2.0f, view_projection, world);
	}

	// Note: The second argument of `node::render()` is supposed to be the
	// parent transform of the node, not the whole world matrix, as the
	// node internally manages its local transforms. However in our case we
	// manage all the local transforms ourselves, so the internal transform
	// of the node is just the identity matrix and we can forward the whole
	// world matrix.
	_body.node.render(view_projection, world);

	/*if (_body.is_earth)
	{
		//original vector
		glm::vec3 vec(_body.orbit.radius, 0.0f, 0.0f);

		//rotate angle
		//float angle = _body.orbit.rotation_angle *(glm::half_pi<float>() / 90.0);
		float angle = glm::radians(_body.orbit.rotation_angle);
		//float sine_value = sin(angle); 
		//float cosine_value = cos(angle);

		//set rotate matrix
		glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, (glm::vec3(0.0f, 1.0f, 0.0f))); // Y

		//apply rotate matrix
		glm::vec4 rotatedVec = rotationMatrix * glm::vec4(vec, 1.0f);

		//get direct translation vector
		glm::vec3 Direct_offset_vector = glm::vec3(rotatedVec.x, rotatedVec.y, rotatedVec.z);

		glm::mat4 Direct_Translation_matrix = glm::translate(glm::mat4(1.0f), Direct_offset_vector);

		world = parent_transform * orbit_tilt_matrix * Direct_Translation_matrix * spin_transform;
		//world = parent_transform * R2o_matrix * R1o_matrix * Translation_matrix * R2s_matrix * Rspin_matrix * scaling_matrix;

	}*/


	if (_ring.is_set)
	{
		// 计算环的变换矩阵
		glm::mat4 ring_transform = glm::scale(glm::mat4(1.0f), glm::vec3(_ring.scale, 1.0f));
		ring_transform = glm::rotate(ring_transform, glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate 90 degrees around x-axis

		// 将环的变换矩阵与天体的变换矩阵结合
		glm::mat4 ring_world = parent_transform * orbit_transform * spin_matrix * ring_transform;

		// 渲染环
		_ring.node.render(view_projection, ring_world);
	}

	//return parent_transform;
	return std::make_pair(world, parent);
}

void CelestialBody::add_child(CelestialBody* child)
{
	_children.push_back(child);
}

std::vector<CelestialBody*> const& CelestialBody::get_children() const
{
	return _children;
}

void CelestialBody::set_orbit(OrbitConfiguration const& configuration)
{
	_body.orbit.radius = configuration.radius;
	_body.orbit.inclination = configuration.inclination;
	_body.orbit.speed = configuration.speed;
	_body.orbit.rotation_angle = 0.0f;
}

void CelestialBody::set_scale(glm::vec3 const& scale)
{
	_body.scale = scale;
}

void CelestialBody::set_spin(SpinConfiguration const& configuration)
{
	_body.spin.axial_tilt = configuration.axial_tilt;
	_body.spin.speed = configuration.speed;
	_body.spin.rotation_angle = 0.0f;
}

void CelestialBody::set_ring(bonobo::mesh_data const& shape,
                             GLuint const* program,
                             GLuint diffuse_texture_id,
                             glm::vec2 const& scale)
{
	_ring.node.set_geometry(shape);
	_ring.node.add_texture("diffuse_texture", diffuse_texture_id, GL_TEXTURE_2D);
	_ring.node.set_program(program);

	_ring.scale = scale;

	_ring.is_set = true;
}

glm::mat4 CelestialBody::computeScalingMatrix(glm::mat4 const& matrix, glm::vec3 const& scale)
{

	glm::mat4 scaling_matrix = glm::scale(matrix, scale);

	return scaling_matrix;
}


/*void CelestialBody::isset()
{
	_body.is_earth = true;
}*/
