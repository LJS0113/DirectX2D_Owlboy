#include "jsRenderer.h"

namespace js::renderer
{
	Vertex vertexes[3] = {};

	// Input Layout(���� ����)
	ID3D11InputLayout* triangleLayout = nullptr;

	// vertexes�� cpu�� ����Ǿ� �ִ� �����̹Ƿ� �̰� gpu�� �������� ���۰� �ʿ���. �װ� ���ؽ� ����.
	// Vertext Buffer
	ID3D11Buffer* triangleBuffer = nullptr;

	// �����Ͻÿ� ���� �߸� �ƴ��� �𸣴� �������� ������ ��Ƴ��� ���� �������
	// error blob
	ID3DBlob* errorBlob = nullptr;

	// ���̴����� ó���� �������� ������� ��. ���ؽ� ���̴����� ó���� �������� ���ؽ����̴���ӿ� ������.
	// Vertex Shader Code -> binary code
	ID3DBlob* triangleVSBlob = nullptr;

	// ���޹޴� ������ �����ϴ°��� ���ؽ� ���̴�.(���� = ��ġ������ ������ �°� ��ȯ)
	// blob���� �ڵ常 ��������� �� �ڵ带 �����Ű�°� �ٷ� ���ؽ� ���̴�.
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
		triangleDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC; // default�ϴϱ� ������ ���̳������� ����.
		triangleDesc.ByteWidth = sizeof(Vertex) * 3; // ���� ũ��, ���� �츮�� ���� Vertex struct�� �� 3���� �����̹Ƿ� ��3
		triangleDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER; // ���� ���۷� ����� ���̳ĸ� ����.
		triangleDesc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE; // cpu���� ����� ������ ���̳� => ���� �ְ�.

		D3D11_SUBRESOURCE_DATA triangleData = {}; // �����͸� �ѱ涩 �ٷ� �ѱ�°� �ƴ϶� ���긮�ҽ��� ��� �����ش�.
		triangleData.pSysMem = vertexes;
		js::graphics::GetDevice()->CreateBuffer(&triangleBuffer, &triangleDesc, &triangleData);
	}

	void LoadShader()
	{
		js::graphics::GetDevice()->CreateShader();
	}

	void Initialize()
	{
		vertexes[0].pos = Vector3(0.0f, 0.5f, 0.0f);
		vertexes[0].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);

		vertexes[1].pos = Vector3(0.5f, -0.5f, 0.0f);
		vertexes[1].color = Vector4(0.0f, 1.0f, 0.0f, 1.0f);

		vertexes[2].pos = Vector3(-0.5f, -0.5f, 0.0f);
		vertexes[2].color = Vector4(0.0f, 0.0f, 1.0f, 1.0f);

		SetupState();
		LoadBuffer();
		LoadShader();
	}
}