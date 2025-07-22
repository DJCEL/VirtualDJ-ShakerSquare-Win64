#include "ShakerSquare.h"


//------------------------------------------------------------------------------------------
CShakerSquare::CShakerSquare()
{
	pD3DDevice = nullptr; 
	pD3DDeviceContext = nullptr;
	pNewVertexBuffer = nullptr;
	pPixelShader = nullptr;
	pD3DRenderTargetView = nullptr;
	pBlendState = nullptr;
	ZeroMemory(pNewVertices, 6 * sizeof(TVertex8));
	ZeroMemory(m_SliderValue, 2 * sizeof(float));
	m_DirectX_On = false;
	m_WidthOnDeviceInit = 0;
	m_HeightOnDeviceInit = 0;
	m_Width = 0;
	m_Height = 0;
	m_VertexCount = 0;
	m_Alpha = 1.0f;
	m_Speed = 0.0f;
	m_PosVideo = 1;
	m_Inverted = 0;
	m_SavedSongPosBeats = 0;
}
//------------------------------------------------------------------------------------------
CShakerSquare::~CShakerSquare()
{

}
//------------------------------------------------------------------------------------------
HRESULT VDJ_API CShakerSquare::OnLoad()
{
	HRESULT hr = S_FALSE;

	hr = DeclareParameterSlider(&m_SliderValue[0], ID_SLIDER_1, "Wet/Dry", "W/D", 1.0f);
	hr = DeclareParameterSlider(&m_SliderValue[1], ID_SLIDER_2, "Speed", "S", 0.28f);
	hr = DeclareParameterSwitch(&m_Inverted, ID_SWITCH_1, "Inverted", "I", 0.0f);

	OnParameter(ID_INIT);
	return S_OK;
}
//------------------------------------------------------------------------------------------
HRESULT VDJ_API CShakerSquare::OnGetPluginInfo(TVdjPluginInfo8 *info)
{
	info->Author = "djcel";
	info->PluginName = "ShakerSquare";
	info->Description = "Move the video on the beat.";
	info->Flags = 0x00; // VDJFLAG_VIDEO_OUTPUTRESOLUTION | VDJFLAG_VIDEO_OUTPUTASPECTRATIO;
	info->Version = "3.0 (64-bit)";

	return S_OK;
}
//------------------------------------------------------------------------------------------
ULONG VDJ_API CShakerSquare::Release()
{
	delete this;
	return 0;
}
//------------------------------------------------------------------------------------------
HRESULT VDJ_API CShakerSquare::OnParameter(int id)
{
	if (id == ID_INIT)
	{
		for (int i = ID_SLIDER_1; i <= ID_SLIDER_2; i++) OnSlider(i);
	}

	OnSlider(id);

	return S_OK;
}
//------------------------------------------------------------------------------------------
void CShakerSquare::OnSlider(int id)
{
	switch (id)
	{
		case ID_SLIDER_1:
			m_Alpha = m_SliderValue[0];
			break;

		case ID_SLIDER_2:
			m_Speed = (float)floor(m_SliderValue[1] * MAX_BEATS);
			break;
	}
}
//-------------------------------------------------------------------------------------------
HRESULT VDJ_API CShakerSquare::OnGetParameterString(int id, char* outParam, int outParamSize)
{
	switch (id)
	{
		case ID_SLIDER_1:
			sprintf_s(outParam, outParamSize, "%.0f%%", m_Alpha * 100);
			break;

		case ID_SLIDER_2:
			if (m_Speed == 0) sprintf_s(outParam, outParamSize, "Off");
			else sprintf_s(outParam, outParamSize, "%.0f beat(s)", m_Speed);
			break;
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------
HRESULT VDJ_API CShakerSquare::OnDeviceInit()
{
	HRESULT hr = S_FALSE;

	m_DirectX_On = true;
	m_WidthOnDeviceInit = width;
	m_HeightOnDeviceInit = height;
	m_Width = width;
	m_Height = height;

	hr = GetDevice(VdjVideoEngineDirectX11, (void**)  &pD3DDevice);
	if(hr!=S_OK || pD3DDevice==NULL) return E_FAIL;

	hr = Initialize_D3D11(pD3DDevice);

	return S_OK;
}
//-------------------------------------------------------------------------------------------
HRESULT VDJ_API CShakerSquare::OnDeviceClose()
{
	Release_D3D11();
	SAFE_RELEASE(pD3DRenderTargetView);
	SAFE_RELEASE(pD3DDeviceContext);
	m_DirectX_On = false;
	
	return S_OK;
}
//-------------------------------------------------------------------------------------------
HRESULT VDJ_API CShakerSquare::OnStart() 
{
	m_SavedSongPosBeats = SongPosBeats;
	return S_OK;
}
//-------------------------------------------------------------------------------------------
HRESULT VDJ_API CShakerSquare::OnStop() 
{
	m_SavedSongPosBeats = 0;
	return S_OK;
}
//-------------------------------------------------------------------------------------------
HRESULT VDJ_API CShakerSquare::OnDraw()
{
	HRESULT hr = S_FALSE;
	ID3D11ShaderResourceView *pTexture = nullptr;
	TVertex8* vertices = nullptr;

	if (m_Width != width || m_Height != height)
	{
		OnResizeVideo();
	}

	pD3DDevice->GetImmediateContext(&pD3DDeviceContext);
	if (!pD3DDeviceContext) return S_FALSE;

	pD3DDeviceContext->OMGetRenderTargets(1, &pD3DRenderTargetView, nullptr);
	if (!pD3DRenderTargetView) return S_FALSE;

	// We get current texture and vertices
	hr = GetTexture(VdjVideoEngineDirectX11, (void**)&pTexture, &vertices);
	if (hr != S_OK) return S_FALSE;

	hr = Rendering_D3D11(pD3DDevice, pD3DDeviceContext, pD3DRenderTargetView, pTexture, vertices);
	if (hr != S_OK) return S_FALSE;

	return S_OK;
}
//-----------------------------------------------------------------------
HRESULT VDJ_API CShakerSquare::OnAudioSamples(float* buffer, int nb)
{ 
	return E_NOTIMPL;
}
//-----------------------------------------------------------------------
void CShakerSquare::OnResizeVideo()
{
	m_Width = width;
	m_Height = height;
}
//-----------------------------------------------------------------------
HRESULT CShakerSquare::Initialize_D3D11(ID3D11Device* pDevice)
{
	HRESULT hr = S_FALSE;

	hr = Create_VertexBufferDynamic_D3D11(pDevice);
	if (hr != S_OK) return S_FALSE;

	hr = Create_PixelShader_D3D11(pDevice);
	if (hr != S_OK) return S_FALSE;

	hr = Create_BlendState_D3D11(pDevice);
	if (hr != S_OK) return S_FALSE;

	return S_OK;
}
//-----------------------------------------------------------------------
void CShakerSquare::Release_D3D11()
{
	SAFE_RELEASE(pNewVertexBuffer);
	SAFE_RELEASE(pPixelShader);
	SAFE_RELEASE(pBlendState);
}
// -----------------------------------------------------------------------
HRESULT CShakerSquare::Rendering_D3D11(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, ID3D11RenderTargetView* pRenderTargetView, ID3D11ShaderResourceView* pTextureView, TVertex8* pVertices)
{
	HRESULT hr = S_FALSE;

#ifdef _DEBUG
	InfoTexture2D InfoRTV = {};
	InfoTexture2D InfoSRV = {};
	hr = GetInfoFromRenderTargetView(pRenderTargetView, &InfoRTV);
	hr = GetInfoFromShaderResourceView(pTextureView, &InfoSRV);
#endif


	int AlphaMain = (int)((1.0f - m_Alpha) * 255.0f);
	pVertices[0].color = D3DCOLOR_RGBA(255, 255, 255, AlphaMain);
	pVertices[1].color = D3DCOLOR_RGBA(255, 255, 255, AlphaMain);
	pVertices[2].color = D3DCOLOR_RGBA(255, 255, 255, AlphaMain);
	pVertices[3].color = D3DCOLOR_RGBA(255, 255, 255, AlphaMain);

	hr = DrawDeck();
	if (hr != S_OK) return S_FALSE;



	if (pRenderTargetView)
	{
		//FLOAT backgroundColor[4] = { 0.0f, 0.0f , 0.0f , 1.0f };
		//pDeviceContext->ClearRenderTargetView(pRenderTargetView, backgroundColor);
		//pDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, nullptr);
	}

	if (m_SavedSongPosBeats != SongPosBeats)
	{ 
		m_PosVideo = ChangeVideoPosition();
		m_SavedSongPosBeats = SongPosBeats;
	}
	

	hr = Update_VertexBufferDynamic_D3D11(pDeviceContext);
	if (hr != S_OK) return S_FALSE;

	
	if (pPixelShader)
	{
		pDeviceContext->PSSetShader(pPixelShader, nullptr, 0);
	}
	
	if (pTextureView)
	{
		pDeviceContext->PSSetShaderResources(0, 1, &pTextureView);
	}

	if (pBlendState)
	{
		//pDeviceContext->OMSetBlendState(pBlendState, nullptr, 0xFFFFFFFF);
	}
	
	if (pNewVertexBuffer)
	{
		UINT m_VertexStride = sizeof(TLVERTEX);
		UINT m_VertexOffset = 0;
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		pDeviceContext->IASetVertexBuffers(0, 1, &pNewVertexBuffer, &m_VertexStride, &m_VertexOffset);
	}
	
	pDeviceContext->Draw(m_VertexCount, 0);
	
	return S_OK;
}

//-----------------------------------------------------------------------
int CShakerSquare::ChangeVideoPosition()
{
	int pos = 0;
	int pos_tmp = 0;
	double x = 0;

	if (m_Speed == 0) pos = 5;
	else
	{
		x = fmod(fabs(SongPosBeats), m_Speed) / m_Speed;
		pos = 1 + (int)(x * 5.0f);

		if (m_Inverted)
		{
			pos = 6 - pos;
		}

		// Ensure pos is within the range of 1 to 5 when m_Speed is set
		if (pos < 1) pos = 5;
		else if (pos > 5) pos = 1;
		
	}

	if (pos != m_PosVideo) return pos;
	else return m_PosVideo;
}
//-----------------------------------------------------------------------
HRESULT CShakerSquare::Create_VertexBufferDynamic_D3D11(ID3D11Device* pDevice)
{
	HRESULT hr = S_FALSE;

	if (!pDevice) return S_FALSE;

	// Set the number of vertices in the vertex array.
	m_VertexCount = 4; // = ARRAYSIZE(pNewVertices);
	
	// Fill in a buffer description.
	D3D11_BUFFER_DESC VertexBufferDesc;
	ZeroMemory(&VertexBufferDesc, sizeof(VertexBufferDesc));
	VertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;   // CPU_Access=Write_Only & GPU_Access=Read_Only
	VertexBufferDesc.ByteWidth = sizeof(TLVERTEX) * m_VertexCount;
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; //D3D11_BIND_INDEX_BUFFER
	VertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // Allow CPU to write in buffer
	VertexBufferDesc.MiscFlags = 0;

	hr = pDevice->CreateBuffer(&VertexBufferDesc, NULL, &pNewVertexBuffer);
	if (hr != S_OK || !pNewVertexBuffer) return S_FALSE;

	return S_OK;
}
//-----------------------------------------------------------------------
HRESULT CShakerSquare::Update_VertexBufferDynamic_D3D11(ID3D11DeviceContext* ctx)
{
	HRESULT hr = S_FALSE;

	if (!ctx) return S_FALSE;
	if (!pNewVertexBuffer) return S_FALSE;

	D3D11_MAPPED_SUBRESOURCE MappedSubResource;
	ZeroMemory(&MappedSubResource, sizeof(D3D11_MAPPED_SUBRESOURCE));


	hr = ctx->Map(pNewVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubResource);
	if (hr != S_OK) return S_FALSE;

	//TLVERTEX* pNewVertices = (TLVERTEX*) MappedSubResource.pData;

	hr = Update_Vertices_D3D11();

	memcpy(MappedSubResource.pData, pNewVertices, m_VertexCount * sizeof(TLVERTEX));

	ctx->Unmap(pNewVertexBuffer, NULL);

	return S_OK;
}
//-----------------------------------------------------------------------
HRESULT CShakerSquare::Update_Vertices_D3D11()
{
	float frameWidth = (float) m_Width;
	float frameHeight = (float) m_Height;

	D3DXPOSITION P1 = { 0.0f , 0.0f, 0.0f }, // Top Left
		P2 = { 0.0f, frameHeight, 0.0f }, // Bottom Left
		P3 = { frameWidth, 0.0f, 0.0f }, // Top Right
		P4 = { frameWidth, frameHeight, 0.0f }, // Bottom Right
		P5 = { frameWidth / 2.0f, frameHeight / 2.0f, 0.0f }, // Center Center
		P6 = { frameWidth / 2.0f, 0.0f, 0.0f }, // Top Center
		P7 = { frameWidth, frameHeight / 2.0f, 0.0f }, // Center Right
		P8 = { frameWidth / 2.0f, frameHeight, 0.0f }, // Bottom Center
	    P9 = { 0.0f, frameHeight / 2.0f, 0.0f }, // Center Left
		P10 = { frameWidth / 4.0f, frameHeight / 4.0f, 0.0f }, // Center Top Left
		P11 = { frameWidth / 4.0f, frameHeight * 3.0f / 4.0f, 0.0f }, // Center Bottom Left
		P12 = { frameWidth * 3.0f / 4.0f, frameHeight / 4.0f, 0.0f }, // Center Top Right
		P13 = { frameWidth * 3.0f / 4.0f, frameHeight * 3.0f / 4.0f, 0.0f }; // Center Bottom Right

	D3DXCOLOR color_vertex = D3DXCOLOR(1.0f, 1.0f, 1.0f, m_Alpha); // White color with alpha layer
	D3DXTEXCOORD T1 = { 0.0f , 0.0f }, T2 = { 0.0f , 1.0f }, T3 = { 1.0f , 0.0f }, T4 = { 1.0f , 1.0f };

	switch (m_PosVideo)
	{
		case 0: // center
			pNewVertices[0] = { P1, color_vertex, T1 };
			pNewVertices[1] = { P2, color_vertex, T2 };
			pNewVertices[2] = { P3, color_vertex, T3 };
			pNewVertices[3] = { P4, color_vertex, T4 };
			break;

		case 1: // top left
			pNewVertices[0] = { P1, color_vertex, T1 };
			pNewVertices[1] = { P9, color_vertex, T2 };
			pNewVertices[2] = { P6, color_vertex, T3 };
			pNewVertices[3] = { P5, color_vertex, T4 };
			break;

		case 2: // top right
			pNewVertices[0] = { P6, color_vertex, T1 };
			pNewVertices[1] = { P5, color_vertex, T2 };
			pNewVertices[2] = { P3, color_vertex, T3 };
			pNewVertices[3] = { P7, color_vertex, T4 };
			break;

		case 3: // bottom right
			pNewVertices[0] = { P5, color_vertex, T1 };
			pNewVertices[1] = { P8, color_vertex, T2 };
			pNewVertices[2] = { P7, color_vertex, T3 };
			pNewVertices[3] = { P4, color_vertex, T4 };
			break;

		case 4: // bottom left
			pNewVertices[0] = { P9, color_vertex, T1 };
			pNewVertices[1] = { P2, color_vertex, T2 };
			pNewVertices[2] = { P5, color_vertex, T3 };
			pNewVertices[3] = { P8, color_vertex, T4 };
			break;

		case 5: // center
			pNewVertices[0] = { P10, color_vertex, T1 };
			pNewVertices[1] = { P11, color_vertex, T2 };
			pNewVertices[2] = { P12, color_vertex, T3 };
			pNewVertices[3] = { P13, color_vertex, T4 };
			break;
	}

	return S_OK;
}
//-----------------------------------------------------------------------
HRESULT CShakerSquare::Create_PixelShader_D3D11(ID3D11Device* pDevice)
{
	HRESULT hr = S_FALSE;
	const WCHAR* pShaderHLSLFilepath = L"PixelShader.hlsl";
	const WCHAR* pShaderCSOFilepath = L"PixelShader.cso";
	const WCHAR* resourceType = RT_RCDATA;
	const WCHAR* resourceName = L"PIXELSHADER_CSO";

	SAFE_RELEASE(pPixelShader);

	hr = Create_PixelShaderFromResourceCSOFile_D3D11(pDevice, resourceType, resourceName);

	return hr;
}
//-----------------------------------------------------------------------
HRESULT CShakerSquare::Create_PixelShaderFromResourceCSOFile_D3D11(ID3D11Device* pDevice, const WCHAR* resourceType, const WCHAR* resourceName)
{
	HRESULT hr = S_FALSE;
	
	void* pShaderBytecode = nullptr;
	SIZE_T BytecodeLength = 0;

	hr = ReadResource(resourceType, resourceName, &BytecodeLength, &pShaderBytecode);
	if (hr != S_OK) return S_FALSE;
	
	hr = pDevice->CreatePixelShader(pShaderBytecode, BytecodeLength, nullptr, &pPixelShader);

	return hr;
}
//-----------------------------------------------------------------------
HRESULT CShakerSquare::ReadResource(const WCHAR* resourceType, const WCHAR* resourceName, SIZE_T* size, LPVOID* data)
{
	HRESULT hr = S_FALSE;

	HRSRC rc = FindResource(hInstance, resourceName, resourceType);
	if (!rc) return S_FALSE;

	HGLOBAL rcData = LoadResource(hInstance, rc);
	if (!rcData) return S_FALSE;

	*size = (SIZE_T)SizeofResource(hInstance, rc);
	if (*size == 0) return S_FALSE;

	*data = LockResource(rcData);
	if (*data == nullptr) return S_FALSE;

	return S_OK;
}
//-----------------------------------------------------------------------
HRESULT CShakerSquare::Create_BlendState_D3D11(ID3D11Device* pDevice)
{
	HRESULT hr = S_FALSE;

	D3D11_RENDER_TARGET_BLEND_DESC RenderTargetBlendDesc;
	ZeroMemory(&RenderTargetBlendDesc, sizeof(D3D11_RENDER_TARGET_BLEND_DESC));
	RenderTargetBlendDesc.BlendEnable = TRUE;
	RenderTargetBlendDesc.SrcBlend = D3D11_BLEND_SRC_COLOR; // The data source is color data (RGB) from a pixel shader. No pre-blend operation.
	RenderTargetBlendDesc.DestBlend = D3D11_BLEND_DEST_COLOR; // The data source is color data from a rendertarget. No pre-blend operation.
	RenderTargetBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
	RenderTargetBlendDesc.SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA; // The data source is alpha data from a pixel shader. No pre-blend operation.
	RenderTargetBlendDesc.DestBlendAlpha = D3D11_BLEND_DEST_ALPHA; // The data source is alpha data from a rendertarget. No pre-blend operation. 
	RenderTargetBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	RenderTargetBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALPHA; // D3D11_COLOR_WRITE_ENABLE_ALL


	D3D11_BLEND_DESC BlendStateDesc;
	ZeroMemory(&BlendStateDesc, sizeof(D3D11_BLEND_DESC));
	BlendStateDesc.AlphaToCoverageEnable = FALSE;
	BlendStateDesc.IndependentBlendEnable = FALSE;
	BlendStateDesc.RenderTarget[0] = RenderTargetBlendDesc;

	hr = pDevice->CreateBlendState(&BlendStateDesc, &pBlendState);

	return hr;
}
//-----------------------------------------------------------------------
HRESULT CShakerSquare::GetInfoFromShaderResourceView(ID3D11ShaderResourceView* pShaderResourceView, InfoTexture2D* info)
{
	HRESULT hr = S_FALSE;

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	ZeroMemory(&viewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

	pShaderResourceView->GetDesc(&viewDesc);

	DXGI_FORMAT ViewFormat = viewDesc.Format;
	D3D11_SRV_DIMENSION ViewDimension = viewDesc.ViewDimension;

	ID3D11Resource* pResource = nullptr;
	pShaderResourceView->GetResource(&pResource);
	if (!pResource) return S_FALSE;

	if (ViewDimension == D3D11_SRV_DIMENSION_TEXTURE2D)
	{
		ID3D11Texture2D* pTexture = nullptr;
		hr = pResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&pTexture);
		if (hr != S_OK || !pTexture) return S_FALSE;

		D3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));

		pTexture->GetDesc(&textureDesc);

		info->Format = textureDesc.Format;
		info->Width = textureDesc.Width;
		info->Height = textureDesc.Height;

		SAFE_RELEASE(pTexture);
	}

	SAFE_RELEASE(pResource);

	return S_OK;
}
//-----------------------------------------------------------------------
HRESULT CShakerSquare::GetInfoFromRenderTargetView(ID3D11RenderTargetView* pRenderTargetView, InfoTexture2D* info)
{
	HRESULT hr = S_FALSE;

	D3D11_RENDER_TARGET_VIEW_DESC viewDesc;
	ZeroMemory(&viewDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));

	pRenderTargetView->GetDesc(&viewDesc);

	DXGI_FORMAT ViewFormat = viewDesc.Format;
	D3D11_RTV_DIMENSION ViewDimension = viewDesc.ViewDimension;

	ID3D11Resource* pResource = nullptr;
	pRenderTargetView->GetResource(&pResource);
	if (!pResource) return S_FALSE;

	if (ViewDimension == D3D11_RTV_DIMENSION_TEXTURE2D)
	{
		ID3D11Texture2D* pTexture = nullptr;
		hr = pResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&pTexture);
		if (hr != S_OK || !pTexture) return S_FALSE;

		D3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));

		pTexture->GetDesc(&textureDesc);

		info->Format = textureDesc.Format;
		info->Width = textureDesc.Width;
		info->Height = textureDesc.Height;

		SAFE_RELEASE(pTexture);
	}

	SAFE_RELEASE(pResource);

	return S_OK;
}