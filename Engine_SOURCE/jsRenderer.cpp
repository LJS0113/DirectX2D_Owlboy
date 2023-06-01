#include "jsRenderer.h"
#include "jsInput.h"


namespace js::renderer
{
	Vertex vertexes[3] = {};

	// Input Layout(정점 정보)
	ID3D11InputLayout* triangleLayout = nullptr;

	// vertexes는 cpu에 저장되어 있는 정보이므로 이걸 gpu로 보내려면 버퍼가 필요함. 그게 버텍스 버퍼.
	// Vertext Buffer
	ID3D11Buffer* triangleBuffer = nullptr;
	ID3D11Buffer* triangleIdxBuffer = nullptr;
	ID3D11Buffer* triangleConstantBuffer = nullptr;

	// 컴파일시에 뭐가 잘못 됐는지 모르니 에러들의 정보를 모아놓는 곳이 에러블롭
	// error blob
	ID3DBlob* errorBlob = nullptr;

	// 셰이더에서 처리한 정보들이 블롭으로 들어감. 버텍스 셰이더에서 처리한 정보들은 버텍스셰이더블롭에 들어가있음.
	// Vertex Shader Code -> binary code
	ID3DBlob* triangleVSBlob = nullptr;

	// 전달받는 정보를 가공하는곳이 버텍스 셰이더.(가공 = 위치정보를 공간에 맞게 변환)
	// blob에는 코드만 들어있으니 이 코드를 실행시키는게 바로 버텍스 셰이더.
	// VertexShader
	ID3D11VertexShader* triangleVSShader = nullptr;

	// PixelShader code -> binary code
	ID3DBlob* trianglePSBlob = nullptr;

	// PixelShader
	ID3D11PixelShader* trianglePSShader = nullptr;

	void SetupState()
	{

	}

	void LoadBuffer()
	{
		D3D11_BUFFER_DESC triangleDesc = {};
		triangleDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC; // default하니까 오류고 다이나믹으로 해줌.
		triangleDesc.ByteWidth = sizeof(Vertex) * 3; // 정점 크기, 개수 우리가 만든 Vertex struct가 총 12개의 정점이므로 곱12
		triangleDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER; // 무슨 버퍼로 사용할 것이냐를 지정.
		triangleDesc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE; // cpu에서 사용이 가능할 것이냐 => 쓸수 있게.

		D3D11_SUBRESOURCE_DATA triangleData = {}; // 데이터를 넘길땐 바로 넘기는게 아니라 서브리소스에 묶어서 보내준다.
		triangleData.pSysMem = vertexes;
		js::graphics::GetDevice()->CreateBuffer(&triangleBuffer, &triangleDesc, &triangleData);

		std::vector<UINT> indexes = {};
		indexes.push_back(0);
		indexes.push_back(1);
		indexes.push_back(2);

		// Index Buffer
		D3D11_BUFFER_DESC triangleIdxDesc = {};
		triangleIdxDesc.ByteWidth = sizeof(UINT) * indexes.size();
		triangleIdxDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
		triangleIdxDesc.Usage = D3D11_USAGE_DEFAULT;
		triangleIdxDesc.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA triangleIdxData = {};
		triangleIdxData.pSysMem = indexes.data();
		js::graphics::GetDevice()->CreateBuffer(&triangleIdxBuffer, &triangleIdxDesc, &triangleIdxData);

		// Constant Buffer
		D3D11_BUFFER_DESC triangleCSDesc = {};
		triangleCSDesc.ByteWidth = sizeof(Vector4);
		triangleCSDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
		triangleCSDesc.Usage = D3D11_USAGE_DYNAMIC;
		triangleCSDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		js::graphics::GetDevice()->CreateBuffer(&triangleConstantBuffer, &triangleCSDesc, nullptr);

		Vector4 pos(0.0f, 0.0f, 0.0f, 1.0f);
		js::graphics::GetDevice()->SetConstantBuffer(renderer::triangleConstantBuffer, &pos, sizeof(Vector4));
		js::graphics::GetDevice()->BindConstantBuffer(eShaderStage::VS, eCBType::Transform, renderer::triangleConstantBuffer);
	}

	void LoadShader()
	{
		js::graphics::GetDevice()->CreateShader();
	}

	void Initialize()
	{
		// 사각형
		// 0 (-1,1)
		vertexes[0].pos = Vector3(0.0f, 0.5f, 0.0f);
		vertexes[0].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
		// 1 (0,1)
		vertexes[1].pos = Vector3(0.5f, -0.5f, 0.0f);
		vertexes[1].color = Vector4(0.0f, 1.0f, 0.0f, 1.0f);
		// 2 (0,0)
		vertexes[2].pos = Vector3(-0.5f, -0.5f, 0.0f);
		vertexes[2].color = Vector4(0.0f, 0.0f, 1.0f, 1.0f);

		SetupState();
		LoadBuffer();
		LoadShader();
	}
}