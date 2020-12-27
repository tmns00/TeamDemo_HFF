#include "PMDShaderHeader.hlsli"

float4 PSmain(Output input) : SV_TARGET
{
	//����
	float3 light = normalize(float3( 1, -1, 1)); 
	//���C�g�̃J���[ 0~1
	float3 lightColor = float3(1, 1, 1);

	//�f�B�t���[�Y
	float diffLight = dot(-light, input.normal);

	float2 normalUV = (input.normal.xy + float2(1, -1))
		* float2(0.5, -0.5);
	//�X�t�B�A�}�b�v�p��uv
	float2 sphereMapUV = input.vnormal.xy;
	sphereMapUV = (sphereMapUV + float2(1, -1)) * float2(0.5, -0.5); 

	//�e�N�X�`���J���[
	float4 texColor = tex.Sample(smp, input.uv); 

	//���˃x�N�g��
	float3 ref = normalize(reflect(light, input.normal.xyz)); 
	//�X�y�L�����[
	float specLight = pow(
		saturate(dot(ref, -input.ray)),
		specular.a
	); 

	return max(
		diffLight * diffuse * texColor               //�f�B�t���[�Y
		* sphere.Sample(smp, sphereMapUV)            //�X�t�B�A�}�b�v
		+ addSph.Sample(smp, sphereMapUV) * texColor //���Z�X�t�B�A�}�b�v
		+ float4(specLight * specular.rgb, 1)        //�X�y�L�����[
		, float4(ambient * texColor, 1));            //�A���r�G���g
	//return addSph.Sample(smp, sphereMapUV) * texColor;
}