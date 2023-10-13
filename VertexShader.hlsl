struct VS_INPUT		// 頂点シェーダ
{
	float4 position : POSITION;		// 頂点座標
	float2 uv : TEXCOORD0;			// テクセル(UV座標)
};

struct VS_OUTPUT	// 頂点シェーダ
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;			// 基本色UV値
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;

	output.position = input.position;
	output.uv = input.uv;

	return output;
}

