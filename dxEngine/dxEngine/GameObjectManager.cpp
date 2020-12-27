#include "GameObjectManager.h"
#include"Collision.h"

GameObjectManager::~GameObjectManager(){
}

void GameObjectManager::Initialize(){
	gameobjsList.clear();
}

void GameObjectManager::Update(){
	for (auto obj : gameobjsList) {
		obj->Update();
	}

	CheckCollision();
}

void GameObjectManager::Draw(
	ID3D12GraphicsCommandList* cmdList
){
	for (auto obj : gameobjsList) {
		obj->Draw(cmdList);
	}
}

void GameObjectManager::Terminate(){
	for (auto obj : gameobjsList) {
		obj->Terminate();
	}
	delete this;
}

void GameObjectManager::AddGameObjectsList(
	GameObject* gameobj
){
	gameobjsList.emplace_back(gameobj);
}

GameObjectManager* GameObjectManager::Craete(
	ID3D12Device* dev
){
	GameObjectManager* instance = new GameObjectManager;

	if (!instance) {
		assert(!"�I�u�W�F�N�g�}�l�[�W���[�̃C���X�^���X�����݂��܂���");
	}

	instance->device = dev;

	return instance;
}

void GameObjectManager::CheckCollision(){
	for (auto hittedObj : gameobjsList) {
		for (auto hitObj : gameobjsList) {
			//�����I�u�W�F�N�g�œ����蔻����Ƃ�Ȃ��悤��
			if (hittedObj == hitObj)continue;

			//�������Ă����瓖���蔻��Ăяo��
			if (Collision::CheckSphere2Sphere(
				hittedObj->GetCollisionData(),
				hitObj->GetCollisionData()))
				hittedObj->Collision(*hitObj);
		}
	}
}
