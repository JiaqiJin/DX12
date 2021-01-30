#include "MyBaseApp.h"
#include "DeviceResources.h"
#include "GameTimer.h"
#include <d3dcompiler.h>

using namespace MyDirectX;
using namespace DirectX;

using Microsoft::WRL::ComPtr;

struct MyVertex
{
	XMFLOAT3 position;
	XMFLOAT2 uv;
	XMFLOAT4 color;
};

struct CBPerFrame
{
	XMFLOAT4 baseColor;
};

MyBaseApp::MyBaseApp(HINSTANCE hInstance, const wchar_t* title, UINT width, UINT height)
	:MyApp(hInstance, title, width, height)
{
}

void MyBaseApp::Update()
{
	// TODO
	float totalTime = m_Timer->TotalTime();
	float val = 0.5f * sin(totalTime) + 0.5f;

	m_BaseColor = XMFLOAT4(val, 1.0f, 1.0f, 1.0f);

	CBPerFrame cbPerFrame = {};
	cbPerFrame.baseColor = m_BaseColor;

	memcpy(m_pCBVDataBegin, &cbPerFrame, sizeof(CBPerFrame));
}

void MyBaseApp::Render()
{

}

void MyBaseApp::InitAssets()
{

}

void MyBaseApp::InitRootSignatures()
{

}

void MyBaseApp::InitShadersAndInputElements()
{

}

void MyBaseApp::InitPipelineStates()
{

}

void MyBaseApp::InitGeometryBuffers()
{

}

void MyBaseApp::InitVertexBuffers()
{

}

void MyBaseApp::InitIndexBuffers()
{

}

void MyBaseApp::InitConstantBuffers()
{

}

void MyBaseApp::InitTextures()
{

}

void MyBaseApp::CustomInit()
{
	// TO DO
}

// Generate a simple black and white checkerboard texture.
std::vector<UINT8> MyBaseApp::GenerateTextureData(bool bRandColor)
{
	const UINT rowPitch = m_TexWidth * m_TexHeight;
	const UINT cellPitch = rowPitch >> 3;		// The width of a cell in the checkboard texture.
	const UINT cellHeight = m_TexWidth >> 3;	// The height of a cell in the checkerboard texture.
	const UINT textureSize = rowPitch * m_TexHeight;

	std::vector<UINT8> data(textureSize);
	UINT8* pData = &data[0];
	UINT8 r = 0xff;
	UINT8 g = 0xff;
	UINT8 b = 0xff;

	if (bRandColor)
	{
		r = rand() & 0xff;
		g = rand() & 0xff;
		b = rand() & 0xff;
	}

	for (UINT n = 0; n < textureSize; n += m_TexturePixelSize)
	{
		UINT x = n % rowPitch;
		UINT y = n / rowPitch;
		UINT i = x / cellPitch;
		UINT j = y / cellHeight;

		if (i % 2 == j % 2)
		{
			pData[n] = 0x00;        // R
			pData[n + 1] = 0x00;    // G
			pData[n + 2] = 0x00;    // B
			pData[n + 3] = 0xff;    // A
		}
		else
		{
			pData[n] = r;			// R
			pData[n + 1] = g;		// G
			pData[n + 2] = b;		// B
			pData[n + 3] = 0xff;    // A
		}
	}

	return data;
}
void MyBaseApp::DisposeUploaders()
{
	m_UploadVB = nullptr;
	m_UploadIB = nullptr;

	m_UploadTex = nullptr;
}