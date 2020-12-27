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
	//ボイス処理パスの開始時
	STDMETHOD_(void, OnVoiceProcessingPassStart)(THIS_ UINT32 ByteRequired) {};
	//ボイス処理パスの終了時
	STDMETHOD_(void, OnVoiceProcessingPassEnd)(THIS) {};
	//バッファストリームの再生が終了したとき
	STDMETHOD_(void, OnStreamEnd)(THIS) {};
	//バッファの使用開始時
	STDMETHOD_(void, OnBufferStart)(THIS_ void* pBufferContext) {};
	//バッファの末尾に達したとき
	STDMETHOD_(void, OnBufferEnd)(THIS_ void* pBufferContext) {
		//バッファを開放する
		//delete[] pBufferContext;
	};
	//再生がループ位置に達したとき
	STDMETHOD_(void, OnLoopEnd)(THIS_ void* pBufferContext) {};
	//ボイスの実行エラー時
	STDMETHOD_(void, OnVoiceError)(THIS_ void* pBufferContext, HRESULT Error) {};
};

class Sound
{
private:
	template<class T>using ComPtr = Microsoft::WRL::ComPtr<T>;

private:
	Sound();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// データのロード
	/// </summary>
	/// <param name="filename"></param>
	HRESULT LoadSoundData(
		const std::string& filename
	);

	//コピー・代入禁止
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
	//チャンクヘッダ
	struct Chunk
	{
		char id[4];   //チャンク毎のID
		int32_t size; //チャンクサイズ
	};

	//RIFFヘッダチャンク
	struct RiffHeader
	{
		Chunk chunk;  //"RIFF"
		char type[4]; //"WAVE"
	};

	//FMTチャンク
	struct FormatChunk
	{
		Chunk chunk;    //"fmt"
		WAVEFORMAT fmt; //波形フォーマット
	};

	//サウンドデータ
	struct SoundData
	{
		FormatChunk format; //FMTチャンク
		Chunk data;                        //Dataチャンク
		char* pBuffer;                     //Dataチャンクの波形データ
	};

private:
	//インスタンス
	static Sound* instance;

private:
	ComPtr<IXAudio2> xAudio2 = nullptr;
	IXAudio2MasteringVoice* masterVoice = nullptr;
	//コールバッククラス
	XAudio2VoiceCallback voiceCallback;

	//ロード済み.wavデータ格納配列
	std::map<std::string, SoundData> soundDatas{};
};
