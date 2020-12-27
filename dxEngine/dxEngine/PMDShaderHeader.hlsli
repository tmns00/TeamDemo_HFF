Texture2D<float4> tex : register(t0); //0�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��
Texture2D<float4> sphere : register(t1); //�X�t�B�A�}�b�v 1�ԃX���b�g
Texture2D<float4> addSph : register(t2); //���Z�X�t�B�A�}�b�v 2�ԃX���b�g
SamplerState smp : register(s0); //0�ԃX���b�g�ɐݒ肳�ꂽ�T���v���[
//�萔�o�b�t�@
cbuffer cbuff0 : register(b0)
{
	matrix world;    //���[���h�ϊ��s��
	matrix view;     //�r���[�s��
	matrix proj;     //�v���W�F�N�V�����s��
	matrix viewproj; //�r���[�E�v���W�F�N�V�����s��
	float3 eye;      //���_
	matrix bones[256]; //�{�[���s��
};
//�萔�o�b�t�@ �}�e���A���p
cbuffer Material : register(b1)
{
	float4 diffuse;  //�f�B�t���[�Y
	float4 specular; //�X�y�L�����[
	float3 ambient;  //�A���r�G���g
}

//���_�V�F�[�_�[����s�N�Z���V�F�[�_�[�ւ̂����Ɏg�p����\����
struct Output
{
	float4 svpos : SV_POSITION; //�V�X�e���p���_���W
	float4 pos : POSITION;      //���_���W
	float4 normal : NORMAL0;    //�@���x�N�g��
	float4 vnormal : NORMAL1;   //�r���[�ϊ���̖@���x�N�g��
	float2 uv : TEXCOORD;       //uv�l
	float3 ray : VECTOR;        //�����x�N�g��
};