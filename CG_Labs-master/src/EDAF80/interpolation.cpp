#include "interpolation.hpp"


glm::vec3
interpolation::evalLERP(glm::vec3 const& p0, glm::vec3 const& p1, float const x)
{
	//! \todo Implement this function
	//return (1.0f - x) * p0 + x * p1;
	return glm::vec3();
}

glm::vec3
interpolation::evalCatmullRom(glm::vec3 const& p0, glm::vec3 const& p1,
                              glm::vec3 const& p2, glm::vec3 const& p3,
                              float const t, float const x)
{
	//! \todo Implement this function
	//return glm::vec3();
	// Catmull-Rom ��ֵ����
	glm::mat4 M = glm::mat4(
		0.0f, 1.0f, 0.0f, 0.0f,
		-t, 0.0f, t, 0.0f,
		2.0f * t, t - 3.0f, 3.0f - 2.0f * t, -t,
		-t, 2.0f - t, t - 2.0f, t
	);

	// �����Ƶ���ϳ� vec4
	glm::vec4 P0 = glm::vec4(p0, 1.0f); // �� vec3 ת��Ϊ vec4
	glm::vec4 P1 = glm::vec4(p1, 1.0f);
	glm::vec4 P2 = glm::vec4(p2, 1.0f);
	glm::vec4 P3 = glm::vec4(p3, 1.0f);

	// ʹ�� t ���� Catmull-Rom ��ֵ
	glm::vec4 pos = M * glm::vec4(1.0f, x, x * x, x * x * x);

	// ��������λ��
	glm::vec4 result = P0 * pos.x + P1 * pos.y + P2 * pos.z + P3 * pos.w;

	return glm::vec3(result); // ���ؽ��ת��Ϊ vec3
}

