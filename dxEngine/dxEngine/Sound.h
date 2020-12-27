#pragma once

#include<xaudio2.h>
#include<wrl.h>
#include<cstdint>
#include<map>
#include<string>

#pragma comment(lib,"xaudio2.lib")

class XAudio2VoiceCallback :public IXAudio2VoiceCallback
{
public:
	//�{�C�X�����p�X�̊J�n��
	STDMETHOD_(void, OnVoiceProcessingPassStart)(THIS_ UINT32 ByteRequired) {};
	//�{�C�X�����p�X�̏I����
	STDMETHOD_(void, OnVoiceProcessingPassEnd)(THIS) {};
	//�o�b�t�@�X�g���[���̍Đ����I�������Ƃ�
	STDMETHOD_(void, OnStreamEnd)(THIS) {};
	//�o�b�t�@�̎g�p�J�n��
	STDMETHOD_(void, OnBufferStart)(THIS_ void* pBufferContext) {};
	//�o�b�t�@�̖����ɒB�����Ƃ�
	STDMETHOD_(void, OnBufferEnd)(THIS_ void* pBufferContext) {
		//�o�b�t�@���J������
		//delete[] pBufferContext;
	};
	//�Đ������[�v�ʒu�ɒB�����Ƃ�
	STDMETHOD_(void, OnLoopEnd)(THIS_ void* pBufferContext) {};
	//�{�C�X�̎��s�G���[��
	STDMETHOD_(void, OnVoiceError)(THIS_ void* pBufferContext, HRESULT Error) {};
};

class Sound
{
private:
	template<class T>using ComPtr = Microsoft::WRL::ComPtr<T>;

private:
	Sound();

	/// <summary>
	/// ������
	/// </summary>
	void Initialize();

	/// <summary>
	/// �f�[�^�̃��[�h
	/// </summary>
	/// <param name="filename"></param>
	HRESULT LoadSoundData(
		const std::string& filename
	);

	//�R�s�[�E����֎~
	Sound(const Sound&) = delete;
	void operator=(const Sound&) = delete;

public:
	static void Create();

	static Sound* GetInstance();

	static void Terminate();

public:
	void PlaySE(
		const std::string& key
	);

	void PlayBGM(
		const std::string& key
	);

public:
	//�`�����N�w�b�_
	struct Chunk
	{
		char id[4];   //�`�����N����ID
		int32_t size; //�`�����N�T�C�Y
	};

	//RIFF�w�b�_�`�����N
	struct RiffHeader
	{
		Chunk chunk;  //"RIFF"
		char type[4]; //"WAVE"
	};

	//FMT�`�����N
	struct FormatChunk
	{
		Chunk chunk;    //"fmt"
		WAVEFORMAT fmt; //�g�`�t�H�[�}�b�g
	};

	//�T�E���h�f�[�^
	struct SoundData
	{
		FormatChunk format; //FMT�`�����N
		Chunk data;                        //Data�`�����N
		char* pBuffer;                     //Data�`�����N�̔g�`�f�[�^
	};

private:
	//�C���X�^���X
	static Sound* instance;

private:
	ComPtr<IXAudio2> xAudio2 = nullptr;
	IXAudio2MasteringVoice* masterVoice = nullptr;
	//�R�[���o�b�N�N���X
	XAudio2VoiceCallback voiceCallback;

	//���[�h�ς�.wav�f�[�^�i�[�z��
	std::map<std::string, SoundData> soundDatas{};
};
