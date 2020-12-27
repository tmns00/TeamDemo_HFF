#include"GameApp.h"
#include"Camera.h"
#include"Collision.h"
#include"VMDLoader.h"

using namespace DirectX;

const int deltaCone_num = 10;
const int sylinder_num = 5;
const int sphere_num = 15;

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

	//PMD���f��
	pmdObj = PMDObject::Create(
		dxSystem->GetDevice(),
		"Resources/Model/�����~�N.pmd"
	);
	//objManager->AddGameObjectsList(pmdObj);

	//PMD���f��
	fbxObj = FBXObject::Create(
		dxSystem->GetDevice(),
		"Resources/Model/fbxcube.fbx"
	);
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
		cube_rot.z = 30.0f;
		cube_rot.x = 160.0f;
		omg = 0;
		g = -0.05f;
		t = 0;
		//radzx = cube->GetRotation().x;
		//radxx = cube->GetRotation().z;
		//radzy = cube->GetRotation().x;
		//radxy = 90 - cube->GetRotation().z;
		col_plane.normal = XMVectorSet(0, 1, 0, 0);
		col_plane.distance = 0.0f;
		//F.x = 0.001;
	}

	return true;
}

void GameApp::Run() {
	float angle = 0;

	MSG msg = {};

	//�}�E�X�E�h���b�O
	bool flag_mouseRDrag = false;
	//�}�E�X�̃X�N���[������W(�O�t���[��)
	POINT pre_mPoint;
	//�}�E�X�̃X�N���[������W(���݃t���[��)
	POINT now_mPoint;
	//�������p�x(radian)
	float camRot_theta = -90.0f * (XM_PI / 180.0f);
	//�c�����p�x(radian)
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
				if (hit)
				{
					isHit = true;
					Grav = false;
				}
			}
			if (input->isTrigger(DIK_F))
				Grav = true;
			if (Grav)
			{
				cube_pos.y -= 1;
			}

			XMFLOAT3 cube_rot = cube->GetRotation();

			if (input->isKey(DIK_UP) || input->isKey(DIK_DOWN)
				|| input->isKey(DIK_RIGHT) || input->isKey(DIK_LEFT))
			{
				// �ړ���̍��W���v�Z
				if (input->isKey(DIK_UP)) { cube_rot.z += 1; }
				else if (input->isKey(DIK_DOWN)) { cube_rot.z -= 1; }
				if (input->isKey(DIK_RIGHT)) { cube_rot.x += 1; }
				else if (input->isKey(DIK_LEFT)) { cube_rot.x -= 1; }

				if (cube_rot.z > 360)
				{
					cube_rot.z = 0;
				}
				if (cube_rot.z < 0)
				{
					cube_rot.z = 360;
				}

				//cube->SetRotation(cube_rot);
			}

			if (isHit)
			{

				if (Fx.x < Fx.y)
				{
					bool isStop = false;

					if (cube_rot.z == 0 || cube_rot.z == 90
						|| cube_rot.z == 180 || cube_rot.z == 270
						|| cube_rot.z == 360)
						isStop = true;
					radxx += 1;
					if (!isStop) {
						cube_rot.z += 1;
						//cube->SetRotation(cube_rot);
					}

				}
				if (Fx.x > Fx.y)
				{
					bool isStop = false;
					if (cube_rot.z == 0 || cube_rot.z == 90
						|| cube_rot.z == 180 || cube_rot.z == 270
						|| cube_rot.z == 360)

						isStop = true;
					radxx -= 1;
					if (!isStop) {
						cube_rot.z -= 1;
						//cube->SetRotation(cube_rot);
					}

				}
				if (Fz.x < Fz.y)
				{
					bool isStop = false;
					if (cube_rot.x == 0 || cube_rot.x == 90
						|| cube_rot.x == 180 || cube_rot.x == 270
						|| cube_rot.x == 360)
						isStop = true;
					radzx += 1;
					if (!isStop) {
						cube_rot.x += 1;
						//cube->SetRotation(cube_rot);
					}
				}
				if (Fz.x > Fz.y)
				{
					bool isStop = false;
					if (cube_rot.x == 0 || cube_rot.x == 90
						|| cube_rot.x == 180 || cube_rot.x == 270
						|| cube_rot.x == 360)
						isStop = true;
					radzx -= 1;
					if (!isStop) {
						cube_rot.x -= 1;
						//cube->SetRotation(cube_rot);
					}
				}

				Fx.x = 10 * cos(radxx * 3.14 / 180);
				Fx.y = 10 * sin(radxx * 3.14 / 180);
				Fz.x = 10 * cos(radzx * 3.14 / 180);
				Fz.y = 10 * sin(radzx * 3.14 / 180);


				if (cube_rot.z == 0 || cube_rot.z == 90
					|| cube_rot.z == 180 || cube_rot.z == 270
					|| cube_rot.z == 360
					&& cube_rot.x == 0 || cube_rot.x == 90
					|| cube_rot.x == 180 || cube_rot.x == 270
					|| cube_rot.x == 360)
					PosStop = true;
				if (!PosStop)
				{
					cube_pos.z -= 0.1f * cos(radzx * 3.14 / 180);
					cube_pos.x -= 0.1f * cos(radxx * 3.14 / 180);
					cube_pos.y -= 0.1f * abs((sin(radxx * 3.14 / 180) - sin(radzx * 3.14 / 180)));
				}
			}
			cube->SetPosition(cube_pos);
			cube->SetRotation(cube_rot);
		}

		//�A�j���[�V����
		{
			if (input->isTrigger(DIK_M))
				pmdObj->PlayAnimation("squat.vmd");

			if (input->isTrigger(DIK_N))
				pmdObj->PlayAnimation("swing2.vmd");

			if (input->isTrigger(DIK_B))
				pmdObj->StopAnimation();
		}

		//�J�����ړ�
		{
			//��
			if (input->isKey(DIK_NUMPAD1) || input->isKey(DIK_NUMPAD3)) {
				if (input->isKey(DIK_NUMPAD1))
					Camera::MoveVector({ -0.1f,0.0f,0.0f });
				else if (input->isKey(DIK_NUMPAD3))
					Camera::MoveVector({ +0.1f,0.0f,0.0f });
			}
			//�c
			if (input->isKey(DIK_NUMPAD5) || input->isKey(DIK_NUMPAD2)) {
				if (input->isKey(DIK_NUMPAD2))
					Camera::MoveVector({ 0.0f,-0.1f,0.0f });
				else if (input->isKey(DIK_NUMPAD5))
					Camera::MoveVector({ 0.0f,+0.1f,0.0f });
			}
			//���s
			if (input->isKey(DIK_NUMPAD4) || input->isKey(DIK_NUMPAD6)) {
				if (input->isKey(DIK_NUMPAD4))
					Camera::MoveVector({ 0.0f,0.0f,-0.1f });
				else if (input->isKey(DIK_NUMPAD6))
					Camera::MoveVector({ 0.0f,0.0f,+0.1f });
			}
		}
		//�J������]
		{
			//�h���b�O�����ǂ���
			if (GetAsyncKeyState(VK_RBUTTON) & 0x8000
				&& !flag_mouseRDrag
				&& winApp->GetWindowActive()) {
				flag_mouseRDrag = true;
				GetCursorPos(&pre_mPoint);
			}
			else if (!(GetAsyncKeyState(VK_RBUTTON) & 0x8000)) {
				flag_mouseRDrag = false;
			}
			//�X�N���[����̈ړ����������]�p�����߂�
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

		//�I�u�W�F�N�g�ړ�
		{
			XMFLOAT3 pos = pmdObj->GetPosition();

			if (input->isKey(DIK_W) || input->isKey(DIK_S)) { //�c
				if (input->isKey(DIK_S)) {
					pos.y -= 0.1f;
				}
				else if (input->isKey(DIK_W)) {
					pos.y += 0.1f;
				}
			}
			if (input->isKey(DIK_A) || input->isKey(DIK_D)) { //��
				if (input->isKey(DIK_A)) {
					pos.x -= 0.1f;
				}
				else if (input->isKey(DIK_D)) {
					pos.x += 0.1f;
				}
			}
			if (input->isKey(DIK_Q) || input->isKey(DIK_E)) { //���s
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

		//�X�V����
		input->Update();
		objManager->Update();

		//�`��O����
		dxSystem->DrawBefore();

		//�`�揈��
		objManager->Draw(dxSystem->GetCmdList());

		//�`��㏈��
		dxSystem->DrawAfter();
	}
}

void GameApp::Delete() {
	objManager->Terminate();
	input->Terminate();
	DirectXSystem::Destroy();
	Camera::Terminate();
	Sound::Terminate();

	//�����N���X�͎g��Ȃ��̂œo�^��������
	winApp->DeleteGameWindow();
	safe_delete(winApp);
}

GameApp::GameApp() {
}

GameApp::~GameApp() {
}
