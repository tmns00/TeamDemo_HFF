#include"CollisionPrimitive.h"

void Col_Triangle::ComputeNormal(){
	DirectX::XMVECTOR p0_p1;
	p0_p1.m128_f32[0] = p1.m128_f32[0] - p0.m128_f32[0];
	p0_p1.m128_f32[1] = p1.m128_f32[1] - p0.m128_f32[1];
	p0_p1.m128_f32[2] = p1.m128_f32[2] - p0.m128_f32[2];
	p0_p1.m128_f32[3] = p1.m128_f32[3] - p0.m128_f32[3];

	DirectX::XMVECTOR p0_p2;
	p0_p2.m128_f32[0] = p2.m128_f32[0] - p0.m128_f32[0];
	p0_p2.m128_f32[1] = p2.m128_f32[1] - p0.m128_f32[1];
	p0_p2.m128_f32[2] = p2.m128_f32[2] - p0.m128_f32[2];
	p0_p2.m128_f32[3] = p2.m128_f32[3] - p0.m128_f32[3];

	//外積により、2辺に垂直なベクトルを算出する
	normal = DirectX::XMVector3Cross(p0_p1, p0_p2);
	normal = DirectX::XMVector3Normalize(normal);
}
