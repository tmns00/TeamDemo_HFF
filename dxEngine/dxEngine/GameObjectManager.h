#pragma once

#include<d3dx12.h>

#include"GameObject.h"

class GameObjectManager
{
private:
	//�Q�[���I�u�W�F�N�g�i�[�R���e�i
	std::vector<GameObject*> gameobjsList;

	//�f�o�C�X
	ID3D12Device* device = nullptr;

private:
	/// <summary>
	/// �R���X�g���N�^
	/// </summary>
	GameObjectManager() {};

	//�R�s�[�E����֎~
	GameObjectManager(const GameObjectManager&) = delete;
	void operator=(const GameObjectManager) = delete;

public:
	///�f�X�g���N�^
	~GameObjectManager();

	/// <summary>
	/// ������
	/// </summary>
	void Initialize();

	/// <summary>
	/// �X�V
	/// </summary>
	void Update();

	/// <summary>
	/// �`��
	/// </summary>
	/// <param name="cmdList">�R�}���h���X�g</param>
	void Draw(
		ID3D12GraphicsCommandList* cmdList
	);

	/// <summary>
	/// �폜
	/// </summary>
	void Terminate();

	/// <summary>
	/// �Q�[���I�u�W�F�N�g���X�g�ɒǉ�
	/// </summary>
	/// <param name="gameobj">�ǉ�����Q�[���I�u�W�F�N�g</param>
	void AddGameObjectsList(
		GameObject* gameobj
	);

public:
	/// <summary>
	/// �N���X�̐���
	/// </summary>
	/// <param name="dev">�f�o�C�X</param>
	/// <returns>�N���X�̃|�C���^�[</returns>
	static GameObjectManager* Craete(
		ID3D12Device* dev
	);

private:
	/// <summary>
	/// ���X�g�ɓ����Ă���Q�[���I�u�W�F�N�g���m�̓����蔻��
	/// </summary>
	void CheckCollision();
};

