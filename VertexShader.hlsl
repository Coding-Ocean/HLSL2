struct VS_INPUT		// ���_�V�F�[�_
{
	float4 position : POSITION;		// ���_���W
	float2 uv : TEXCOORD0;			// �e�N�Z��(UV���W)
};

struct VS_OUTPUT	// ���_�V�F�[�_
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;			// ��{�FUV�l
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;

	output.position = input.position;
	output.uv = input.uv;

	return output;
}

