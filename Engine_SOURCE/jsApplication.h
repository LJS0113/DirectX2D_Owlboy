#pragma once
#include "JSEngine.h"
#include "jsGraphicDevice_DX11.h"
namespace js
{
	class Application
	{
	public:
		Application();
		~Application();

		void Run();

		void Initialize();
		void Update();
		void LateUpdate();
		void Render();

		void SetWindow(HWND hwnd, UINT width, UINT height);

		UINT GetWidth() { return mWidth; }
		UINT GetHeight() { return mHeight; }
		HWND GetHwnd() { return mHwnd; }

	private:
		bool mbInitialize = false;

		// ������ �Ѱ��� ��ü�� ����� �ִ� ����Ʈ ������
		std::unique_ptr<js::graphics::GraphicDevice_DX11> graphicDevice;
									
		// HDC mHdc; -> GPU API
		HWND mHwnd;
		UINT mWidth;
		UINT mHeight;
	};

}