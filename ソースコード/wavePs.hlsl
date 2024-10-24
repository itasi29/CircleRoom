// ピクセルシェーダーの入力
struct PsInput
{
	float4 pos	: SV_POSITION;
	float4 dif	: COLOR0;
	float2 uv	: TEXCOORD0;
};

// 出力
struct PsOutput
{
	float4 output	: SV_TARGET0;
};

cbuffer Constant : register(b3)
{
	float angle;
}

PsOutput main(PsInput psInput)
{
	PsOutput psOutput;

	float2 xy = psInput.uv - 0.5;
	float posAngle = atan2(xy.y, xy.x);

	float len = length(xy);

	float h = 0.5 + (clamp(sin(len - angle), 0.0, 0.5) * cos(posAngle + 3.14)) * abs(sin(angle / 5.0));

	// 色の計算
	psOutput.output.rgb = h;
	// α値は1のままで
	psOutput.output.a = 1;

	return psOutput;
}