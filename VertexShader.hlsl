struct VS_INPUT
{
	float4 pos : POSITION;
	float2 uv : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT i)
{
	VS_OUTPUT o;

	o.pos = i.pos;
	o.uv = i.uv;

	return o;
}

