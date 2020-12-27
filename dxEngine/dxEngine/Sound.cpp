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

	//サウンドデータ格納配列のクリア
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

	//ファイルオープン
	//ファイル入力ストリームのインスタンス
	std::ifstream file;
	//.wavファイルをバイナリモードで開く
	file.open(
		loadName,
		std::ios_base::binary
	);
	//ファイルオープン失敗を検出する
	if (file.fail()) {
		assert(0);
	}

	//.wavファイルの読み込み処理
	//RIFFヘッダーの読み込み
	RiffHeader riff;
	file.read(
		(char*)&riff,
		sizeof(riff)
	);
	//ファイルがRIFFかチェック
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	//Formatチャンクの読み込み
	loadData.format;
	file.read(
		(char*)&loadData.format,
		sizeof(loadData.format)
	);
	//Dataチャンクの読み込み
	loadData.data;
	file.read(
		(char*)&loadData.data,
		sizeof(loadData.data)
	);
	//Dataチャンクのデータ部(波形データ)の読み込み
	loadData.pBuffer = new char[loadData.data.size];
	file.read(
		loadData.pBuffer,
		loadData.data.size
	);
	//Waveファイルを閉じる
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

	//サウンド再生
	WAVEFORMATEX wformat_ex{};
	//波形フォーマットの設定
	memcpy(
		&wformat_ex,
		&useData.format.fmt,
		sizeof(useData.format.fmt)
	);
	wformat_ex.wBitsPerSample =
		useData.format.fmt.nBlockAlign * 8 / useData.format.fmt.nChannels;
	//波形フォーマットを元にSourceVoiceの生成
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

	//再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = (BYTE*)useData.pBuffer;
	buf.pContext = useData.pBuffer;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	buf.AudioBytes = useData.data.size;
	res = S_FALSE;
	//波形データの再生
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

	//サウンド再生
	WAVEFORMATEX wformat_ex{};
	//波形フォーマットの設定
	memcpy(
		&wformat_ex,
		&useData.format.fmt,
		sizeof(useData.format.fmt)
	);
	wformat_ex.wBitsPerSample =
		useData.format.fmt.nBlockAlign * 8 / useData.format.fmt.nChannels;
	//波形フォーマットを元にSourceVoiceの生成
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

	//再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = (BYTE*)useData.pBuffer;
	buf.pContext = useData.pBuffer;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	buf.AudioBytes = useData.data.size;
	buf.LoopCount = XAUDIO2_LOOP_INFINITE;
	res = S_FALSE;
	//波形データの再生
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