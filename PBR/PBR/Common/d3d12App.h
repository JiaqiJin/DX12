#pragma once

#include <memory>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

#include "Setting.h"

namespace D3D12 
{
	using Microsoft::WRL::ComPtr;

	class Renderer
	{
	public:
		GLFWwindow* initialize(int width, int height, int maxSamples) ;
		void render(GLFWwindow* window, const ViewSettings& view, const SceneSettings& scene);
		void shutdown();
		void setup();
	private:


	private:
		ComPtr<ID3D12Device> m_device;
		ComPtr<ID3D12CommandQueue> m_commandQueue;
		ComPtr<ID3D12GraphicsCommandList> m_commandList;
		ComPtr<IDXGISwapChain3> m_swapChain;


	};
}