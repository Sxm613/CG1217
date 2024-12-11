#include "assignment5.hpp"

#include "config.hpp"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/helpers.hpp"
#include "core/ShaderProgramManager.hpp"

#include "parametric_shapes.hpp"
#include "core/node.hpp"

#include <imgui.h>
#include <tinyfiledialogs.h>

#include <clocale>
#include <stdexcept>

#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <random>
template <typename T>
T clamp(T value, T min, T max) {
	return (value < min) ? min : (value > max) ? max : value;
}

edaf80::Assignment5::Assignment5(WindowManager& windowManager) :
	mCamera(0.5f * glm::half_pi<float>(),
		static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
		0.01f, 10000.0f),
	inputHandler(), mWindowManager(windowManager), window(nullptr)
{
	WindowManager::WindowDatum window_datum{ inputHandler, mCamera, config::resolution_x, config::resolution_y, 0, 0, 0, 0 };

	window = mWindowManager.CreateGLFWWindow("EDAF80: Assignment 5", window_datum, config::msaa_rate);
	if (window == nullptr) {
		throw std::runtime_error("Failed to get a window: aborting!");
	}

	bonobo::init();
}

edaf80::Assignment5::~Assignment5()
{
	bonobo::deinit();
}

void
edaf80::Assignment5::run()
{
	// Set up the camera

	//mCamera.mWorld.SetTranslate(glm::vec3(0.0f, 0.0f, 6.0f));
	mCamera.mWorld.SetTranslate(glm::vec3(100.0f, 0.0f, -50.0f));
	mCamera.mMouseSensitivity = glm::vec2(0.003f);
	mCamera.mMovementSpeed = glm::vec3(50.0f); // 3 m/s => 10.8 km/h
	auto camera_position = mCamera.mWorld.GetTranslation();



	// Create the shader programs
	ShaderProgramManager program_manager;
	GLuint fallback_shader = 0u;
	program_manager.CreateAndRegisterProgram("Fallback",
		{ { ShaderType::vertex, "common/fallback.vert" },
		  { ShaderType::fragment, "common/fallback.frag" } },
		fallback_shader);
	if (fallback_shader == 0u) {
		LogError("Failed to load fallback shader");
		return;
	}

	//
	// Todo: Insert the creation of other shader programs.
	//       (Check how it was done in assignment 3.)
	//
	GLuint diffuse_shader = 0u;
	program_manager.CreateAndRegisterProgram("Diffuse",
		{ { ShaderType::vertex, "EDAF80/diffuse.vert" },
		  { ShaderType::fragment, "EDAF80/diffuse.frag" } },
		diffuse_shader);
	if (diffuse_shader == 0u)
		LogError("Failed to load diffuse shader");

	GLuint normal_shader = 0u;
	program_manager.CreateAndRegisterProgram("Normal",
		{ { ShaderType::vertex, "EDAF80/normal.vert" },
		  { ShaderType::fragment, "EDAF80/normal.frag" } },
		normal_shader);
	if (normal_shader == 0u)
		LogError("Failed to load normal shader");

	GLuint texcoord_shader = 0u;
	program_manager.CreateAndRegisterProgram("Texture coords",
		{ { ShaderType::vertex, "EDAF80/texcoord.vert" },
		  { ShaderType::fragment, "EDAF80/texcoord.frag" } },
		texcoord_shader);
	if (texcoord_shader == 0u)
		LogError("Failed to load texcoord shader");

	GLuint skybox_shader = 0u;
	program_manager.CreateAndRegisterProgram("Skybox",
		{ { ShaderType::vertex, "EDAF80/skybox.vert" },
		  { ShaderType::fragment, "EDAF80/skybox.frag" } },
		skybox_shader);
	if (skybox_shader == 0u)
		LogError("Failed to load skybox shader");

	GLuint water_shader = 0u;
	program_manager.CreateAndRegisterProgram("water",
		{ { ShaderType::vertex, "EDAF80/water.vert" },
		  { ShaderType::fragment, "EDAF80/water.frag" } },
		water_shader);
	if (water_shader == 0u)
		LogError("Failed to load water shader");

	GLuint ring_shader = 0u;
	program_manager.CreateAndRegisterProgram("ring",
		{ { ShaderType::vertex, "EDAF80/ring.vert" },
		  { ShaderType::fragment, "EDAF80/ring.frag" } },
		ring_shader);
	if (ring_shader == 0u)
		LogError("Failed to load ring shader");

	auto light_position = glm::vec3(-2.0f, 4.0f, 2.0f);
	auto const set_uniforms = [&light_position](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
	};

	bool use_normal_mapping = false;
	float elapsed_time_s = 0.0f;
	auto const ring_set_uniforms = [&light_position, &camera_position, &elapsed_time_s](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));

	};

	auto const water_set_uniforms = [&light_position, &camera_position, &elapsed_time_s](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
		glUniform1f(glGetUniformLocation(program, "elapsed_time_s"), elapsed_time_s);
	};


	//
	// Todo: Load your geometry
	//

	auto ring_shape = parametric_shapes::createTorus(20.0f, 2.0f, 1000, 1000);
	if (ring_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the shape");
		return;
	}
	GLuint roughnessTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\sponza\\textures\\sponza_ceiling_a_ddn.png", true);

	std::array<glm::vec3, 9> control_point_locations;

	std::random_device rd;  // 获取随机数种子
	std::mt19937 gen(rd()); // 随机数引擎
	std::uniform_real_distribution<float> disX(-500.0f, 500.0f);
	std::uniform_real_distribution<float> disY(0.0f, 10.0f);
	std::uniform_real_distribution<float> disZ(-500.0f, 500.0f);

	for (auto& point : control_point_locations) {
		point = glm::vec3(disX(gen), disY(gen), disZ(gen));
	}

	std::array<Node, control_point_locations.size()> rings;
	for (std::size_t i = 0; i < control_point_locations.size(); ++i) {
		auto& control_point = rings[i];
		// 创建环的节点
		Node ring;
		bonobo::material_data ring_material;
		ring_material.ambient = glm::vec3(0.1f, 0.1f, 0.1f);
		ring_material.shininess = 10.0f;

		// 设置几何体、材质和程序
		ring.set_geometry(ring_shape);
		ring.set_material_constants(ring_material);
		ring.set_program(&ring_shader, ring_set_uniforms);
		/*ring.add_texture("skybox", skybox_texture, GL_TEXTURE_CUBE_MAP);*/
		ring.add_texture("specularTexture", roughnessTexture, GL_TEXTURE_2D);

		// 设置位置
		ring.get_transform().SetTranslate(control_point_locations[i]);

		// 将环添加到控制点中
		control_point = ring;
	}

	//bonobo::material_data ring_material;
	//ring_material.ambient = glm::vec3(0.1f, 0.1f, 0.1f);
	//ring_material.shininess = 10.0f;
	//Node ring;
	//ring.set_geometry(ring_shape);
	//ring.set_material_constants(ring_material);
	//ring.set_program(&ring_shader, ring_set_uniforms);
	//ring.add_texture("normalTexture", normalTexture, GL_TEXTURE_2D);
	//ring.add_texture("skybox", skybox_texture, GL_TEXTURE_CUBE_MAP);

	auto water_shape = parametric_shapes::createQuad(7000.0f, 7000.0f, 1000, 1000);
	//auto water_shape = parametric_shapes::createQuad(100.0f, 100.0f, 1000, 1000);
	if (water_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the shape");
		return;
	}
	GLuint normalTexture = bonobo::loadTexture2D("C:\\Users\\Ximeng\\Desktop\\CG_Labs-master\\CG_Labs-master\\res\\textures\\waves.png", true);
	if (normalTexture == 0) {
		LogError("Failed to load normal texture");
		return;
	}
	bonobo::material_data water_material;
	water_material.ambient = glm::vec3(0.1f, 0.1f, 0.1f);
	water_material.shininess = 10.0f;
	Node water;
	water.set_geometry(water_shape);
	water.set_material_constants(water_material);
	water.set_program(&water_shader, water_set_uniforms);
	water.add_texture("normalTexture", normalTexture, GL_TEXTURE_2D);
	/*water.add_texture("skybox", skybox_texture, GL_TEXTURE_CUBE_MAP);*/
	water.get_transform().SetTranslate(glm::vec3(0.0f, -50.0f, 0.0f));


	auto mouse_shape = parametric_shapes::createSphere(5.0f, 20u, 20u);
	if (mouse_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the mouse cursor");
		return;
	}
	Node mouse;
	mouse.set_geometry(mouse_shape);
	mouse.set_program(&diffuse_shader, set_uniforms);



	glClearDepthf(1.0f);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);


	auto lastTime = std::chrono::high_resolution_clock::now();

	bool show_logs = true;
	bool show_gui = true;
	bool shader_reload_failed = false;
	bool show_basis = false;
	float basis_thickness_scale = 1.0f;
	float basis_length_scale = 1.0f;
	auto polygon_mode = bonobo::polygon_mode_t::fill;

	while (!glfwWindowShouldClose(window)) {
		// 运动更新逻辑
		auto const nowTime = std::chrono::high_resolution_clock::now();
		auto const deltaTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(nowTime - lastTime);
		lastTime = nowTime;
		elapsed_time_s += std::chrono::duration<float>(deltaTimeUs).count();

		auto& io = ImGui::GetIO();
		inputHandler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);

		glfwPollEvents();
		inputHandler.Advance();
		mCamera.Update(deltaTimeUs, inputHandler);

		const glm::vec3 minPosition(-3000.0f, -10.0f, -3000.0f);
		const glm::vec3 maxPosition(3000.0f, 50.0f, 3000.0f);
		glm::mat4 worldMatrix = mCamera.mWorld.GetMatrix(); // 获取变换矩阵
		glm::vec3 forward = glm::normalize(glm::vec3(worldMatrix[2])); // 前向向量
		glm::vec3 right = glm::normalize(glm::vec3(worldMatrix[0])); // 右向向量


		// 检查按键输入并设置加速度
		if (inputHandler.GetKeycodeState(GLFW_KEY_W) & PRESSED) {
			cameraAcceleration = -forward * 20.0f; // 向前加速
		}
		else if (inputHandler.GetKeycodeState(GLFW_KEY_S) & PRESSED) {
			cameraAcceleration = forward * 20.0f; // 向后加速
		}
		else if (inputHandler.GetKeycodeState(GLFW_KEY_A) & PRESSED) {
			cameraAcceleration = right * 20.0f; // 向左加速
		}
		else if (inputHandler.GetKeycodeState(GLFW_KEY_D) & PRESSED) {
			cameraAcceleration = -right * 20.0f; // 向右加速
		}
		else {
			cameraAcceleration = glm::vec3(0.0f); // 停止加速
		}

		// 更新速度
		float dt = std::chrono::duration<float>(deltaTimeUs).count();
		cameraVelocity += cameraAcceleration * dt;

		// 更新位置
		auto cameraPosition = mCamera.mWorld.GetTranslation();
		cameraPosition += cameraVelocity * dt;

		// 平滑停止
		if (cameraAcceleration == glm::vec3(0.0f)) {
			cameraVelocity *= 0.95f; // 阻尼
		}

		// 限制相机位置
		cameraPosition.x = glm::clamp(cameraPosition.x, minPosition.x, maxPosition.x);
		cameraPosition.y = glm::clamp(cameraPosition.y, minPosition.y, maxPosition.y);
		cameraPosition.z = glm::clamp(cameraPosition.z, minPosition.z, maxPosition.z);
		mCamera.mWorld.SetTranslate(cameraPosition);

		if (inputHandler.GetKeycodeState(GLFW_KEY_R) & JUST_PRESSED) {
			shader_reload_failed = !program_manager.ReloadAllPrograms();
			if (shader_reload_failed)
				tinyfd_notifyPopup("Shader Program Reload Error",
					"An error occurred while reloading shader programs; see the logs for details.\n"
					"Rendering is suspended until the issue is solved. Once fixed, just reload the shaders again.",
					"error");
		}
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


		//
		// Todo: If you need to handle inputs, you can do it here
		//


		mWindowManager.NewImGuiFrame();

		// 设置鼠标位置，距离相机一定的偏移量
		auto camera = mCamera.mWorld.GetTranslation();

		glm::vec3 forward1 = glm::normalize(glm::vec3(mCamera.mWorld.GetMatrix()[2])); // 更新前向向量
		float mouse_offset_distance = -50.0f; // 根据需要调整这个值
		glm::vec3 mouse_position = camera + forward1 * mouse_offset_distance;
		// 更新鼠标位置
		mouse.get_transform().SetTranslate(mouse_position);

		for (size_t i = 0; i < rings.size(); ++i) {
			glm::vec3 ringPosition = rings[i].get_transform().GetTranslation();
			glm::vec3 mousePosition = mouse.get_transform().GetTranslation();

			float majorRadius = 20.0f; // 圆环的主半径
			float minorRadius = 5.0f;   // 圆环的次半径

			// 计算点到圆环中心的向量
			glm::vec3 toPoint = mousePosition - ringPosition;
			// 计算在 XY 平面上的距离
			float distanceFromCenter = glm::length(glm::vec2(toPoint.x, toPoint.y));

			// 检查是否在主半径和次半径范围内
			if (distanceFromCenter < majorRadius + minorRadius) {
				// 计算高度（Z 轴的差值）
				float height = toPoint.z;

				// 检查是否在次半径范围内
				if (height * height + (distanceFromCenter - majorRadius) * (distanceFromCenter - majorRadius) < minorRadius * minorRadius) {
					//std::cout << "Collision detected between the ball and ring!" << std::endl;
					
					// 计算碰撞方向
					glm::vec3 direction = glm::normalize(mousePosition - ringPosition);

					// 调整鼠标位置，防止穿透
					mouse.get_transform().SetTranslate(ringPosition + direction * (majorRadius + minorRadius));

				}
			}
		}




		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		bonobo::changePolygonMode(polygon_mode);


		if (!shader_reload_failed) {
			//
			// Todo: Render all your geometry here.
			//
		/*	skybox.render(mCamera.GetWorldToClipMatrix());*/
			for (const auto& ring : rings) { // 遍历所有环并渲染
				ring.render(mCamera.GetWorldToClipMatrix());
			}
			water.render(mCamera.GetWorldToClipMatrix());
			mouse.render(mCamera.GetWorldToClipMatrix());
		}


		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//
		// Todo: If you want a custom ImGUI window, you can set it up
		//       here
		//
		bool const opened = ImGui::Begin("Scene Controls", nullptr, ImGuiWindowFlags_None);
		if (opened) {
			ImGui::Checkbox("Show basis", &show_basis);
			ImGui::SliderFloat("Basis thickness scale", &basis_thickness_scale, 0.0f, 100.0f);
			ImGui::SliderFloat("Basis length scale", &basis_length_scale, 0.0f, 100.0f);
			bonobo::uiSelectPolygonMode("Polygon mode", polygon_mode);

			ImGui::Text("Camera Velocity: (%.2f, %.2f, %.2f)", cameraVelocity.x, cameraVelocity.y, cameraVelocity.z);
		}
		ImGui::End();

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
		edaf80::Assignment5 assignment5(framework.GetWindowManager());
		assignment5.run();
	}
	catch (std::runtime_error const& e) {
		LogError(e.what());
	}
}
