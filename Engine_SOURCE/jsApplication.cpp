#include "jsApplication.h"
#include "jsTime.h"
#include "jsInput.h"
#include "jsRenderer.h"
#include "jsMath.h"

namespace js
{
	Application::Application()
		: graphicDevice(nullptr)
		, mHwnd(NULL)
		, mWidth(-1)
		, mHeight(-1)
		, pos(0.0f, 0.0f, 0.0f, 1.0f)
	{
	}
	Application::~Application()
	{
	}
	void Application::Run()
	{
		Update();
		LateUpdate();
		Render();
	}
	void Application::Initialize()
	{
		Time::Initialize();
		Input::Initialize();

		renderer::Initialize();
	}
	void Application::Update()
	{
		Time::Update();
		Input::Update();


		if (Input::GetKeyDown(eKeyCode::W))
		{
			pos.y += 0.1f;
		}
		if (Input::GetKeyDown(eKeyCode::A))
		{
			pos.x -= 0.1f;
		}
		if (Input::GetKeyDown(eKeyCode::S))
		{
			pos.y -= 0.1f;
		}
		if (Input::GetKeyDown(eKeyCode::D))
		{
			pos.x += 0.1f;
		}
		js::graphics::GetDevice()->SetConstantBuffer(renderer::triangleConstantBuffer, &pos, sizeof(Vector4));
		js::graphics::GetDevice()->BindConstantBuffer(eShaderStage::VS, eCBType::Transform, renderer::triangleConstantBuffer);

	}
	void Application::LateUpdate()
	{
	}
	void Application::Render()
	{
		Time::Render();

		graphicDevice->Draw();
	}
	void Application::SetWindow(HWND hwnd, UINT width, UINT height)
	{
		if (graphicDevice == nullptr)
		{
			mHwnd = hwnd;
			mWidth = width;
			mHeight = height;

			graphicDevice = std::make_unique<js::graphics::GraphicDevice_DX11>();
			js::graphics::GetDevice() = graphicDevice.get();
		}

		RECT rt = { 0, 0, (LONG)width , (LONG)height };
		AdjustWindowRect(&rt, WS_OVERLAPPEDWINDOW, false);
		SetWindowPos(mHwnd, nullptr, 0, 0, rt.right - rt.left, rt.bottom - rt.top, 0);
		ShowWindow(mHwnd, true);
		UpdateWindow(mHwnd);
	}
}