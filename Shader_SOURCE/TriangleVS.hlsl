
// VSIn은 버텍스셰이더로 들어오는 정보.(정점데이터)
struct VSIn
{
	float3 Pos : POSITION;
	float4 Color : COLOR;
};

// VSOut은 나가는 데이터. 
struct VSOut
{
	// SV_POSITION은 무조건 float4로 맞춰줘야함. 3으로 하니까 오류났음.
	float4 Pos : SV_Position;
	float4 Color : COLOR;
};

cbuffer Transform : register(b0)
{
    float4 Pos;
}

VSOut main(VSIn In) 
{
	// 0.0f로 모든값 초기화.
	VSOut Out = (VSOut)0.0f;

	Out.Pos = float4(In.Pos, 1.0f);
    Out.Pos.x += Pos.x;
    Out.Pos.y += Pos.y;
	
    Out.Color = In.Color;

	return Out;
}