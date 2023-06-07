#include "jsGraphicDevice_DX11.h"
#include "jsApplication.h"
#include "jsRenderer.h"

extern js::Application application;

namespace js::graphics
{
	GraphicDevice_DX11::GraphicDevice_DX11()
	{
		// Device, Context 생성
		HWND hwnd = application.GetHwnd();
		UINT deviceFlag = D3D11_CREATE_DEVICE_DEBUG;
		D3D_FEATURE_LEVEL featureLevel = (D3D_FEATURE_LEVEL)0;

		D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceFlag, nullptr, 0
			, D3D11_SDK_VERSION, mDevice.GetAddressOf(), &featureLevel, mContext.GetAddressOf());

		// SwapChain 생성
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferCount = 2;
		swapChainDesc.BufferDesc.Width = application.GetWidth();
		swapChainDesc.BufferDesc.Height = application.GetHeight();

		if (!CreateSwapChain(&swapChainDesc, hwnd))
			return;

		// get renderTarget by SwapChain
		// 스왑체인을 만드는 순간 렌더타겟이 생기는데 그걸 이제 우리의 mSwapchain변수에다가  옮겨줘야함
		if (FAILED(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)mRenderTarget.GetAddressOf())))
			return;

		// create renderTargetView
		mDevice->CreateRenderTargetView((ID3D11Resource*)mRenderTarget.Get(), nullptr, mRenderTargetView.GetAddressOf());

		D3D11_TEXTURE2D_DESC depthStencilDesc = {};
		depthStencilDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		depthStencilDesc.CPUAccessFlags = 0;

		depthStencilDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.Width = application.GetWidth();
		depthStencilDesc.Height = application.GetHeight();
		depthStencilDesc.ArraySize = 1;

		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;

		depthStencilDesc.MipLevels = 0;
		depthStencilDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA data;
		if (!CreateTexture(&depthStencilDesc, &data))
			return;

		RECT winRect = {};
		GetClientRect(hwnd, &winRect);

		mViewPort =
		{
			0.0f, 0.0f
			, (float)(winRect.right - winRect.left)
			, (float)(winRect.bottom - winRect.top)
			, 0.0f, 1.0f
		};

		BindViewPort(&mViewPort);

		mContext->OMSetRenderTargets(1, mRenderTargetView.GetAddressOf(), mDepthStencilView.Get());

	}

	GraphicDevice_DX11::~GraphicDevice_DX11()
	{
	}

	bool GraphicDevice_DX11::CreateSwapChain(const DXGI_SWAP_CHAIN_DESC* desc, HWND hwnd)
	{
		DXGI_SWAP_CHAIN_DESC dxgiDesc = {};
		dxgiDesc.OutputWindow = hwnd;
		dxgiDesc.Windowed = true;
		dxgiDesc.BufferCount = desc->BufferCount;
		// swapeffect::discard = 백버퍼의 내용을 삭제하고 다시 덮어써라.
		dxgiDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD; 

		dxgiDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		dxgiDesc.BufferDesc.Width = desc->BufferDesc.Width;
		dxgiDesc.BufferDesc.Height = desc->BufferDesc.Height;
		dxgiDesc.BufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
		dxgiDesc.BufferDesc.RefreshRate.Numerator = 240;
		dxgiDesc.BufferDesc.RefreshRate.Denominator = 1;
		dxgiDesc.BufferDesc.Scaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_UNSPECIFIED;
		dxgiDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

		dxgiDesc.SampleDesc.Count = 1;
		dxgiDesc.SampleDesc.Quality = 0;

		// device = 렌더타겟을 만들어줌
		Microsoft::WRL::ComPtr<IDXGIDevice> pDXGIDevice = nullptr;
		// adapter = 모니터를 의미.
		Microsoft::WRL::ComPtr<IDXGIAdapter> pAdapter = nullptr;
		// factory = 전체화면 전환을 처리해줌
		Microsoft::WRL::ComPtr<IDXGIFactory> pFactory= nullptr;

		if (FAILED(mDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)pDXGIDevice.GetAddressOf())))
			return false;
		if (FAILED(pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void**)pAdapter.GetAddressOf())))
			return false;
		if (FAILED(pAdapter->GetParent(__uuidof(IDXGIFactory), (void**)pFactory.GetAddressOf())))
			return false;

		if (FAILED(pFactory->CreateSwapChain(mDevice.Get(), &dxgiDesc, mSwapChain.GetAddressOf())))
			return false;

		return true;
	}

	bool GraphicDevice_DX11::CreateBuffer(ID3D11Buffer** buffer, D3D11_BUFFER_DESC* desc, D3D11_SUBRESOURCE_DATA* data)
	{
		if (FAILED(mDevice->CreateBuffer(desc, data, buffer)))
			return false;

		return true;
	}

	bool GraphicDevice_DX11::CreateShader()
	{
		// 셰이더코드를 컴파일 한번 해줘야함(수동으로)
		ID3DBlob* vsBlob = nullptr;
		std::filesystem::path shaderPath = std::filesystem::current_path().parent_path();
		shaderPath += L"\\Shader_SOURCE\\";

		std::filesystem::path vsPath(shaderPath.c_str());
		vsPath += L"TriangleVS.hlsl";

		D3DCompileFromFile(vsPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0
			, &renderer::triangleVSBlob, &renderer::errorBlob);

		if (renderer::errorBlob)
		{
			OutputDebugStringA((char*)renderer::errorBlob->GetBufferPointer());
			renderer::errorBlob->Release();
		}

		mDevice->CreateVertexShader(renderer::triangleVSBlob->GetBufferPointer(), renderer::triangleVSBlob->GetBufferSize()
			, nullptr, &renderer::triangleVSShader);
		// 블롭은 자료형이 blob 이므로 GetBufferPointer()

		std::filesystem::path psPath(shaderPath.c_str());
		psPath += L"TrianglePS.hlsl";

		D3DCompileFromFile(psPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0
			, &renderer::trianglePSBlob, &renderer::errorBlob);

		if (renderer::errorBlob)
		{
			OutputDebugStringA((char*)renderer::errorBlob->GetBufferPointer());
			renderer::errorBlob->Release();
		}

		mDevice->CreatePixelShader(renderer::trianglePSBlob->GetBufferPointer(), renderer::trianglePSBlob->GetBufferSize()
			, nullptr, &renderer::trianglePSShader);

		// Input Layout 정점 구조 정보를 넘겨줘야 한다.
		D3D11_INPUT_ELEMENT_DESC arrLayout[2] = {};
		// 우리는 셰이더에 포지션과 컬러 두 가지 정보를 가지고 있으므로 배열의 크기는 2

		arrLayout[0].AlignedByteOffset = 0;
		arrLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		arrLayout[0].InputSlot = 0;
		arrLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		arrLayout[0].SemanticName = "POSITION";
		arrLayout[0].SemanticIndex = 0;

		arrLayout[1].AlignedByteOffset = 12;
		arrLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		arrLayout[1].InputSlot = 0;
		arrLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		arrLayout[1].SemanticName = "COLOR";
		arrLayout[1].SemanticIndex = 0;

		mDevice->CreateInputLayout(arrLayout, 2
			, renderer::triangleVSBlob->GetBufferPointer()
			, renderer::triangleVSBlob->GetBufferSize()
			, &renderer::triangleLayout);

		return true;
	}

	bool GraphicDevice_DX11::CreateTexture(const D3D11_TEXTURE2D_DESC* desc, void* data)
	{
		D3D11_TEXTURE2D_DESC dxgiDesc = {};
		dxgiDesc.BindFlags = desc->BindFlags;
		dxgiDesc.Usage = desc->Usage;
		dxgiDesc.CPUAccessFlags = 0;

		dxgiDesc.Format = desc->Format;
		dxgiDesc.Width = desc->Width;
		dxgiDesc.Height = desc->Height;
		dxgiDesc.ArraySize = desc->ArraySize;

		dxgiDesc.SampleDesc.Count = desc->SampleDesc.Count;
		dxgiDesc.SampleDesc.Quality = 0;

		// 밉맵레벨은 중요한데, 밉맵레벨이란 원래의 텍스쳐 이미지를 우리가 원하는 사이즈에 맞게 줄일때에
		// 미리 텍스쳐 사이즈를 크기별로 만들어놓고 그에 적당한 사이즈에 맞게 텍스쳐를 넣음
		dxgiDesc.MipLevels = desc->MipLevels;
		dxgiDesc.MiscFlags = desc->MiscFlags;

		if(FAILED(mDevice->CreateTexture2D(&dxgiDesc, nullptr, mDepthStencilBuffer.GetAddressOf())))
			return false;

		if (FAILED(mDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), nullptr, mDepthStencilView.GetAddressOf())))
			return false;

		return true;
	}

	void GraphicDevice_DX11::BindViewPort(D3D11_VIEWPORT* viewPort)
	{
		mContext->RSSetViewports(1, viewPort);
	}

	void GraphicDevice_DX11::SetConstantBuffer(ID3D11Buffer* buffer, void* data, UINT size)
	{
		D3D11_MAPPED_SUBRESOURCE subRes = {};
		mContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subRes);
		memcpy(subRes.pData, data, size);
		mContext->Unmap(buffer, 0);
	}

	void GraphicDevice_DX11::BindConstantBuffer(eShaderStage stage, eCBType type, ID3D11Buffer* buffer)
	{
		switch (stage)
		{
		case eShaderStage::VS:
			mContext->VSSetConstantBuffers((UINT)type, 1, &buffer);
			break;
		case eShaderStage::HS:
			mContext->HSSetConstantBuffers((UINT)type, 1, &buffer);
			break;
		case eShaderStage::DS:
			mContext->DSSetConstantBuffers((UINT)type, 1, &buffer);
			break;
		case eShaderStage::GS:
			mContext->GSSetConstantBuffers((UINT)type, 1, &buffer);
			break;
		case eShaderStage::PS:
			mContext->PSSetConstantBuffers((UINT)type, 1, &buffer);
			break;
		case eShaderStage::CS:
			mContext->CSSetConstantBuffers((UINT)type, 1, &buffer);
			break;
		case eShaderStage::End:
			break;
		default:
			break;
		}
	}

	void GraphicDevice_DX11::BindsConstantBuffer(eShaderStage stage, eCBType type, ID3D11Buffer* buffer)
	{
		mContext->VSSetConstantBuffers((UINT)type, 1, &buffer);
		mContext->HSSetConstantBuffers((UINT)type, 1, &buffer);
		mContext->DSSetConstantBuffers((UINT)type, 1, &buffer);
		mContext->GSSetConstantBuffers((UINT)type, 1, &buffer);
		mContext->PSSetConstantBuffers((UINT)type, 1, &buffer);
		mContext->CSSetConstantBuffers((UINT)type, 1, &buffer);
	}

	void GraphicDevice_DX11::Draw()
	{
		FLOAT bgColor[4] = { 0.2f, 0.2f, 0.2f, 1.0f };

		// 화면에 그리기 전에 이전에 있던 내용은 지워주고
		mContext->ClearRenderTargetView(mRenderTargetView.Get(), bgColor);
		mContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);

		// viewport update
		HWND hwnd = application.GetHwnd();
		RECT winRect = {};
		GetClientRect(hwnd, &winRect);
		mViewPort =
		{
			0.0f, 0.0f
			, (float)(winRect.right - winRect.left)
			, (float)(winRect.bottom - winRect.top)
			, 0.0f, 1.0f
		};

		BindViewPort(&mViewPort);
		mContext->OMSetRenderTargets(1, mRenderTargetView.GetAddressOf(), mDepthStencilView.Get());

		// input assembler 정점 데이터 정보 지정
		UINT vertexsize = sizeof(renderer::Vertex);
		UINT offset = 0;

		mContext->IASetVertexBuffers(0, 1, &renderer::triangleBuffer, &vertexsize, &offset);
		mContext->IASetIndexBuffer(renderer::triangleIdxBuffer, DXGI_FORMAT_R32_UINT, 0);
		mContext->IASetInputLayout(renderer::triangleLayout);
		mContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Bind VS,PS
		mContext->VSSetShader(renderer::triangleVSShader, 0, 0);
		mContext->PSSetShader(renderer::trianglePSShader, 0, 0);

		// Draw Render Target
		//mContext->Draw(3, 0);
		mContext->DrawIndexed(6, 0, 0);
		// 클리어와 present사이에 우리가 그려줄 것을 넣어줌.

		// 화면에 그려줌
		mSwapChain->Present(0, 0);
	}

}

