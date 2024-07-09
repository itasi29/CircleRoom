/*波紋を描いたシェーダーの画面画像と画像を合わせるシェーダー(の認識であっているはず)*/

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
	float4 color	: SV_TARGET0;
};

SamplerState sam;
// テクスチャ
Texture2D tex : register(t0);
Texture2D nor : register(t1);

cbuffer ConstantBuffer : register(b0)
{
	float threthold;
	float angle;
}

PsOutput main(PsInput input)
{
	PsOutput output;

	// サンプリング
	float2 n = nor.Sample(sam, input.uv).xy;
	// -1~1の範囲に
	n = n * 2 - 1;

	float2 ygm = n.xy * 0.05;

	output.color = tex.Sample(sam, input.uv + ygm);

	return output;
}