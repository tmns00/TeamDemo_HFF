#include "Sound.h"

#include<assert.h>
#include<fstream>

Sound* Sound::instance = nullptr;

const char* filePath = "Resources/Sound/";

Sound::Sound() {
}

void Sound::Initialize() {
	HRESULT res = S_FALSE;

	res = XAudio2Create(
		&xAudio2,
		0,
		XAUDIO2_DEFAULT_PROCESSOR
	);
	if (FAILED(res)) {
		assert(0);
	}

	res = S_FALSE;
	res = xAudio2->CreateMasteringVoice(&masterVoice);

	//�T�E���h�f�[�^�i�[�z��̃N���A
	soundDatas.clear();

	std::string sound_list[] =
	{
		"Alarm01.wav",
		"Alarm02.wav",
		"Alarm03.wav",
		"Alarm04.wav",
		"Alarm05.wav",
		"button.wav",
		"dmg.wav"
	};

	for (int i = 0; i < _countof(sound_list); ++i) {
		LoadSoundData(sound_list[i]);
	}
}

HRESULT Sound::LoadSoundData(
	const std::string& filename
) {
	HRESULT res = S_FALSE;

	SoundData loadData;

	std::string loadName = filePath + filename;

	//�t�@�C���I�[�v��
	//�t�@�C�����̓X�g���[���̃C���X�^���X
	std::ifstream file;
	//.wav�t�@�C�����o�C�i�����[�h�ŊJ��
	file.open(
		loadName,
		std::ios_base::binary
	);
	//�t�@�C���I�[�v�����s�����o����
	if (file.fail()) {
		assert(0);
	}

	//.wav�t�@�C���̓ǂݍ��ݏ���
	//RIFF�w�b�_�[�̓ǂݍ���
	RiffHeader riff;
	file.read(
		(char*)&riff,
		sizeof(riff)
	);
	//�t�@�C����RIFF���`�F�b�N
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	//Format�`�����N�̓ǂݍ���
	loadData.format;
	file.read(
		(char*)&loadData.format,
		sizeof(loadData.format)
	);
	//Data�`�����N�̓ǂݍ���
	loadData.data;
	file.read(
		(char*)&loadData.data,
		sizeof(loadData.data)
	);
	//Data�`�����N�̃f�[�^��(�g�`�f�[�^)�̓ǂݍ���
	loadData.pBuffer = new char[loadData.data.size];
	file.read(
		loadData.pBuffer,
		loadData.data.size
	);
	//Wave�t�@�C�������
	file.close();

	soundDatas.emplace(filename, loadData);

	return S_OK;
}

void Sound::Create() {
	instance = new Sound;
	if (!instance) {
		assert(0);
	}

	instance->Initialize();
}

Sound* Sound::GetInstance() {
	return instance;
}

void Sound::Terminate(){
	delete instance;
	instance = nullptr;
}

void Sound::PlaySE(
	const std::string& key
) {
	HRESULT res = S_FALSE;

	SoundData useData;
	useData.format = soundDatas.find(key)->second.format;
	useData.data = soundDatas.find(key)->second.data;
	useData.pBuffer = soundDatas.find(key)->second.pBuffer;

	//�T�E���h�Đ�
	WAVEFORMATEX wformat_ex{};
	//�g�`�t�H�[�}�b�g�̐ݒ�
	memcpy(
		&wformat_ex,
		&useData.format.fmt,
		sizeof(useData.format.fmt)
	);
	wformat_ex.wBitsPerSample =
		useData.format.fmt.nBlockAlign * 8 / useData.format.fmt.nChannels;
	//�g�`�t�H�[�}�b�g������SourceVoice�̐���
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	res = xAudio2->CreateSourceVoice(
		&pSourceVoice,
		&wformat_ex,
		0,
		2.0f,
		&voiceCallback
	);
	if (FAILED(res)) {
		delete[] useData.pBuffer;
		return;
	}

	//�Đ�����g�`�f�[�^�̐ݒ�
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = (BYTE*)useData.pBuffer;
	buf.pContext = useData.pBuffer;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	buf.AudioBytes = useData.data.size;
	res = S_FALSE;
	//�g�`�f�[�^�̍Đ�
	res = pSourceVoice->SubmitSourceBuffer(&buf);
	if (FAILED(res)) {
		assert(0);
	}
	res = S_FALSE;
	res = pSourceVoice->Start();
	if (FAILED(res)) {
		assert(0);
	}
}

void Sound::PlayBGM(
	const std::string& key
) {
	HRESULT res = S_FALSE;

	SoundData useData;
	useData.format = soundDatas.find(key)->second.format;
	useData.data = soundDatas.find(key)->second.data;
	useData.pBuffer = soundDatas.find(key)->second.pBuffer;

	//�T�E���h�Đ�
	WAVEFORMATEX wformat_ex{};
	//�g�`�t�H�[�}�b�g�̐ݒ�
	memcpy(
		&wformat_ex,
		&useData.format.fmt,
		sizeof(useData.format.fmt)
	);
	wformat_ex.wBitsPerSample =
		useData.format.fmt.nBlockAlign * 8 / useData.format.fmt.nChannels;
	//�g�`�t�H�[�}�b�g������SourceVoice�̐���
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	res = xAudio2->CreateSourceVoice(
		&pSourceVoice,
		&wformat_ex,
		0,
		2.0f,
		&voiceCallback
	);
	if (FAILED(res)) {
		delete[] useData.pBuffer;
		return;
	}

	//�Đ�����g�`�f�[�^�̐ݒ�
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = (BYTE*)useData.pBuffer;
	buf.pContext = useData.pBuffer;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	buf.AudioBytes = useData.data.size;
	buf.LoopCount = XAUDIO2_LOOP_INFINITE;
	res = S_FALSE;
	//�g�`�f�[�^�̍Đ�
	res = pSourceVoice->SubmitSourceBuffer(&buf);
	if (FAILED(res)) {
		assert(0);
	}
	res = S_FALSE;
	res = pSourceVoice->Start();
	if (FAILED(res)) {
		assert(0);
	}
}