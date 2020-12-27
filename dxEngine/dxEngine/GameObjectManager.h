#pragma once

#include<d3dx12.h>

#include"GameObject.h"

class GameObjectManager
{
private:
	//ゲームオブジェクト格納コンテナ
	std::vector<GameObject*> gameobjsList;

	//デバイス
	ID3D12Device* device = nullptr;

private:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	GameObjectManager() {};

	//コピー・代入禁止
	GameObjectManager(const GameObjectManager&) = delete;
	void operator=(const GameObjectManager) = delete;

public:
	///デストラクタ
	~GameObjectManager();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="cmdList">コマンドリスト</param>
	void Draw(
		ID3D12GraphicsCommandList* cmdList
	);

	/// <summary>
	/// 削除
	/// </summary>
	void Terminate();

	/// <summary>
	/// ゲームオブジェクトリストに追加
	/// </summary>
	/// <param name="gameobj">追加するゲームオブジェクト</param>
	void AddGameObjectsList(
		GameObject* gameobj
	);

public:
	/// <summary>
	/// クラスの生成
	/// </summary>
	/// <param name="dev">デバイス</param>
	/// <returns>クラスのポインター</returns>
	static GameObjectManager* Craete(
		ID3D12Device* dev
	);

private:
	/// <summary>
	/// リストに入っているゲームオブジェクト同士の当たり判定
	/// </summary>
	void CheckCollision();
};

