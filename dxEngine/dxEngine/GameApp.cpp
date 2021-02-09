#include"GameApp.h"
#include"Camera.h"
#include"Collision.h"
#include"VMDLoader.h"
//追加
#include<sstream>
#include<iomanip>
using namespace DirectX;

const int deltaCone_num = 10;
const int sylinder_num = 5;
const int sphere_num = 15;

GameApp& GameApp::Instance() {
	static GameApp instance;
	return instance;
}

bool GameApp::Initialize() {
	//ウィンドウアプリケーション生成
	WindowsApp::Create();
	winApp = WindowsApp::GetInstance();
	winApp->CreateGameWindow();

	//DirectX初期化関連
	DirectXSystem::Create();
	dxSystem = DirectXSystem::GetInstance();
	dxSystem->Initialize(winApp);

	//キー入力クラス生成
	Input::CreateInstance(winApp->GetHWND());
	input = Input::GetInstance();
	//カメラクラス生成
	Camera::Create();
	//サウンドクラス生成
	Sound::Create();
	sound = Sound::GetInstance();
	//VMDロードクラス
	VMDLoader::Create();

	//ゲームオブジェクト管理クラス生成
	objManager = GameObjectManager::Craete(dxSystem->GetDevice());

	//立方体
	cube = Cube::Create(dxSystem->GetDevice());
	cube->SetPosition({ -15,5,0 });
	objManager->AddGameObjectsList(cube);

	plane = Plane::Create(dxSystem->GetDevice());
	plane->SetPosition({ 0,0,0 });
	objManager->AddGameObjectsList(plane);
	XMFLOAT3 plane_scale = { 1000,1000,1000 };
	plane->SetScale(plane_scale);

	//PMDモデル
	pmdObj = PMDObject::Create(
		dxSystem->GetDevice(),
		"Resources/Model/初音ミク.pmd"
	);
	//objManager->AddGameObjectsList(pmdObj);

	//PMDモデル
	fbxObj = FBXObject::Create(
		dxSystem->GetDevice(),
		"Resources/Model/fbxcube.fbx"
	);
	objManager->AddGameObjectsList(fbxObj);

	//OBJモデル
	objObj = OBJObject::Create(
		dxSystem->GetDevice(),
		"skydome"
	);
	objObj->SetPosition({ 15,0,0 });
	objManager->AddGameObjectsList(objObj);

	//物理
	{
		t = 0.0f;
		isJamp = false;
		vec = { 0,0,0 };
		a = { 0,0,0 };
		vec2 = { 0,0,0 };
		a2 = { 0,0,0 };
		vec3 = { 0,0,0 };
		a3 = { 0,0,0 };
		r = { 1,1,-1 };
		cube_rot.z = 50.0f;
		cube_rot.x = 60.0f;
		omg = 0;
		g = -0.05f;
		t = 0;
		rectangle.radius = 0.2f;
		col_plane.normal = XMVectorSet(0, 1, 0, 0);
		col_plane.distance = 0.0f;
		//F.x = 0.001;
		radxx = 30;
		cube->SetRotation(cube_rot);

		for (int i = 0; i < 24; i++)
		{
			bool isGet = true;
			for (int f = 0; f < 8; f++)
			{
				if (v[f].x == cube->GetVertPos(i).x)
				{
					if (v[f].y == cube->GetVertPos(i).y)
					{
						if (v[f].z == cube->GetVertPos(i).z)
						{
							isGet = false;
						}
					}
				}
			}
			if (isGet)
			{
				v[count] = cube->GetVertPos(i);
				count++;
			}
		}
		XMFLOAT3 cube_pos = cube->GetPosition();
		Len.x = v[0].x - cube_pos.x;
		Len.y = v[0].y - cube_pos.y;
		Len.z = v[0].z - cube_pos.z;
		R1 = sqrt(v[0].x * v[0].x + v[0].y * v[0].y);
		for (int i = 0; i < 4; i++)
		{
			v[cnt1[i]].x = R1 * cos((cube_rot.z + 45 + 90 * i) * 3.14 / 180);
			v[cnt1[i]].y = R1 * sin((cube_rot.z + 45 + 90 * i) * 3.14 / 180);
			v[cnt2[i]].x = R1 * cos((cube_rot.z + 45 + 90 * i) * 3.14 / 180);
			v[cnt2[i]].y = R1 * sin((cube_rot.z + 45 + 90 * i) * 3.14 / 180);
		}
		R2 = sqrt(v[0].z * v[0].z + v[0].y * v[0].y);
	
		Theat = acos(v[0].z / R2)*180/3.14;
		for (int i = 0; i < 4; i++)
		{
			v[cnt1[i]].z = R2 * cos((cube_pos.x + Theat + 90 * i) * 3.14 / 180);
			v[cnt1[i]].y = R2 * sin((cube_pos.x + Theat + 90 * i) * 3.14 / 180);
			v[cnt2[i]].z = R2 * cos((cube_pos.x + Theat + 90 * i) * 3.14 / 180);
			v[cnt2[i]].y = R2 * sin((cube_pos.x + Theat + 90 * i) * 3.14 / 180);
			std::ostringstream oss;

			oss << R2 << ","
				<< v[0].z << ","
				<<Theat<<","
				<<v[0].y;
			oss << std::endl;


			OutputDebugStringA(oss.str().c_str());
		}

	}
	return true;
}

void GameApp::Run() {
	float angle = 0;

	MSG msg = {};

	//マウス右ドラッグ
	bool flag_mouseRDrag = false;
	//マウスのスクリーン上座標(前フレーム)
	POINT pre_mPoint;
	//マウスのスクリーン上座標(現在フレーム)
	POINT now_mPoint;
	//横方向角度(radian)
	float camRot_theta = -90.0f * (XM_PI / 180.0f);
	//縦方向角度(radian)
	float camRot_delta = 0.0f * (XM_PI / 180.0f);

	XMFLOAT3 scale = { 0.5f,0.5f,0.5f };
	pmdObj->SetScale(scale);

	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//アプリケーションが終わるときにmessageがWM_QUITになる
		if (msg.message == WM_QUIT)
		{
			break;
		}

		//物理
		{

			// 現在の座標を取得
			XMFLOAT3 cube_pos = cube->GetPosition();



			

			for (int i = 0; i < 8; i++)
			{
				XMFLOAT3 p;
				p.x = cube_pos.x + v[i].x;
				p.y = cube_pos.y + v[i].y;
				p.z = cube_pos.z + v[i].z;
				


				
				rectangle.center.m128_f32[0] = p.x;
				rectangle.center.m128_f32[1] = p.y;
				rectangle.center.m128_f32[2] = p.z;
				XMVECTOR inter;
				hit = Collision::CheckSphere2Plane(rectangle, col_plane, &inter);
				/*if (0 > cube_pos.y + cube->GetVertPos(i).y)
				{
					PosStop = true;
					isStopX = true;
					isStopZ = true;

				}*/
				if (hit && GetPos)
				{
					POS = cube_pos;
					INTER.x = cube_pos.x - v[i].x;
					INTER.y = cube_pos.y - v[i].y;
					INTER.z = cube_pos.z - v[i].z;
					count = i;
					inter2.x = inter.m128_f32[0];
					inter2.y = inter.m128_f32[1];
					inter2.z = inter.m128_f32[2];
					position.x = cube_pos.x;
					position.y = cube_pos.y;
					position.z = cube_pos.z;

					theat = acos(theta(INTER));
					len = sqrt(position.x * position.x + position.y * position.y + position.z * position.z);

					phai = acos(Phi(INTER));

					rady = acos(Rad(position, inter2)) * 180 / 3.14;
					radxx = cube_rot.z;
					if (cube_rot.z >= 90 && cube_rot.z < 180)
						radxx -= 90;
					if (cube_rot.z >= 180 && cube_rot.z < 270)
						radxx -= 180;
					if (cube_rot.z >= 270 && cube_rot.z < 360)
						radxx -= 270;

					radzx = cube_rot.x;
					if (cube_rot.x >= 90 && cube_rot.x < 180)
						radzx -= 90;
					if (cube_rot.x >= 180 && cube_rot.x < 270)
						radzx -= 180;
					if (cube_rot.x >= 270 && cube_rot.x < 360)
						radzx -= 270;
					GetPos = false;

				}
				if (hit)
				{
					isHit = true;
					Grav = false;


				}
			}
		

		

			std::ostringstream oss;
			
			oss << theat << ","
				<< position.y << ","
				<<sqrt(9)
			 << std::endl;
			OutputDebugStringA(oss.str().c_str());
			if (input->isTrigger(DIK_F))
				Grav = true;
			if (Grav)
			{
				g += -0.01f;
				cube_pos.y += g;
			}
			if (cube_rot.z < 0)
				cube_rot.z = 360;
			if (cube_rot.z > 360)
				cube_rot.z = 0;

			if (cube_rot.x < 0)
				cube_rot.x = 360;
			if (cube_rot.x > 360)
				cube_rot.x = 0;
			XMFLOAT3 cube_rot = cube->GetRotation();

			if (input->isKey(DIK_UP) || input->isKey(DIK_DOWN)
				|| input->isKey(DIK_RIGHT) || input->isKey(DIK_LEFT))
			{
				// 移動後の座標を計算
				if (input->isKey(DIK_UP)) { cube_rot.z += 1; }
				else if (input->isKey(DIK_DOWN)) { cube_rot.z -= 1; }
				if (input->isKey(DIK_RIGHT)) { cube_rot.x += 1; }
				else if (input->isKey(DIK_LEFT)) { cube_rot.x -= 1; }

				//cube->SetRotation(cube_rot);
			}

			if (isHit && input->isKey(DIK_L))
			{
				float F = cos(phai);
				phai += 0.01f;
				cube_pos.y = 10+len * sin(phai);
				cube_pos.x =10+ len * cos(phai);
				float Len2 = sqrt(cube_pos.x * cube_pos.x);
			
				cube_pos.x =10+ Len2 * cos(theat);
				cube_pos.z =10+ Len2 * sin(theat);
				if (cos(phai) < 0 || cos(phai) > 0)
				{
					theat *= 180 * 3.14 / 180;
				}
				//t += 1 * 3.14 / 180;
				////isStopZ = true;
				////isStopX = true;

				//Fx.x = 1 * cos(radxx * 3.14 / 180);
				//Fx.y = 1 * sin(radxx * 3.14 / 180);
				//Fz.x = 1 * cos(radzx * 3.14 / 180);
				//Fz.y = 1 * sin(radzx * 3.14 / 180);
				//Fy.x = 1 * cos(rady * 3.14 / 180);
				//Fy.y = 1 * sin(rady * 3.14 / 180);

				//if (cube_rot.z == 0 ||
				//	cube_rot.z == 90 ||
				//	cube_rot.z == 180 ||
				//	cube_rot.z == 270 ||
				//	cube_rot.z == 360)
				//	isStopX = true;
				//if (cube_rot.x == 0 ||
				//	cube_rot.x == 90 ||
				//	cube_rot.x == 180 ||
				//	cube_rot.x == 270 ||
				//	cube_rot.x == 360)
				//	isStopZ = true;
				//if (Fx.x < Fx.y)
				//{
				//	float F = Fx.y - Fx.x;
				//	VecX += Acc(F, 10.0f);
				//	omgX = Omg(VecX, 5);

				//	radxx += omgX;
				//	if (!isStopX) {
				//		cube_rot.z += omgX;
				//		//cube->SetRotation(cube_rot);
				//	}

				//	/*	if (cube_rot.z > 45 && cube_rot.z < 90)
				//		{
				//			if (radxx >= 80)
				//			{
				//				cube_rot.z = 90;
				//				isStopX = true;
				//			}
				//		}
				//		if (cube_rot.z > 135 && cube_rot.z < 180)
				//		{
				//			if (radxx >= 80)
				//			{
				//				cube_rot.z = 180;
				//				isStopX = true;
				//			}
				//		}
				//		if (cube_rot.z > 225 && cube_rot.z < 270)
				//		{
				//			if (radxx >= 80)
				//			{
				//				cube_rot.z = 270;
				//				isStopX = true;
				//			}
				//		}
				//		if (cube_rot.z > 315 && cube_rot.z < 360)
				//		{
				//			if (radxx >= 80)
				//			{
				//				cube_rot.z = 360;
				//				isStopX = true;
				//			}
				//		}*/

				//}
				//if (Fx.x > Fx.y)
				//{
				//	float F = Fx.x - Fx.y;
				//	VecX += Acc(F, 10.0f);
				//	omgX = Omg(VecX, 5);

				//	radxx -= omgX;
				//	if (!isStopX) {
				//		cube_rot.z -= omgX;

				//	}

				//	/*		if (cube_rot.z > 0 && cube_rot.z < 45)
				//			{
				//				if (radxx <= 10)
				//				{
				//					cube_rot.z = 0;
				//					isStopX = true;
				//				}
				//			}
				//			if (cube_rot.z > 90 && cube_rot.z < 135)
				//			{
				//				if (radxx <= 10)
				//				{
				//					cube_rot.z = 90;
				//					isStopX = true;
				//				}
				//			}
				//			if (cube_rot.z > 180 && cube_rot.z < 225)
				//			{
				//				if (radxx <= 10)
				//				{
				//					cube_rot.z = 180;
				//					isStopX = true;
				//				}
				//			}
				//			if (cube_rot.z > 270 && cube_rot.z < 315)
				//			{
				//				if (radxx <= 10)
				//				{
				//					cube_rot.z = 270;
				//					isStopX = true;
				//				}
				//			}*/

				//}
				//if (Fz.x < Fz.y)
				//{
				//	float F = Fz.y - Fz.x;
				//	VecZ += Acc(F, 10.0f);
				//	omgZ = Omg(VecZ, 5);

				//	radzx += omgZ;
				//	if (!isStopZ) {
				//		cube_rot.x += omgZ;

				//	}
				//	/*	if (cube_rot.x > 45 && cube_rot.x < 90)
				//		{
				//			if (radzx >= 80)
				//			{
				//				cube_rot.x = 90;
				//				isStopZ = true;
				//			}
				//		}
				//		if (cube_rot.x > 135 && cube_rot.x < 180)
				//		{
				//			if (radzx >= 80)
				//			{
				//				cube_rot.x = 180;
				//				isStopZ = true;
				//			}
				//		}
				//		if (cube_rot.x > 225 && cube_rot.x < 270)
				//		{
				//			if (radzx >= 80)
				//			{
				//				cube_rot.x = 270;
				//				isStopZ = true;
				//			}
				//		}
				//		if (cube_rot.x >= 315 && cube_rot.x < 360)
				//		{
				//			if (radzx >= 80)
				//			{
				//				cube_rot.x = 360;
				//				isStopZ = true;
				//			}
				//		}*/
				//}
				//if (Fz.x > Fz.y)
				//{
				//	float F = Fz.x - Fz.y;
				//	VecZ += Acc(F, 10.0f);
				//	omgZ = Omg(VecZ, 5);

				//	radzx -= omgZ;
				//	if (!isStopZ) {
				//		cube_rot.x -= omgZ;
				//		//cube->SetRotation(cube_rot);
				//	}
				//	/*	if (radzx > 0 && radzx < 45)
				//		{

				//			if (radzx <= 10)
				//			{
				//				cube_rot.x = 0;
				//				isStopZ = true;
				//			}
				//		}
				//		if (cube_rot.x > 90 && cube_rot.x < 135)
				//		{
				//			if (radzx <= 10)
				//			{
				//				cube_rot.x = 90;
				//				isStopZ = true;
				//			}
				//		}
				//		if (cube_rot.x > 180 && cube_rot.x < 225)
				//		{
				//			if (radzx <= 10)
				//			{
				//				cube_rot.x = 180;
				//				isStopZ = true;
				//			}
				//		}
				//		if (cube_rot.x > 270 && cube_rot.x < 315)
				//		{
				//			if (radzx <= 10)
				//			{
				//				cube_rot.x = 270;
				//				isStopZ = true;
				//			}
				//		}*/
				//}
				//if (Fy.x < Fy.y)
				//{
				//	bool isStop = false;
				//	VecY += Acc(Fy.y, 10.0f);
				//	omgY = Omg(VecY, 5);
				//	if (cube_rot.y == 0 || cube_rot.y == 90
				//		|| cube_rot.y == 180 || cube_rot.y == 270
				//		|| cube_rot.y == 360)
				//		isStop = true;

				//	if (!isStop) {
				//		rady -= omgY;
				//	}

				//}
				//if (Fy.x > Fy.y)
				//{
				//	bool isStop = false;
				//	VecY += Acc(Fy.x, 10.0f);
				//	omgY = Omg(VecY, 5);
				//	if (cube_rot.y == 0 || cube_rot.y == 90
				//		|| cube_rot.y == 180 || cube_rot.y == 270
				//		|| cube_rot.y == 360)
				//		isStop = true;

				//	if (!isStop) {
				//		rady -= omgY;
				//	}

				//}


				//if (isStopX && isStopZ)
				//	PosStop = true;
				//if (!PosStop)
				//{
				//	cube_pos.z = rotz(POS.z, INTER.z, radzx * 3.14 / 180);
				//	cube_pos.x = rotx(POS.x, INTER.x, radxx * -1 * 3.14 / 180);
				//	cube_pos.y = roty(POS.y, INTER.y, rady * 3.14 / 180);
				//	/*cube_pos.x = inter2.x;
				//	cube_pos.y = inter2.y;
				//	cube_pos.z = inter2.z;*/
				//}

				cube->SetPosition(cube_pos);
				cube->SetRotation(cube_rot);
			}


			//アニメーション
			{
				if (input->isTrigger(DIK_M))
					pmdObj->PlayAnimation("squat.vmd");

				if (input->isTrigger(DIK_N))
					pmdObj->PlayAnimation("swing2.vmd");

				if (input->isTrigger(DIK_B))
					pmdObj->StopAnimation();
			}

			//カメラ移動
			{
				//横
				if (input->isKey(DIK_NUMPAD1) || input->isKey(DIK_NUMPAD3)) {
					if (input->isKey(DIK_NUMPAD1))
						Camera::MoveVector({ -0.1f,0.0f,0.0f });
					else if (input->isKey(DIK_NUMPAD3))
						Camera::MoveVector({ +0.1f,0.0f,0.0f });
				}
				//縦
				if (input->isKey(DIK_NUMPAD5) || input->isKey(DIK_NUMPAD2)) {
					if (input->isKey(DIK_NUMPAD2))
						Camera::MoveVector({ 0.0f,-0.1f,0.0f });
					else if (input->isKey(DIK_NUMPAD5))
						Camera::MoveVector({ 0.0f,+0.1f,0.0f });
				}
				//奥行
				if (input->isKey(DIK_NUMPAD4) || input->isKey(DIK_NUMPAD6)) {
					if (input->isKey(DIK_NUMPAD4))
						Camera::MoveVector({ 0.0f,0.0f,-0.1f });
					else if (input->isKey(DIK_NUMPAD6))
						Camera::MoveVector({ 0.0f,0.0f,+0.1f });
				}
			}
			//カメラ回転
			{
				//ドラッグ中かどうか
				if (GetAsyncKeyState(VK_RBUTTON) & 0x8000
					&& !flag_mouseRDrag
					&& winApp->GetWindowActive()) {
					flag_mouseRDrag = true;
					GetCursorPos(&pre_mPoint);
				}
				else if (!(GetAsyncKeyState(VK_RBUTTON) & 0x8000)) {
					flag_mouseRDrag = false;
				}
				//スクリーン上の移動距離から回転角を決める
				if (flag_mouseRDrag) {
					GetCursorPos(&now_mPoint);

					camRot_theta -= (now_mPoint.x - pre_mPoint.x) * 0.001f;

					if (camRot_delta + (now_mPoint.y - pre_mPoint.y) * 0.001f >=
						XM_PI / 2.0f - 0.0001f) {
						camRot_delta = XM_PI / 2.0f - 0.0001f;
					}
					else if (camRot_delta + (now_mPoint.y - pre_mPoint.y) * 0.001f <=
						-XM_PI / 2.0f + 0.0001f) {
						camRot_delta = -XM_PI / 2.0f + 0.0001f;
					}
					else {
						camRot_delta += (now_mPoint.y - pre_mPoint.y) * 0.001f;
					}

					GetCursorPos(&pre_mPoint);

					Camera::RotationCamForMouse(
						camRot_theta,
						camRot_delta
					);
				}
			}

			//オブジェクト移動
			{
				XMFLOAT3 pos = pmdObj->GetPosition();

				if (input->isKey(DIK_W) || input->isKey(DIK_S)) { //縦
					if (input->isKey(DIK_S)) {
						cube_pos.z -= 0.1f;
					}
					else if (input->isKey(DIK_W)) {
						cube_pos.z += 0.1f;
					}
				}
				if (input->isKey(DIK_A) || input->isKey(DIK_D)) { //横
					if (input->isKey(DIK_A)) {
						cube_pos.x -= 0.1f;
					}
					else if (input->isKey(DIK_D)) {
						cube_pos.x += 0.1f;
					}
				}
				if (input->isKey(DIK_Q) || input->isKey(DIK_E)) { //奥行
					if (input->isKey(DIK_Q)) {
						pos.z -= 0.1f;
					}
					else if (input->isKey(DIK_E)) {
						pos.z += 0.1f;
					}
				}

				pmdObj->SetPosition(pos);
			}

			if (input->isTrigger(DIK_1)) {
				sound->PlaySE("Alarm01.wav");
			}
			if (input->isTrigger(DIK_2)) {
				sound->PlaySE("Alarm02.wav");
			}
			if (input->isTrigger(DIK_3)) {
				sound->PlaySE("Alarm03.wav");
			}
			if (input->isTrigger(DIK_4)) {
				sound->PlaySE("button.wav");
			}
			if (input->isTrigger(DIK_5)) {
				sound->PlayBGM("dmg.wav");
			}

			//更新処理
			input->Update();
			objManager->Update();
			cube->SetPosition(cube_pos);
			cube->SetRotation(cube_rot);
			//描画前処理
			dxSystem->DrawBefore();

			//描画処理
			objManager->Draw(dxSystem->GetCmdList());

			//描画後処理
			dxSystem->DrawAfter();
		}
	}

}
void GameApp::Delete() {
	objManager->Terminate();
	input->Terminate();
	DirectXSystem::Destroy();
	Camera::Terminate();
	Sound::Terminate();

	//もうクラスは使わないので登録解除する
	winApp->DeleteGameWindow();
	safe_delete(winApp);
}

GameApp::GameApp() {
}

GameApp::~GameApp() {
}
