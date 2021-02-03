#include"GameApp.h"
#include"Camera.h"
#include"Collision.h"
#include"VMDLoader.h"
//�ǉ�
#include<sstream>
#include<iomanip>
using namespace DirectX;

GameApp& GameApp::Instance() {
	static GameApp instance;
	return instance;
}

bool GameApp::Initialize() {
	//�E�B���h�E�A�v���P�[�V��������
	WindowsApp::Create();
	winApp = WindowsApp::GetInstance();
	winApp->CreateGameWindow();

	//DirectX�������֘A
	DirectXSystem::Create();
	dxSystem = DirectXSystem::GetInstance();
	dxSystem->Initialize(winApp);

	//�L�[���̓N���X����
	Input::CreateInstance(winApp->GetHWND());
	input = Input::GetInstance();
	//�J�����N���X����
	Camera::Create();
	camera = Camera::GetInstance();
	//�T�E���h�N���X����
	Sound::Create();
	sound = Sound::GetInstance();
	//VMD���[�h�N���X
	VMDLoader::Create();

	//�Q�[���I�u�W�F�N�g�Ǘ��N���X����
	objManager = GameObjectManager::Craete(dxSystem->GetDevice());

	//������
	cube = Cube::Create(dxSystem->GetDevice());
	cube->SetPosition({ -15,5,0 });
	objManager->AddGameObjectsList(cube);

	plane = Plane::Create(dxSystem->GetDevice());
	plane->SetPosition({ 0,0,0 });
	objManager->AddGameObjectsList(plane);
	XMFLOAT3 plane_scale = { 1000,1000,1000 };
	plane->SetScale(plane_scale);

	//FBX���f��
	fbxObj = new FbxObj2();
	fbxObj->device = dxSystem->GetDevice();
	fbxObj->Initialize();
	objManager->AddGameObjectsList(fbxObj);

	//OBJ���f��
	objObj = OBJObject::Create(
		dxSystem->GetDevice(),
		"skydome"
	);
	objObj->SetPosition({ 15,0,0 });
	objManager->AddGameObjectsList(objObj);

	//����
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
		cube_rot.z = 60.0f;
		cube_rot.x = 290.0f;
		omg = 0;
		g = -0.05f;
		t = 0;
		//rectangle.radius = .0f;
		col_plane.normal = XMVectorSet(0, 1, 0, 0);
		col_plane.distance = 0.0f;
		//F.x = 0.001;
		radxx = 30;
		cube->SetRotation(cube_rot);
	}

	return true;
}

void GameApp::Run() {
	float angle = 0;

	MSG msg = {};

	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//�A�v���P�[�V�������I���Ƃ���message��WM_QUIT�ɂȂ�
		if (msg.message == WM_QUIT)
		{
			break;
		}

		//����
		{
			// ���݂̍��W���擾
			XMFLOAT3 cube_pos = cube->GetPosition();

			for (int i = 0; i < 24; i++)
			{
				XMFLOAT3 p;
				p.x = cube_pos.x + cube->GetVertPos(i).x;
				p.y = cube_pos.y + cube->GetVertPos(i).y;
				p.z = cube_pos.z + cube->GetVertPos(i).z;
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
					INTER.x = cube_pos.x - inter.m128_f32[0];
					INTER.y = cube_pos.y - inter.m128_f32[1];
					INTER.z = cube_pos.z - inter.m128_f32[2];

					inter2.x = inter.m128_f32[0];
					inter2.y = inter.m128_f32[1];
					inter2.z = inter.m128_f32[2];
					position.x = cube_pos.x;
					position.y = cube_pos.y;
					position.z = cube_pos.z;
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
			if (input->IsTrigger(DIK_F))
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

			if (input->IsKey(DIK_UP) || input->IsKey(DIK_DOWN)
				|| input->IsKey(DIK_RIGHT) || input->IsKey(DIK_LEFT))
			{
				// �ړ���̍��W���v�Z
				if (input->IsKey(DIK_UP)) { cube_rot.z += 1; }
				else if (input->IsKey(DIK_DOWN)) { cube_rot.z -= 1; }
				if (input->IsKey(DIK_RIGHT)) { cube_rot.x += 1; }
				else if (input->IsKey(DIK_LEFT)) { cube_rot.x -= 1; }

				//cube->SetRotation(cube_rot);
			}

			if (isHit)
			{

				t += 1 * 3.14 / 180;
				//isStopZ = true;
				//isStopX = true;
				if (cube_rot.z == 0 ||
					cube_rot.z == 90 ||
					cube_rot.z == 180 ||
					cube_rot.z == 270||
					cube_rot.z == 360)
					isStopX = true;
				if (cube_rot.x == 0 ||
					cube_rot.x == 90 ||
					cube_rot.x == 180 ||
					cube_rot.x == 270 ||
					cube_rot.x == 360)
					isStopZ = true;
				if (Fx.x < Fx.y)
				{
					float F = Fx.y - Fx.x;
					VecX += Acc(F, 10.0f);
					omgX = Omg(VecX, 5);

					radxx += omgX;
					if (!isStopX) {
						cube_rot.z += omgX;
						//cube->SetRotation(cube_rot);
					}
					
					if (cube_rot.z > 45 && cube_rot.z < 90)
					{
						if (radxx >= 85)
						{
							cube_rot.z = 90;
							isStopX = true;
						}
					}
					if (cube_rot.z > 135 && cube_rot.z < 180)
					{
						if (radxx >= 85)
						{
							cube_rot.z = 180;
							isStopX = true;
						}
					}
					if (cube_rot.z > 225 && cube_rot.z < 270)
					{
						if (radxx >= 85)
						{
							cube_rot.z = 270;
							isStopX = true;
						}
					}
					if (cube_rot.z > 315 && cube_rot.z < 360)
					{
						if (radxx >= 85)
						{
							cube_rot.z = 360;
							isStopX = true;
						}
					}

				}
				if (Fx.x > Fx.y)
				{
					float F = Fx.x - Fx.y;
					VecX += Acc(F, 10.0f);
					omgX = Omg(VecX, 5);

					radxx -= omgX;
					if (!isStopX) {
						cube_rot.z -= omgX;

					}
					
					if (radxx >= 0 && radxx < 45)
					{
						if (radxx <= 5)
						{
							cube_rot.z = 0;
							isStopX = true;
						}
					}
					if (cube_rot.z >= 90 && cube_rot.z < 135)
					{
						if (radxx <= 5)
						{
							cube_rot.z = 90;
							isStopX = true;
						}
					}
					if (cube_rot.z >= 180 && cube_rot.z < 225)
					{
						if (radxx <= 5)
						{
							cube_rot.z = 180;
							isStopX = true;
						}
					}
					if (cube_rot.z >= 270 && cube_rot.z < 315)
					{
						if (radxx <= 5)
						{
							cube_rot.z = 270;
							isStopX = true;
						}
					}

				}
				if (Fz.x < Fz.y)
				{
					float F = Fz.y - Fz.x;
					VecZ += Acc(F, 10.0f);
					omgZ = Omg(VecZ, 5);

					radzx += omgZ;
					if (!isStopZ) {
						cube_rot.x += omgZ;

					}
					if (cube_rot.x > 45 && cube_rot.x <= 90)
					{
						if (radzx >= 85)
						{
							cube_rot.x = 90;
							isStopZ = true;
						}
					}
					if (cube_rot.x > 135 && cube_rot.x <= 180)
					{
						if (radxx >= 85)
						{
							cube_rot.x = 180;
							isStopZ = true;
						}
					}
					if (cube_rot.x = 225 && cube_rot.x <= 270)
					{
						if (radxx >= 85)
						{
							cube_rot.x = 270;
							isStopZ = true;
						}
					}
					if (cube_rot.x >= 315 && cube_rot.x <= 360)
					{
						if (radxx >= 85)
						{
							cube_rot.x = 360;
							isStopZ = true;
						}
					}
				}
				if (Fz.x > Fz.y)
				{
					float F = Fz.x - Fz.y;
					VecZ += Acc(F, 10.0f);
					omgZ = Omg(VecZ, 5);

					radzx -= omgZ;
					if (!isStopZ) {
						cube_rot.x -= omgZ;
						//cube->SetRotation(cube_rot);
					}
					if (radzx >= 0 && radzx < 45)
					{

						if (radzx <= 5)
						{
							cube_rot.x = 0;
							isStopZ = true;
						}
					}
					if (cube_rot.x >= 90 && cube_rot.x < 135)
					{
						if (radxx <= 5)
						{
							cube_rot.x = 90;
							isStopZ = true;
						}
					}
					if (cube_rot.x >= 180 && cube_rot.x < 225)
					{
						if (radxx <= 5)
						{
							cube_rot.x = 180;
							isStopZ = true;
						}
					}
					if (cube_rot.x >= 270 && cube_rot.x < 315)
					{
						if (radxx <= 5)
						{
							cube_rot.x = 270;
							isStopZ = true;
						}
					}
				}
				if (Fy.x < Fy.y)
				{
					bool isStop = false;
					VecY += Acc(Fy.y, 10.0f);
					omgY = Omg(VecY, 5);
					if (cube_rot.y == 0 || cube_rot.y == 90
						|| cube_rot.y == 180 || cube_rot.y == 270
						|| cube_rot.y == 360)
						isStop = true;

					if (!isStop) {
						rady -= omgY;
					}

				}
				if (Fy.x > Fy.y)
				{
					bool isStop = false;
					VecY += Acc(Fy.x, 10.0f);
					omgY = Omg(VecY, 5);
					if (cube_rot.y == 0 || cube_rot.y == 90
						|| cube_rot.y == 180 || cube_rot.y == 270
						|| cube_rot.y == 360)
						isStop = true;

					if (!isStop) {
						rady -= omgY;
					}

				}
				Fx.x = 1 * cos(radxx * 3.14 / 180);
				Fx.y = 1 * sin(radxx * 3.14 / 180);
				Fz.x = 1 * cos(radzx * 3.14 / 180);
				Fz.y = 1 * sin(radzx * 3.14 / 180);
				Fy.x = 1 * cos(rady * 3.14 / 180);
				Fy.y = 1 * sin(rady * 3.14 / 180);

				if (isStopX && isStopZ)
					PosStop = true;
				if (!PosStop)
				{
					cube_pos.z = rotz(POS.z, INTER.z, radzx * 3.14 / 180);
					cube_pos.x = rotx(POS.x, INTER.x, radxx * -1 * 3.14 / 180);
					cube_pos.y = roty(POS.y, INTER.y, rady * 3.14 / 180);
					/*cube_pos.x = inter2.x;
					cube_pos.y = inter2.y;
					cube_pos.z = inter2.z;*/
				}
				cube->SetPosition(cube_pos);
				cube->SetRotation(cube_rot);
			}


			//�A�j���[�V����
			{
				if (input->IsTrigger(DIK_M))
					pmdObj->PlayAnimation("squat.vmd");

				if (input->IsTrigger(DIK_N))
					pmdObj->PlayAnimation("swing2.vmd");

				if (input->IsTrigger(DIK_B))
					pmdObj->StopAnimation();
			}

			//�I�u�W�F�N�g�ړ�
			{
				XMFLOAT3 pos = fbxObj->GetPosition();

				if (input->IsKey(DIK_W) || input->IsKey(DIK_S)) { //�c
					if (input->IsKey(DIK_S)) {
						cube_pos.z -= 0.1f;
					}
					else if (input->IsKey(DIK_W)) {
						cube_pos.z += 0.1f;
					}
				}
				if (input->IsKey(DIK_A) || input->IsKey(DIK_D)) { //��
					if (input->IsKey(DIK_A)) {
						cube_pos.x -= 0.1f;
					}
					else if (input->IsKey(DIK_D)) {
						cube_pos.x += 0.1f;
					}
				}
				if (input->IsKey(DIK_Q) || input->IsKey(DIK_E)) { //���s
					if (input->IsKey(DIK_Q)) {
						pos.z -= 0.1f;
					}
					else if (input->IsKey(DIK_E)) {
						pos.z += 0.1f;
					}
				}

				fbxObj->SetPosition(pos);
			}

			if (input->IsTrigger(DIK_1)) {
				sound->PlaySE("Alarm01.wav");
			}
			if (input->IsTrigger(DIK_2)) {
				sound->PlaySE("Alarm02.wav");
			}
			if (input->IsTrigger(DIK_3)) {
				sound->PlaySE("Alarm03.wav");
			}
			if (input->IsTrigger(DIK_4)) {
				sound->PlaySE("button.wav");
			}
			if (input->IsTrigger(DIK_5)) {
				sound->PlayBGM("dmg.wav");
			}

			//�X�V����
			input->Update();
			camera->Update();
			objManager->Update();
			cube->SetPosition(cube_pos);
			cube->SetRotation(cube_rot);
			//�`��O����
			dxSystem->DrawBefore();

			//�`�揈��
			objManager->Draw(dxSystem->GetCmdList());

			//�`��㏈��
			dxSystem->DrawAfter();
		}
	}

}
void GameApp::Delete() {
	objManager->Terminate();
	input->Terminate();
	camera->Terminate();
	DirectXSystem::Destroy();
	Sound::Terminate();

	//�����N���X�͎g��Ȃ��̂œo�^��������
	winApp->DeleteGameWindow();
	safe_delete(winApp);
}

GameApp::GameApp() {
}

GameApp::~GameApp() {
}
