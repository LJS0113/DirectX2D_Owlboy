#include "jsGraphicDevice_DX11.h"
#include "jsApplication.h"
#include "jsRenderer.h"

extern js::Application application;

namespace js::graphics
{
	GraphicDevice_DX11::GraphicDevice_DX11()
	{
		// Device, Context ����
		HWND hwnd = application.GetHwnd();
		UINT deviceFlag = D3D11_CREATE_DEVICE_DEBUG;
		D3D_FEATURE_LEVEL featureLevel = (D3D_FEATURE_LEVEL)0;

		D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceFlag, nullptr, 0
							,D3D11_SDK_VERSION, mDevice.GetAddressOf(), &featureLevel, mContext.GetAddressOf());

		// SwapChain ����
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferCount = 2;
		swapChainDesc.BufferDesc.Width = application.GetWidth();
		swapChainDesc.BufferDesc.Height = application.GetHeight();

		if (!CreateSwapChain(&swapChainDesc, hwnd))
			return;

		// get renderTarget by SwapChain
		// ����ü���� ����� ���� ����Ÿ���� ����µ� �װ� ���� �츮�� mSwapchain�������ٰ�  �Ű������
		if (FAILED(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)mRenderTarget.GetAddressOf())))
			return;

		// create renderTargetView
		mDevice->CreateRenderTargetView((ID3D11Resource*)mRenderTarget.Get(), nullptr, mRenderTargetView.GetAddressOf());

		D3D11_TEXTURE2D_DESC depthStencilDesc = {};
		depthStencilDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.Width = application.GetWidth();
		depthStencilDesc.Height= application.GetHeight();
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA data;
		if (!CreateTexture(&depthStencilDesc, &data))
			return;

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
		// swapeffect::discard = ������� ������ �����ϰ� �ٽ� ������.
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

		// device = ����Ÿ���� �������
		Microsoft::WRL::ComPtr<IDXGIDevice> pDXGIDevice = nullptr;
		// adapter = ����͸� �ǹ�.
		Microsoft::WRL::ComPtr<IDXGIAdapter> pAdapter = nullptr;
		// factory = ��üȭ�� ��ȯ�� ó������
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
		// ���̴��ڵ带 ������ �ѹ� �������(��������)
		ID3DBlob* vsBlob = nullptr;
		std::filesystem::path shaderPath = std::filesystem::current_path().parent_path();
		shaderPath += L"\\Shader_SOURCE\\";

		std::filesystem::path vsPath(shaderPath.c_str());
		vsPath += L"TriangleVS.hlsl";

		D3DCompileFromFile(vsPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0
			, &js::renderer::triangleVSBlob, &js::renderer::errorBlob);

		if (js::renderer::errorBlob)
		{
			OutputDebugStringA((char*)js::renderer::errorBlob->GetBufferPointer());
			js::renderer::errorBlob->Release();
		}

		mDevice->CreateVertexShader(js::renderer::triangleVSBlob->GetBufferPointer(), js::renderer::triangleVSBlob->GetBufferSize()
			, nullptr, &js::renderer::triangleVSShader);
		// ����� �ڷ����� blob �̹Ƿ� GetBufferPointer()

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

		// �Ӹʷ����� �߿��ѵ�, �Ӹʷ����̶� ������ �ؽ��� �̹����� �츮�� ���ϴ� ����� �°� ���϶���
		// �̸� �ؽ��� ����� ũ�⺰�� �������� �׿� ������ ����� �°� �ؽ��ĸ� ����
		dxgiDesc.MipLevels = desc->MipLevels;
		dxgiDesc.MiscFlags = desc->MiscFlags;

		if(FAILED(mDevice->CreateTexture2D(&dxgiDesc, nullptr, mDepthStencilBuffer.GetAddressOf())))
			return false;

		if (FAILED(mDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), nullptr, mDepthStencilView.GetAddressOf())))
			return false;

		return true;
	}

	void GraphicDevice_DX11::Draw()
	{
		FLOAT bgColor[4] = { 0.2f, 0.2f, 0.2f, 1.0f };

		// ȭ�鿡 �׸��� ���� ������ �ִ� ������ �����ְ�
		mContext->ClearRenderTargetView(mRenderTargetView.Get(), bgColor);

		// Ŭ����� present���̿� �츮�� �׷��� ���� �־���.

		// ȭ�鿡 �׷���
		mSwapChain->Present(0, 0);
	}

}

