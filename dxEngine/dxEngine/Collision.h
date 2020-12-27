#pragma once

#include"CollisionPrimitive.h"

class Collision
{
public:
	/// <summary>
	/// ���̂Ƌ��̂̓����蔻��
	/// </summary>
	/// <param name="sphere1">����1</param>
	/// <param name="sphere2">����2</param>
	/// <param name="inter">��_</param>
	/// <returns>��������</returns>
	static bool CheckSphere2Sphere(
		const Col_Sphere& sphere1,
		const Col_Sphere& sphere2,
		DirectX::XMVECTOR* inter = nullptr
	);

	/// <summary>
	/// ���̂ƕ��ʂ̓����蔻��
	/// </summary>
	/// <param name="sphere">����</param>
	/// <param name="plane">����</param>
	/// <param name="inter">��_</param>
	/// <returns>��������</returns>
	static bool CheckSphere2Plane(
		const Col_Sphere& sphere,
		const Col_Plane& plane,
		DirectX::XMVECTOR* inter = nullptr
	);

	/// <summary>
	/// �_�ƎO�p�`�̍ŋߐړ_�����߂�
	/// </summary>
	/// <param name="point">�_</param>
	/// <param name="triangle">�O�p�`</param>
	/// <param name="closest">�ŋߐړ_</param>
	static void ClosestPtPoint2Triangle(
		const DirectX::XMVECTOR& point,
		const Col_Triangle& triangle,
		DirectX::XMVECTOR* closest
	);

	/// <summary>
	/// ���Ɩ@���t���O�p�`�̓�����`�F�b�N
	/// </summary>
	/// <param name="sphere">��</param>
	/// <param name="triangle">�O�p�`</param>
	/// <param name="inter">��_(�O�p�`��̍ŋߐړ_)</param>
	/// <returns>�������Ă邩�ǂ���</returns>
	static bool CheckSphere2Triangle(
		const Col_Sphere& sphere,
		const Col_Triangle& triangle,
		DirectX::XMVECTOR* inter = nullptr
	);

	/// <summary>
	/// ���C�ƕ��ʂ̓����蔻��
	/// </summary>
	/// <param name="lay">���C</param>
	/// <param name="plane">����</param>
	/// <param name="distance">����</param>
	/// <param name="inter">��_</param>
	/// <returns>��������</returns>
	static bool CheckLay2Plane(
		const Col_Lay& lay,
		const Col_Plane& plane,
		float* distance = nullptr,
		DirectX::XMVECTOR* inter = nullptr
	);

	/// <summary>
	/// ���C�Ɩ@���t���O�p�`�̓����蔻��
	/// </summary>
	/// <param name="lay">���C</param>
	/// <param name="triangle">�O�p�`</param>
	/// <param name="distance">����</param>
	/// <param name="inter">��_</param>
	/// <returns>��������</returns>
	static bool CheckLay2Triangle(
		const Col_Lay& lay,
		const Col_Triangle& triangle,
		float* distance = nullptr,
		DirectX::XMVECTOR* inter = nullptr
	);

	/// <summary>
	/// ���C�Ƌ��̓����蔻��
	/// </summary>
	/// <param name="lay">���C</param>
	/// <param name="sphere">��</param>
	/// <param name="distance">����</param>
	/// <param name="inter">��_</param>
	/// <returns>��������</returns>
	static bool CheckLay2Sphere(
		const Col_Lay& lay,
		const Col_Sphere& sphere,
		float* distance = nullptr,
		DirectX::XMVECTOR* inter = nullptr
	);
};

