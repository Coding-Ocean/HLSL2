// 定数バッファ変数
cbuffer ConstantBuffer : register(b0)
{
	float Time;
};

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};

float4 main(PS_INPUT i) : SV_TARGET
{
	float3 col = float3(1, 1, 1);
	return float4(col, 1);
}
