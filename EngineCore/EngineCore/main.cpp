#include "pch.h"
#include "Application.h"

using namespace DirectX;

#pragma comment(lib, "runtimeobject.lib")

int main(int argc, const char* argv[])
{
	Microsoft::WRL::Wrappers::RoInitializeWrapper InitializeWinRT(RO_INIT_MULTITHREADED);

	HINSTANCE hInst = GetModuleHandle(0);

	try
	{
		Application app(hInst);

		auto setup = []()
		{

		};

		return app.Run(1024, 768, setup);
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}

}

// https://www.3dgep.com/learning-directx-12-2/#Inline_Descriptors
// TODO Command Context
// GPU buffer
/*
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	int exit = 0;
	MSG msg;
	WNDCLASS win;
	ZeroMemory( &win, sizeof(WNDCLASS) );
	win.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
	win.lpfnWndProc = WindowProc;
	win.hInstance = 0;
	win.lpszClassName = "DirectX 12 Demo";
	win.hbrBackground =(HBRUSH)(COLOR_WINDOW+1);
	RegisterClass(&win);
	HWND hWnd = CreateWindowEx(0, win.lpszClassName, "DirectX 12 Demo", WS_VISIBLE|WS_OVERLAPPEDWINDOW, 0, 0, WIDTH, HEIGHT, 0, 0, 0, 0);
	HDC hdc = GetDC(hWnd);
	IDXGIFactory4* pFactory;
	CreateDXGIFactory1((REFIID)&IID_IDXGIFactory4, (LPVOID*)(&pFactory));
	D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_12_0, (REFIID)&IID_ID3D12Device, (LPVOID*)(&mDevice));
	D3D12_COMMAND_QUEUE_DESC queueDesc = {D3D12_COMMAND_LIST_TYPE_DIRECT, 0, D3D12_COMMAND_QUEUE_FLAG_NONE, 0};
	mDevice->lpVtbl->CreateCommandQueue(mDevice, &queueDesc, (REFIID)&IID_ID3D12CommandQueue, (LPVOID*)(&mCommandQueue));
	DXGI_SWAP_CHAIN_DESC descSwapChain = {(DXGI_MODE_DESC){WIDTH,HEIGHT,{0,0},DXGI_FORMAT_R8G8B8A8_UNORM,0,0},(DXGI_SAMPLE_DESC){1,0},1L << (1 + 4),FRAMES,hWnd,1,3,0};
	IDXGISwapChain* SwapChain;
	pFactory->lpVtbl->CreateSwapChain(pFactory, (IUnknown *)mCommandQueue, &descSwapChain, &SwapChain);
	SwapChain->lpVtbl->QueryInterface(SwapChain, (REFIID)&IID_IDXGISwapChain3, (LPVOID*)(&mSwapChain));
	SwapChain->lpVtbl->Release(SwapChain);
	D3D12_INPUT_ELEMENT_DESC layout[] ={{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }};
	ID3DBlob* blob;
	D3D12_ROOT_PARAMETER timeParam;
	ZeroMemory(&timeParam, sizeof(timeParam));
	timeParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	timeParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	timeParam.Constants = (D3D12_ROOT_CONSTANTS){ 0,0, 1  };
	D3D12_ROOT_SIGNATURE_DESC descRootSignature = {1, &timeParam, 0, NULL, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT};
	D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &blob, 0);
	mDevice->lpVtbl->CreateRootSignature(mDevice, 0, blob->lpVtbl->GetBufferPointer(blob), blob->lpVtbl->GetBufferSize(blob), (REFIID)&IID_ID3D12RootSignature, (LPVOID*)(&mRootSignature));
	D3D12_RASTERIZER_DESC rasterizer ={D3D12_FILL_MODE_SOLID,D3D12_CULL_MODE_BACK,0,D3D12_DEFAULT_DEPTH_BIAS,D3D12_DEFAULT_DEPTH_BIAS_CLAMP,0.0f,1,0,0,0,0};
	D3D12_BLEND_DESC blendstate = { 0, 0,{0, 0, 1, 0, D3D12_BLEND_OP_ADD, 1, 0, D3D12_BLEND_OP_ADD, D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL} };
	static D3D12_GRAPHICS_PIPELINE_STATE_DESC pDesc;
	pDesc.pRootSignature = mRootSignature;
	pDesc.VS = (D3D12_SHADER_BYTECODE){VertexShader,sizeof(VertexShader)};
	pDesc.PS = (D3D12_SHADER_BYTECODE){PixelShader,sizeof(PixelShader)};
	pDesc.InputLayout = (D3D12_INPUT_LAYOUT_DESC) {layout, _countof(layout)};
	pDesc.RasterizerState = rasterizer;
	pDesc.BlendState = blendstate;
	pDesc.DepthStencilState = (D3D12_DEPTH_STENCIL_DESC){0,0,0,0,0,0,0,0};
	pDesc.SampleMask = UINT_MAX;
	pDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pDesc.NumRenderTargets = 1;
	pDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	pDesc.SampleDesc.Count = 1;
	mDevice->lpVtbl->CreateGraphicsPipelineState(mDevice, &pDesc, (REFIID)&IID_ID3D12PipelineState, (LPVOID*)(&mPSO));
	static D3D12_DESCRIPTOR_HEAP_DESC descHeap = {D3D12_DESCRIPTOR_HEAP_TYPE_RTV, FRAMES, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0};
	mDevice->lpVtbl->CreateDescriptorHeap(mDevice, &descHeap, (REFIID)&IID_ID3D12DescriptorHeap, (LPVOID*)(&mDescriptorHeap));
	UINT mrtvDescriptorIncrSize = mDevice->lpVtbl->GetDescriptorHandleIncrementSize(mDevice, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
	((void(__stdcall*)(ID3D12DescriptorHeap*, D3D12_CPU_DESCRIPTOR_HANDLE*)) mDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart)(mDescriptorHeap, &rtvHandle);
	for (UINT i = 0; i < FRAMES; i++)
	{
		mSwapChain->lpVtbl->GetBuffer(mSwapChain, i, (REFIID)&IID_ID3D12Resource, (LPVOID*)(&mRenderTarget[i]));
		mDevice->lpVtbl->CreateRenderTargetView(mDevice, mRenderTarget[i], NULL, rtvHandle);
		rtvHandle.ptr += mrtvDescriptorIncrSize;
	}
	mDevice->lpVtbl->CreateCommandAllocator(mDevice, D3D12_COMMAND_LIST_TYPE_DIRECT, (REFIID)&IID_ID3D12CommandAllocator, (LPVOID*)(&mCommandAllocator));
	mDevice->lpVtbl->CreateCommandList(mDevice, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocator, mPSO, (REFIID)&IID_ID3D12CommandList, (LPVOID*)(&mCommandList));
	D3D12_VIEWPORT mViewport = { 0.0f, 0.0f, (float)(WIDTH), (float)(HEIGHT), 0.0f, 1.0f };
	D3D12_RECT mRectScissor = { 0, 0, (LONG)(WIDTH), (LONG)(HEIGHT) };
	float vertices[] ={ -1.0f, -1.0f, 0.0f, -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, -1.0f, 1.0, 0.0f, 1.0f, 1.0f, 0.0f};
	static D3D12_HEAP_PROPERTIES heapProperties ={D3D12_HEAP_TYPE_UPLOAD,D3D12_CPU_PAGE_PROPERTY_UNKNOWN,D3D12_MEMORY_POOL_UNKNOWN,1,1};
	static D3D12_RESOURCE_DESC VertexBufferDesc ={D3D12_RESOURCE_DIMENSION_BUFFER,0,_countof(vertices) * 12,1,1,1,DXGI_FORMAT_UNKNOWN,{1,0},1,0};
	mDevice->lpVtbl->CreateCommittedResource(mDevice,&heapProperties,0,&VertexBufferDesc,D3D12_RESOURCE_STATE_GENERIC_READ,NULL,(REFIID)&IID_ID3D12Resource, (LPVOID*)(&buffer));
	float* data;
	buffer->lpVtbl->Map(buffer, 0, NULL, (void**)(&data));
	memcpy(data, vertices, sizeof(vertices));
	buffer->lpVtbl->Unmap(buffer, 0, NULL);
	D3D12_VERTEX_BUFFER_VIEW mDescViewBufVert = {buffer->lpVtbl->GetGPUVirtualAddress(buffer), sizeof(vertices), 12};
	mCommandList->lpVtbl->Close(mCommandList);
	mDevice->lpVtbl->CreateFence(mDevice, 0, D3D12_FENCE_FLAG_NONE, (REFIID)&IID_ID3D12Fence, (LPVOID*)(&mFence));
	mFenceValue = 1;
	mfenceEvent = CreateEventEx(NULL, FALSE, FALSE, EVENT_ALL_ACCESS);
	while (!exit)
	{
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) exit = 1;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		mCommandAllocator->lpVtbl->Reset(mCommandAllocator);
		mCommandList->lpVtbl->Reset(mCommandList, mCommandAllocator, mPSO);
		mCommandList->lpVtbl->SetGraphicsRootSignature(mCommandList, mRootSignature);
		float timer = GetTickCount() * 0.001f;
		mCommandList->lpVtbl->SetGraphicsRoot32BitConstants(mCommandList,0,1,&timer,0);
		mCommandList->lpVtbl->RSSetViewports(mCommandList, 1, &mViewport);
		mCommandList->lpVtbl->RSSetScissorRects(mCommandList, 1, &mRectScissor);
		D3D12_RESOURCE_BARRIER barrierRTAsTexture = {0, 0,{ mRenderTarget[mframeIndex], D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET }};
		mCommandList->lpVtbl->ResourceBarrier(mCommandList, 1, &barrierRTAsTexture);
		((void(__stdcall*)(ID3D12DescriptorHeap*, D3D12_CPU_DESCRIPTOR_HANDLE*)) mDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart)(mDescriptorHeap, &rtvHandle);
		rtvHandle.ptr += mframeIndex * mrtvDescriptorIncrSize;
		float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		mCommandList->lpVtbl->ClearRenderTargetView(mCommandList, rtvHandle, clearColor, 0, NULL);
		mCommandList->lpVtbl->OMSetRenderTargets(mCommandList, 1, &rtvHandle, TRUE, NULL);
		mCommandList->lpVtbl->IASetPrimitiveTopology(mCommandList, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		mCommandList->lpVtbl->IASetVertexBuffers(mCommandList, 0, 1, &mDescViewBufVert);
		mCommandList->lpVtbl->DrawInstanced(mCommandList, 6, 1, 0, 0);
		D3D12_RESOURCE_BARRIER barrierRTForPresent = {0, 0,{ mRenderTarget[mframeIndex], D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT }};
		mCommandList->lpVtbl->ResourceBarrier(mCommandList, 1, &barrierRTForPresent);
		mCommandList->lpVtbl->Close(mCommandList);
		ID3D12CommandList* ppCommandLists[] = { (ID3D12CommandList *) mCommandList };
		mCommandQueue->lpVtbl->ExecuteCommandLists(mCommandQueue, _countof(ppCommandLists), ppCommandLists);
		mSwapChain->lpVtbl->Present(mSwapChain, 0, 0);
		WaitForPreviousFrame();
	}
	WaitForPreviousFrame();
	CloseHandle(mfenceEvent);
	mDevice->lpVtbl->Release(mDevice);
	mSwapChain->lpVtbl->Release(mSwapChain);
	buffer->lpVtbl->Release(buffer);
	for (UINT n = 0; n < FRAMES; n++) mRenderTarget[n]->lpVtbl->Release(mRenderTarget[n]);
	mCommandAllocator->lpVtbl->Release(mCommandAllocator);
	mCommandQueue->lpVtbl->Release(mCommandQueue);
	mDescriptorHeap->lpVtbl->Release(mDescriptorHeap);
	mCommandList->lpVtbl->Release(mCommandList);
	mPSO->lpVtbl->Release(mPSO);
	mFence->lpVtbl->Release(mFence);
	mRootSignature->lpVtbl->Release(mRootSignature);
	return 0;
}
*/
