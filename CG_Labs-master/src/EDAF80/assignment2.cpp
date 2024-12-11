#include "assignment2.hpp"
#include "parametric_shapes.hpp"

#include "config.hpp"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/helpers.hpp"
#include "core/node.hpp"
#include "core/ShaderProgramManager.hpp"
#include <imgui.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <clocale>
#include <cstdlib>
#include <stdexcept>

edaf80::Assignment2::Assignment2(WindowManager& windowManager) :
	mCamera(0.5f * glm::half_pi<float>(),
	        static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
	        0.01f, 1000.0f),
	inputHandler(), mWindowManager(windowManager), window(nullptr)
{
	WindowManager::WindowDatum window_datum{ inputHandler, mCamera, config::resolution_x, config::resolution_y, 0, 0, 0, 0};

	window = mWindowManager.CreateGLFWWindow("EDAF80: Assignment 2", window_datum, config::msaa_rate);
	if (window == nullptr) {
		throw std::runtime_error("Failed to get a window: aborting!");
	}

	bonobo::init();
}

edaf80::Assignment2::~Assignment2()
{
	bonobo::deinit();
}
void
edaf80::Assignment2::run()
{

	// Set up the camera
	mCamera.mWorld.SetTranslate(glm::vec3(0.0f, -2.0f, 10.0f));
	mCamera.mMouseSensitivity = glm::vec2(0.003f);
	mCamera.mMovementSpeed = glm::vec3(3.0f); // 3 m/s => 10.8 km/h
	auto camera_position = mCamera.mWorld.GetTranslation();


	// Create the shader programs
	ShaderProgramManager program_manager;
	GLuint diffuse_shader = 0u;
	program_manager.CreateAndRegisterProgram("Diffuse",
	                                         { { ShaderType::vertex, "EDAF80/diffuse.vert" },
	                                           { ShaderType::fragment, "EDAF80/diffuse.frag" } },
	                                         diffuse_shader);
	if (diffuse_shader == 0u)
		LogError("Failed to load diffuse shader");
	GLuint water_shader = 0u;
	program_manager.CreateAndRegisterProgram("water",
		                                     { { ShaderType::vertex, "EDAF80/water.vert" },
		                                       { ShaderType::fragment, "EDAF80/water.frag" } },
		                                     water_shader);
	if (water_shader == 0u)
		LogError("Failed to load water shader");
	GLuint phong_shader = 0u;
	program_manager.CreateAndRegisterProgram("phong",
                                             { { ShaderType::vertex, "EDAF80/phong.vert" },
		                                       { ShaderType::fragment, "EDAF80/phong.frag" } },
		                                     phong_shader);
	if (phong_shader == 0u)
		LogError("Failed to load phong shader");
	GLuint plane_phong_shader = 0u;
	program_manager.CreateAndRegisterProgram("plane_phong",
		                                     { { ShaderType::vertex, "EDAF80/plane_phong.vert" },
	                                     	   { ShaderType::fragment, "EDAF80/plane_phong.frag" } },
		                                     plane_phong_shader);
	if (plane_phong_shader == 0u)
		LogError("Failed to load plane_phong shader");

	float elapsed_time_s = 0.0f;
	auto const light_position = glm::vec3(-2.0f, 4.0f, 2.0f);
	auto const color_white = glm::vec3(1.0f, 1.0f, 1.0f);
	auto const color_black = glm::vec3(0.0f, 0.0f, 0.0f);

	auto const white_set_uniforms = [&light_position, &color_white](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "object_color"), 1, glm::value_ptr(color_white));
	};
	auto const black_set_uniforms = [&light_position, &color_black](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "object_color"), 1, glm::value_ptr(color_black));
	};
	auto const water_set_uniforms = [&light_position, &camera_position, &elapsed_time_s](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
		glUniform1f(glGetUniformLocation(program, "elapsed_time_s"), elapsed_time_s);
	};

	float metaldisplacementScale = 0.01f;
	float metalambientScale = 0.01f;
	float metaldiffuseScale = 1.0f;
	float metalspecularScale = 0.1f;
	auto const metal_set_uniforms = [&light_position, &camera_position, metaldisplacementScale, metalambientScale, metaldiffuseScale, metalspecularScale](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
		glUniform1f(glGetUniformLocation(program, "displacementScale"), metaldisplacementScale);
		glUniform1f(glGetUniformLocation(program, "ambientScale"), metalambientScale);
		glUniform1f(glGetUniformLocation(program, "diffuseScale"), metaldiffuseScale);
		glUniform1f(glGetUniformLocation(program, "specularScale"), metalspecularScale);
	};
	float marbledisplacementScale = 0.01f;
	float marbleambientScale = 0.01f;
	float marblediffuseScale = 1.0f;
	float marblespecularScale = 1.0f;
	auto const marble_set_uniforms = [&light_position, &camera_position, marbledisplacementScale, marbleambientScale, marblediffuseScale, marblespecularScale](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
		glUniform1f(glGetUniformLocation(program, "displacementScale"), marbledisplacementScale);
		glUniform1f(glGetUniformLocation(program, "ambientScale"), marbleambientScale);
		glUniform1f(glGetUniformLocation(program, "diffuseScale"), marblediffuseScale);
		glUniform1f(glGetUniformLocation(program, "specularScale"), marblespecularScale);
	};
	float carpetdisplacementScale = 0.05f;
	float carpetambientScale = 0.01f;
	float carpetdiffuseScale = 1.0f;
	float carpetspecularScale = 1.0f;
	auto const carpet_set_uniforms = [&light_position, &camera_position, carpetdisplacementScale, carpetambientScale, carpetdiffuseScale, carpetspecularScale](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
		glUniform1f(glGetUniformLocation(program, "displacementScale"), carpetdisplacementScale);
		glUniform1f(glGetUniformLocation(program, "ambientScale"), carpetambientScale);
		glUniform1f(glGetUniformLocation(program, "diffuseScale"), carpetdiffuseScale);
		glUniform1f(glGetUniformLocation(program, "specularScale"), carpetspecularScale);
	};
	float grassdisplacementScale = 0.1f;
	float grassambientScale = 0.01f;
	float grassdiffuseScale = 1.0f;
	float grassspecularScale = 0.1f;
	auto const grass_set_uniforms = [&light_position, &camera_position, grassdisplacementScale, grassambientScale, grassdiffuseScale, grassspecularScale](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
		glUniform1f(glGetUniformLocation(program, "displacementScale"), grassdisplacementScale);
		glUniform1f(glGetUniformLocation(program, "ambientScale"), grassambientScale);
		glUniform1f(glGetUniformLocation(program, "diffuseScale"), grassdiffuseScale);
		glUniform1f(glGetUniformLocation(program, "specularScale"), grassspecularScale);
	};
	float stonedisplacementScale = 0.5f;
	float stoneambientScale = 0.01f;
	float stonediffuseScale = 1.0f;
	float stonespecularScale = 0.1f;
	float keepdisplacement = 1.0f;
	auto const stone_set_uniforms = [&light_position, &camera_position, stonedisplacementScale, stoneambientScale, stonediffuseScale, stonespecularScale, keepdisplacement](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
		glUniform1f(glGetUniformLocation(program, "displacementScale"), stonedisplacementScale);
		glUniform1f(glGetUniformLocation(program, "ambientScale"), stoneambientScale);
		glUniform1f(glGetUniformLocation(program, "diffuseScale"), stonediffuseScale);
		glUniform1f(glGetUniformLocation(program, "specularScale"), stonespecularScale);
		glUniform1f(glGetUniformLocation(program, "keepdisplacement"), keepdisplacement);
	};

	float wooddisplacementScale = 0.01f;
	float woodambientScale = 0.01f;
	float wooddiffuseScale = 1.0f;
	float woodspecularScale = 1.0f;
	auto const wood_set_uniforms = [&light_position, &camera_position, wooddisplacementScale, woodambientScale, wooddiffuseScale, woodspecularScale, keepdisplacement](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
		glUniform1f(glGetUniformLocation(program, "displacementScale"), wooddisplacementScale);
		glUniform1f(glGetUniformLocation(program, "ambientScale"), woodambientScale);
		glUniform1f(glGetUniformLocation(program, "diffuseScale"), wooddiffuseScale);
		glUniform1f(glGetUniformLocation(program, "specularScale"), woodspecularScale);
		glUniform1f(glGetUniformLocation(program, "keepdisplacement"), keepdisplacement);
	};
	float tilesdisplacementScale = 0.1f;
	float tilesambientScale = 0.01f;
	float tilesdiffuseScale = 1.0f;
	float tilesspecularScale = 0.5f;
	auto const tiles_set_uniforms = [&light_position, &camera_position, tilesdisplacementScale, tilesambientScale, tilesdiffuseScale, tilesspecularScale, keepdisplacement](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
		glUniform1f(glGetUniformLocation(program, "displacementScale"), tilesdisplacementScale);
		glUniform1f(glGetUniformLocation(program, "ambientScale"), tilesambientScale);
		glUniform1f(glGetUniformLocation(program, "diffuseScale"), tilesdiffuseScale);
		glUniform1f(glGetUniformLocation(program, "specularScale"), tilesspecularScale);
		glUniform1f(glGetUniformLocation(program, "keepdisplacement"), keepdisplacement);
	};

    keepdisplacement = 0.0f;
	auto const stoneright_set_uniforms = [&light_position, &camera_position, stonedisplacementScale, stoneambientScale, stonediffuseScale, stonespecularScale, keepdisplacement](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
		glUniform1f(glGetUniformLocation(program, "displacementScale"), stonedisplacementScale);
		glUniform1f(glGetUniformLocation(program, "ambientScale"), stoneambientScale);
		glUniform1f(glGetUniformLocation(program, "diffuseScale"), stonediffuseScale);
		glUniform1f(glGetUniformLocation(program, "specularScale"), stonespecularScale);
		glUniform1f(glGetUniformLocation(program, "keepdisplacement"), keepdisplacement);
	};


	auto const Sphere_shape = parametric_shapes::createSphere(1.0f, 100u, 100u);
	if (Sphere_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the shape");
		return;
	}
	auto const XZ_Plane = parametric_shapes::createQuad(10.0f, 24.0f, 100u, 100u);
	if (XZ_Plane.vao == 0u) {
		LogError("Failed to retrieve the mesh for the shape");
		return;
	}
	auto const YZ_Plane = parametric_shapes::createWall(10.0f, 24.0f, 100u, 100u);
	if (YZ_Plane.vao == 0u) {
		LogError("Failed to retrieve the mesh for the shape");
		return;
	}
	auto const XY_Plane = parametric_shapes::createFront(10.0f, 10.0f, 100u, 100u);
	if (XY_Plane.vao == 0u) {
		LogError("Failed to retrieve the mesh for the shape");
		return;
	}

	std::array<glm::vec3, 2> refraction_point_locations = {
	    glm::vec3(-2.0f,  -7.0f,  -5.0f),
	    glm::vec3(-3.0f,  -5.0f,  -5.0f)
	};
	std::array<glm::vec3, 2> reflection_point_locations = {
		glm::vec3(2.0f,  -7.0f,  -5.0f),
		glm::vec3(3.0f,  -5.0f,  -5.0f)
	};

	GLuint normalTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\waves.png", true);
	GLuint skybox_texture = bonobo::loadTextureCubeMap(
		"C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\cubemaps\\box\\posx.jpg",
		"C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\cubemaps\\box\\negx.jpg",
		"C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\cubemaps\\box\\posy.jpg",
		"C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\cubemaps\\box\\negy.jpg",
		"C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\cubemaps\\box\\posz.jpg",
		"C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\cubemaps\\box\\negz.jpg",
		true
	);

	GLuint metalnormalTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\metal\\metal_normal.jpg", true);
	GLuint metaldisplacementTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\metal\\metal_displacement.jpg", true);
	GLuint metalTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\metal\\metal.jpg", true);
	GLuint metalroughnessTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\metal\\metal_rough.jpg", true);
	GLuint metalambientTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\metal\\metal_ambient.jpg", true);

	GLuint marblenormalTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\marble\\marble_normal.jpg", true);
	GLuint marbledisplacementTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\marble\\marble_displacement.jpg", true);
	GLuint marbleTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\marble\\marble.jpg", true);
	GLuint marbleroughnessTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\marble\\marble_rough.jpg", true);
	GLuint marbleambientTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\marble\\marble_ambient.jpg", true);

	GLuint carpetnormalTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\carpet\\carpet_normal.jpg", true);
	GLuint carpetdisplacementTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\carpet\\carpet_displacement.jpg", true);
	GLuint carpetTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\carpet\\carpet.jpg", true);
	GLuint carpetroughnessTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\carpet\\carpet_rough.jpg", true);
	GLuint carpetambientTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\carpet\\carpet_ambient.jpg", true);

	GLuint grassnormalTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\grass\\grass_normal.jpg", true);
	GLuint grassdisplacementTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\grass\\grass_displacement.jpg", true);
	GLuint grassTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\grass\\grass.jpg", true);
	GLuint grassroughnessTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\grass\\grass_rough.jpg", true);
	GLuint grassambientTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\grass\\grass_ambient.jpg", true);

	GLuint stonenormalTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\stone\\stone_normal.jpg", true);
	GLuint stonedisplacementTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\stone\\stone_displacement.jpg", true);
	GLuint stoneTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\stone\\stone.jpg", true);
	GLuint stoneroughnessTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\stone\\stone_rough.jpg", true);
	GLuint stoneambientTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\stone\\stone_ambient.jpg", true);

	GLuint woodnormalTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\wood\\wood_normal.jpg", true);
	GLuint wooddisplacementTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\wood\\wood_displacement.jpg", true);
	GLuint woodTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\wood\\wood.jpg", true);
	GLuint woodroughnessTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\wood\\wood_rough.jpg", true);
	GLuint woodambientTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\wood\\wood_ambient.jpg", true);

	GLuint tilesnormalTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\tiles\\tiles_normal.jpg", true);
	GLuint tilesdisplacementTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\tiles\\tiles_displacement.jpg", true);
	GLuint tilesTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\tiles\\tiles.jpg", true);
	GLuint tilesroughnessTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\tiles\\tiles_rough.jpg", true);
	GLuint tilesambientTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\tiles\\tiles_ambient.jpg", true);

	bonobo::material_data metal_material;
	metal_material.shininess = 32.0f;
	bonobo::material_data marble_material;
	marble_material.shininess = 128.0f;
	bonobo::material_data carpet_material;
	carpet_material.shininess = 8.0f;
	bonobo::material_data grass_material;
	grass_material.shininess = 8.0f;
	bonobo::material_data stone_material;
	stone_material.shininess = 8.0f;
	bonobo::material_data wood_material;
	wood_material.shininess = 8.0f;
	bonobo::material_data tiles_material;
	tiles_material.shininess = 64.0f;  

	Node sphere1;
	sphere1.set_geometry(Sphere_shape);
	sphere1.set_material_constants(metal_material);
	sphere1.set_program(&phong_shader, metal_set_uniforms);
	sphere1.add_texture("normalMap", metalnormalTexture, GL_TEXTURE_2D);
	sphere1.add_texture("displacementMap", metaldisplacementTexture, GL_TEXTURE_2D);
	sphere1.add_texture("ambientlTexture", metalambientTexture, GL_TEXTURE_2D);
	sphere1.add_texture("diffuseTexture", metalTexture, GL_TEXTURE_2D);
	sphere1.add_texture("specularTexture", metalroughnessTexture, GL_TEXTURE_2D);
	sphere1.get_transform().SetTranslate(glm::vec3(-3.0f, -7.0f, -10.0f));
	Node sphere2;
	sphere2.set_geometry(Sphere_shape);
	sphere2.set_material_constants(marble_material);
	sphere2.set_program(&phong_shader, marble_set_uniforms);
	sphere2.add_texture("normalMap", marblenormalTexture, GL_TEXTURE_2D);
	sphere2.add_texture("displacementMap", marbledisplacementTexture, GL_TEXTURE_2D);
	sphere2.add_texture("ambientlTexture", marbleambientTexture, GL_TEXTURE_2D);
	sphere2.add_texture("diffuseTexture", marbleTexture, GL_TEXTURE_2D);
	sphere2.add_texture("specularTexture", marbleroughnessTexture, GL_TEXTURE_2D);
	sphere2.get_transform().SetTranslate(glm::vec3(0.0f, -7.0f, -10.0f));
	Node sphere3;
	sphere3.set_geometry(Sphere_shape);
	sphere3.set_material_constants(carpet_material);
	sphere3.set_program(&phong_shader, carpet_set_uniforms);
	sphere3.add_texture("normalMap", carpetnormalTexture, GL_TEXTURE_2D);
	sphere3.add_texture("displacementMap", carpetdisplacementTexture, GL_TEXTURE_2D);
	sphere3.add_texture("ambientlTexture", carpetambientTexture, GL_TEXTURE_2D);
	sphere3.add_texture("diffuseTexture", carpetTexture, GL_TEXTURE_2D);
	sphere3.add_texture("specularTexture", carpetroughnessTexture, GL_TEXTURE_2D);
	sphere3.get_transform().SetTranslate(glm::vec3(3.0f, -7.0f, -10.0f));

	std::array<Node, refraction_point_locations.size()> refraction_points;
	for (std::size_t i = 0; i < refraction_point_locations.size(); ++i) {
		auto& control_refraction_point = refraction_points[i];
		Node refraction_point;
		refraction_point.set_geometry(Sphere_shape);
		refraction_point.set_program(&diffuse_shader, white_set_uniforms);
		refraction_point.get_transform().SetTranslate(refraction_point_locations[i]);
		control_refraction_point = refraction_point;
	}
	std::array<Node, reflection_point_locations.size()> reflection_points;
	for (std::size_t i = 0; i < reflection_point_locations.size(); ++i) {
		auto& control_reflection_point = reflection_points[i];
		Node reflection_point;
		reflection_point.set_geometry(Sphere_shape);
		reflection_point.set_program(&diffuse_shader, black_set_uniforms);
		reflection_point.get_transform().SetTranslate(reflection_point_locations[i]);
		control_reflection_point = reflection_point;
	}


	Node ground;
	ground.set_geometry(XZ_Plane);
	ground.set_material_constants(tiles_material);
	ground.set_program(&plane_phong_shader, tiles_set_uniforms);
	ground.add_texture("normalMap", tilesnormalTexture, GL_TEXTURE_2D);
	ground.add_texture("displacementMap", tilesdisplacementTexture, GL_TEXTURE_2D);
	ground.add_texture("ambientlTexture", tilesambientTexture, GL_TEXTURE_2D);
	ground.add_texture("diffuseTexture", tilesTexture, GL_TEXTURE_2D);
	ground.add_texture("specularTexture", tilesroughnessTexture, GL_TEXTURE_2D);
	ground.get_transform().SetTranslate(glm::vec3(0.0f, -8.0f, -0.0f));
	Node ceiling;
	ceiling.set_geometry(XZ_Plane);
	ceiling.set_material_constants(wood_material);
	ceiling.set_program(&plane_phong_shader, wood_set_uniforms);
	ceiling.add_texture("normalMap", woodnormalTexture, GL_TEXTURE_2D);
	ceiling.add_texture("displacementMap", wooddisplacementTexture, GL_TEXTURE_2D);
	ceiling.add_texture("ambientlTexture", woodambientTexture, GL_TEXTURE_2D);
	ceiling.add_texture("diffuseTexture", woodTexture, GL_TEXTURE_2D);
	ceiling.add_texture("specularTexture", woodroughnessTexture, GL_TEXTURE_2D);
	ceiling.get_transform().SetTranslate(glm::vec3(0.0f, 1.0f, 0.0f));
	Node right_wall;
	right_wall.set_geometry(YZ_Plane);
	right_wall.set_material_constants(stone_material);
	right_wall.set_program(&plane_phong_shader, stoneright_set_uniforms);
	right_wall.add_texture("normalMap", stonenormalTexture, GL_TEXTURE_2D);
	right_wall.add_texture("displacementMap", stonedisplacementTexture, GL_TEXTURE_2D);
	right_wall.add_texture("ambientlTexture", stoneambientTexture, GL_TEXTURE_2D);
	right_wall.add_texture("diffuseTexture", stoneTexture, GL_TEXTURE_2D);
	right_wall.add_texture("specularTexture", stoneroughnessTexture, GL_TEXTURE_2D);
	right_wall.get_transform().SetTranslate(glm::vec3(5.0f, -4.0f, -0.0f));
	Node left_wall;
	left_wall.set_geometry(YZ_Plane);
	left_wall.set_material_constants(stone_material);
	left_wall.set_program(&plane_phong_shader, stone_set_uniforms);
	left_wall.add_texture("normalMap", stonenormalTexture, GL_TEXTURE_2D);
	left_wall.add_texture("displacementMap", stonedisplacementTexture, GL_TEXTURE_2D);
	left_wall.add_texture("ambientlTexture", stoneambientTexture, GL_TEXTURE_2D);
	left_wall.add_texture("diffuseTexture", stoneTexture, GL_TEXTURE_2D);
	left_wall.add_texture("specularTexture", stoneroughnessTexture, GL_TEXTURE_2D);
	left_wall.get_transform().SetTranslate(glm::vec3(-5.0f, -4.0f, -0.0f));
	Node front_wall;
	front_wall.set_geometry(XY_Plane);
	front_wall.set_material_constants(grass_material);
	front_wall.set_program(&plane_phong_shader, grass_set_uniforms);
	front_wall.add_texture("normalMap", grassnormalTexture, GL_TEXTURE_2D);
	front_wall.add_texture("displacementMap", grassdisplacementTexture, GL_TEXTURE_2D);
	front_wall.add_texture("ambientlTexture", grassambientTexture, GL_TEXTURE_2D);
	front_wall.add_texture("diffuseTexture", grassTexture, GL_TEXTURE_2D);
	front_wall.add_texture("specularTexture", grassroughnessTexture, GL_TEXTURE_2D);
	front_wall.get_transform().SetTranslate(glm::vec3(0.0f, -4.0f, -12.0f));
	Node behind_wall;
	behind_wall.set_geometry(XY_Plane);
	behind_wall.set_material_constants(wood_material);
	behind_wall.set_program(&plane_phong_shader, wood_set_uniforms);
	behind_wall.add_texture("normalMap", woodnormalTexture, GL_TEXTURE_2D);
	behind_wall.add_texture("displacementMap", wooddisplacementTexture, GL_TEXTURE_2D);
	behind_wall.add_texture("ambientlTexture", woodambientTexture, GL_TEXTURE_2D);
	behind_wall.add_texture("diffuseTexture", woodTexture, GL_TEXTURE_2D);
	behind_wall.add_texture("specularTexture", woodroughnessTexture, GL_TEXTURE_2D);
	behind_wall.get_transform().SetTranslate(glm::vec3(0.0f, -4.0f, 12.0f));

	bonobo::material_data water_material;
	water_material.ambient = glm::vec3(0.1f, 0.1f, 0.1f);
	water_material.shininess = 10.0f;
	Node water;
	water.set_geometry(XZ_Plane);
	water.set_material_constants(water_material);
	water.set_program(&water_shader, water_set_uniforms);
	water.add_texture("normalTexture", normalTexture, GL_TEXTURE_2D);
	water.add_texture("skybox", skybox_texture, GL_TEXTURE_CUBE_MAP);
	water.get_transform().SetTranslate(glm::vec3(0.0f, -6.0f, 0.0f));

	glClearDepthf(1.0f);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	auto lastTime = std::chrono::high_resolution_clock::now();

	std::int32_t program_index = 0;
	auto cull_mode = bonobo::cull_mode_t::disabled;
	auto polygon_mode = bonobo::polygon_mode_t::fill;
	bool show_logs = true;
	bool show_gui = true;
	bool show_basis = false;
	float basis_thickness_scale = 1.0f;
	float basis_length_scale = 1.0f;

	changeCullMode(cull_mode);

	while (!glfwWindowShouldClose(window)) {
		auto const nowTime = std::chrono::high_resolution_clock::now();
		auto const deltaTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(nowTime - lastTime);
		lastTime = nowTime;

		auto& io = ImGui::GetIO();
		inputHandler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);

		glfwPollEvents();
		inputHandler.Advance();
		mCamera.Update(deltaTimeUs, inputHandler);
		//camera_position = mCamera.mWorld.GetTranslation();

		elapsed_time_s += std::chrono::duration<float>(deltaTimeUs).count();

		if (inputHandler.GetKeycodeState(GLFW_KEY_F3) & JUST_RELEASED)
			show_logs = !show_logs;
		if (inputHandler.GetKeycodeState(GLFW_KEY_F2) & JUST_RELEASED)
			show_gui = !show_gui;
		if (inputHandler.GetKeycodeState(GLFW_KEY_F11) & JUST_RELEASED)
			mWindowManager.ToggleFullscreenStatusForWindow(window);


		// Retrieve the actual framebuffer size: for HiDPI monitors,
		// you might end up with a framebuffer larger than what you
		// actually asked for. For example, if you ask for a 1920x1080
		// framebuffer, you might get a 3840x2160 one instead.
		// Also it might change as the user drags the window between
		// monitors with different DPIs, or if the fullscreen status is
		// being toggled.
		int framebuffer_width, framebuffer_height;
		glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
		glViewport(0, 0, framebuffer_width, framebuffer_height);

		mWindowManager.NewImGuiFrame();


		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		bonobo::changePolygonMode(polygon_mode);


		sphere1.render(mCamera.GetWorldToClipMatrix());
		sphere2.render(mCamera.GetWorldToClipMatrix());
		sphere3.render(mCamera.GetWorldToClipMatrix());
	/*	for (const auto& refraction_point : refraction_points) {
			refraction_point.render(mCamera.GetWorldToClipMatrix());
		}
		for (const auto& reflection_point : reflection_points) {
			reflection_point.render(mCamera.GetWorldToClipMatrix());
		}*/
		ground.render(mCamera.GetWorldToClipMatrix());
		ceiling.render(mCamera.GetWorldToClipMatrix());
		right_wall.render(mCamera.GetWorldToClipMatrix());
		left_wall.render(mCamera.GetWorldToClipMatrix());
		front_wall.render(mCamera.GetWorldToClipMatrix());
		behind_wall.render(mCamera.GetWorldToClipMatrix());
		//water.render(mCamera.GetWorldToClipMatrix());

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		bool const opened = ImGui::Begin("Scene Controls", nullptr, ImGuiWindowFlags_None);
		if (opened) {
			ImGui::Checkbox("Show basis", &show_basis);
			ImGui::SliderFloat("Basis thickness scale", &basis_thickness_scale, 0.0f, 100.0f);
			ImGui::SliderFloat("Basis length scale", &basis_length_scale, 0.0f, 100.0f);
		}
		ImGui::End();

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		if (show_basis)
			bonobo::renderBasis(basis_thickness_scale, basis_length_scale, mCamera.GetWorldToClipMatrix());
		if (show_logs)
			Log::View::Render();
		mWindowManager.RenderImGuiFrame(show_gui);

		glfwSwapBuffers(window);
	}
}

int main()
{
	std::setlocale(LC_ALL, "");

	Bonobo framework;

	try {
		edaf80::Assignment2 assignment2(framework.GetWindowManager());
		assignment2.run();
	} catch (std::runtime_error const& e) {
		LogError(e.what());
	}
}
