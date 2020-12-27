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
		assert(!"オブジェクトマネージャーのインスタンスが存在しません");
	}

	instance->device = dev;

	return instance;
}

void GameObjectManager::CheckCollision(){
	for (auto hittedObj : gameobjsList) {
		for (auto hitObj : gameobjsList) {
			//同じオブジェクトで当たり判定をとらないように
			if (hittedObj == hitObj)continue;

			//当たっていたら当たり判定呼び出し
			if (Collision::CheckSphere2Sphere(
				hittedObj->GetCollisionData(),
				hitObj->GetCollisionData()))
				hittedObj->Collision(*hitObj);
		}
	}
}
