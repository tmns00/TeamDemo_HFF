#include "VMDLoader.h"

#include<assert.h>

VMDLoader* VMDLoader::instance = nullptr;

std::string directoryPath = "Resources/Model/";

VMDLoader::~VMDLoader(){
}

MotionDatas VMDLoader::GetMotion(
    std::string key
){
    return motionDatasList[key];
}

void VMDLoader::Create(){
    instance = new VMDLoader;

    assert(instance);

    std::string list[] =
    {
        "squat.vmd",
        "swing2.vmd"
    };

    for (int i = 0; i < _countof(list); ++i) {
        instance->LoadVMDFile(list[i]);
    }
}

VMDLoader* VMDLoader::GetInstance(){
    return instance;
}

void VMDLoader::LoadVMDFile(
    std::string path
){
    //追加するデータ
    MotionDatas addData;
    addData.duration = 0;

    //ロードするファイルまでの階層の合成
    std::string loadPath = directoryPath + path;

    //指定ファイルのオープン
    auto fp = fopen(loadPath.c_str(), "rb");

    fseek(fp, 50, SEEK_SET); //最初の50バイトは飛ばす

    //モーションデータの数
    unsigned int motionDataNum = 0;
    fread(&motionDataNum, sizeof(motionDataNum), 1, fp);

    std::vector<VMDMotionData> vmdMotionData(motionDataNum);
    for (auto& motion : vmdMotionData) {
        fread(
            motion.boneName,
            sizeof(motion.boneName),
            1, fp
        );
        fread(
            &motion.frameNo,
            sizeof(motion.frameNo)
            + sizeof(motion.location)
            + sizeof(motion.quaternion)
            + sizeof(motion.bezier),
            1,
            fp
        );

        addData.duration = std::max<unsigned int>(
            addData.duration,
            motion.frameNo
            );
    }

    //表情データ読み込み
    uint32_t morphCount = 0;
    fread(
        &morphCount,
        sizeof(morphCount),
        1,
        fp
    );
    std::vector<VMDMorph> morphs(morphCount);
    fread(
        morphs.data(),
        sizeof(VMDMorph),
        morphCount,
        fp
    );

    //カメラ読み込み
    uint32_t vmdCameraCount = 0;
    fread(
        &vmdCameraCount,
        sizeof(vmdCameraCount),
        1,
        fp
    );
    std::vector<VMDCamera>cameraData(vmdCameraCount);
    fread(
        cameraData.data(),
        sizeof(VMDCamera),
        vmdCameraCount,
        fp
    );

    uint32_t vmdLightCount = 0;
    fread(
        &vmdLightCount,
        sizeof(vmdLightCount),
        1,
        fp
    );
    std::vector<VMDLight> lights(vmdLightCount);
    fread(
        lights.data(),
        sizeof(VMDLight),
        vmdLightCount,
        fp
    );

    uint32_t selfShadowCount = 0;
    fread(
        &selfShadowCount,
        sizeof(selfShadowCount),
        1,
        fp
    );
    std::vector<VMDSelfShadow> selfShadowData(selfShadowCount);
    fread(
        selfShadowData.data(),
        sizeof(VMDSelfShadow),
        selfShadowCount,
        fp
    );

    //IKオンオフの切り替わり数
    uint32_t ikSwitchCount = 0;
    fread(
        &ikSwitchCount,
        sizeof(ikSwitchCount),
        1,
        fp
    );

    //IKオンオフデータの読み込み
    addData.ikEnableData.resize(ikSwitchCount);
    for (auto& ikEnable : addData.ikEnableData) {
        //キーフレーム情報なのでまずはフレーム番号読み込み
        fread(
            &ikEnable.frameNo,
            sizeof(ikEnable.frameNo),
            1,
            fp
        );

        //次の可視フラグは使用しない
        uint8_t visibleFlag = 0;
        fread(
            &visibleFlag,
            sizeof(visibleFlag),
            1,
            fp
        );

        //対象ボーン数読み込み
        uint32_t ikBoneCount = 0;
        fread(
            &ikBoneCount,
            sizeof(ikBoneCount),
            1,
            fp
        );

        //ループしつつ名前とオンオフ情報を取得
        for (int i = 0; i < ikBoneCount; ++i) {
            char ikBoneName[20];
            fread(
                ikBoneName,
                _countof(ikBoneName),
                1,
                fp
            );

            uint8_t flag = 0;
            fread(
                &flag,
                sizeof(flag),
                1,
                fp
            );

            ikEnable.ikEnableTable[ikBoneName] = flag;
        }
    }

    fclose(fp);

    //VMDのモーションデータから、実際に使うモーションテーブルに変換
    Motion motion{};
    for (auto& vmdMotion : vmdMotionData) {
        motion.frameNo = vmdMotion.frameNo;
        motion.quaternion = XMLoadFloat4(&vmdMotion.quaternion);
        motion.offset = vmdMotion.location;
        motion.p1 = XMFLOAT2(
            (float)vmdMotion.bezier[3] / 127.0f,
            (float)vmdMotion.bezier[7] / 127.0f
        );
        motion.p2 = XMFLOAT2(
            (float)vmdMotion.bezier[11] / 127.0f,
            (float)vmdMotion.bezier[15] / 127.0f
        );
        addData.motion[vmdMotion.boneName].emplace_back(
            motion
        );
    }

    motionDatasList.emplace(path, addData);
}
