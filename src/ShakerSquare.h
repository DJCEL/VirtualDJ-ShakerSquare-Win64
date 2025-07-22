#ifndef SHAKERSQUARE_H
#define SHAKERSQUARE_H


#include "vdjVideo8.h"
#include <stdio.h>
#include <d3d11.h>
#include <math.h> // for the function floor()

#pragma comment(lib, "d3d11.lib")


//////////////////////////////////////////////////////////////////////////
// Class definition
//////////////////////////////////////////////////////////////////////////
class CShakerSquare : public IVdjPluginVideoFx8
{
public:
	CShakerSquare();
	~CShakerSquare();
	HRESULT VDJ_API OnLoad();
	HRESULT VDJ_API OnGetPluginInfo(TVdjPluginInfo8 *info);
	ULONG   VDJ_API Release();
	HRESULT VDJ_API OnParameter(int id);
	HRESULT VDJ_API OnGetParameterString(int id, char* outParam, int outParamSize);
	HRESULT VDJ_API OnDeviceInit();
	HRESULT VDJ_API OnDeviceClose();
	HRESULT VDJ_API OnDraw();
	HRESULT VDJ_API OnStart();
	HRESULT VDJ_API OnStop();
	HRESULT VDJ_API OnAudioSamples(float* buffer, int nb);

private:
	struct D3DXPOSITION
	{
		float x;
		float y;
		float z;
	};
	struct D3DXCOLOR
	{
	public:
		D3DXCOLOR() = default;
		D3DXCOLOR(FLOAT r, FLOAT g, FLOAT b, FLOAT a)
		{
			this->r = r;
			this->g = g;
			this->b = b;
			this->a = a;
		}

		operator FLOAT* ()
		{
			return &r;
		}

		FLOAT r, g, b, a;
	};
	struct D3DXTEXCOORD
	{
		float tu;
		float tv;
	};

	struct TLVERTEX
	{
		D3DXPOSITION position;
		D3DXCOLOR color;
		D3DXTEXCOORD texture;
	};

	struct InfoTexture2D
	{
		UINT Width;
		UINT Height;
		DXGI_FORMAT Format;
	};

	void OnResizeVideo();
	void OnSlider(int id);
	HRESULT ReadResource(const WCHAR* resourceType, const WCHAR* resourceName, SIZE_T* size, LPVOID* data);
	int ChangeVideoPosition();

	HRESULT Initialize_D3D11(ID3D11Device* pDevice);
	void Release_D3D11();
	HRESULT Rendering_D3D11(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, ID3D11RenderTargetView* pRenderTargetView, ID3D11ShaderResourceView* pTextureView, TVertex8* pVertices);
	HRESULT Create_PixelShader_D3D11(ID3D11Device* pDevice);
	HRESULT Create_PixelShaderFromResourceCSOFile_D3D11(ID3D11Device* pDevice, const WCHAR* resourceType, const WCHAR* resourceName);
	HRESULT Create_VertexBufferDynamic_D3D11(ID3D11Device* pDevice);
	HRESULT Update_VertexBufferDynamic_D3D11(ID3D11DeviceContext* ctx);
	HRESULT Update_Vertices_D3D11();
	HRESULT Create_BlendState_D3D11(ID3D11Device* pDevice);
	HRESULT GetInfoFromShaderResourceView(ID3D11ShaderResourceView* pShaderResourceView, InfoTexture2D* info);
	HRESULT GetInfoFromRenderTargetView(ID3D11RenderTargetView* pRenderTargetView, InfoTexture2D* info);

	
	ID3D11Device* pD3DDevice;
	ID3D11DeviceContext* pD3DDeviceContext;
	ID3D11RenderTargetView* pD3DRenderTargetView;
	ID3D11Buffer* pNewVertexBuffer;
	ID3D11PixelShader* pPixelShader;
	ID3D11BlendState* pBlendState;
	
	TLVERTEX pNewVertices[6];
	UINT m_VertexCount;
	bool m_DirectX_On;
	int m_WidthOnDeviceInit;
	int m_HeightOnDeviceInit;
	int m_Width;
	int m_Height;
	float m_SliderValue[2];
	float m_Alpha;
	float m_Speed;
	int m_PosVideo;
	int m_Inverted;
	double m_SavedSongPosBeats;

	const float MAX_BEATS = 16.0f;

	typedef enum _ID_Interface
	{
		ID_INIT,
		ID_SLIDER_1,
		ID_SLIDER_2,
		ID_SWITCH_1
	} ID_Interface;

	#ifndef SAFE_RELEASE
	#define SAFE_RELEASE(x) { if (x!=nullptr) { x->Release(); x=nullptr; } }
	#endif

	typedef DWORD D3DCOLOR;
	#define D3DCOLOR_RGBA(r,g,b,a) ((D3DCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
	#define D3DCOLOR_GETRED(c) ((BYTE)(((c)>>16)&0xff))
	#define D3DCOLOR_GETGREEN(c) ((BYTE)(((c)>>8)&0xff))
	#define D3DCOLOR_GETBLUE(c) ((BYTE)(((c))&0xff))
	#define D3DCOLOR_GETALPHA(c) ((BYTE)(((c)>>24)&0xff))

};

#endif
