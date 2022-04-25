#ifndef _COLORSHADERCLASS_H_
#define _COLORSHADERCLASS_H_


#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <fstream>
using namespace std;


class SineTextureShaderClass
{
private:

	struct MatrixBufferType
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
	};

	struct SineBufferType 
	{
		DirectX::XMFLOAT4 commonConst; // (0.0, 0.5, 1.0, 2.0)
		//heights for waves 4 different fronts
		DirectX::XMFLOAT4 waveHeights; //(80.0, 100.0, 5.0, 5.0)
		//lengths for waves 4 different fronts
		DirectX::XMFLOAT4 waveLengths; //(20.0, 40.0, 5.0, 2.0)
		//offset in sine wave.. (ranges 0 to 1)
		DirectX::XMFLOAT4 waveOffset;// (0.0, 0.2, 0.0, 0.0)
		//freqency of the waves (e.g. waves per unit time..)
		DirectX::XMFLOAT4 waveSpeed;// (0.2, 0.15, 0.4, 0.4)
		//diection of waves in tangent space (also controls frequency in space)
		DirectX::XMFLOAT4 waveDirx;// (0.25, 0.0, -0.7, -0.8)
		DirectX::XMFLOAT4 waveDiry;// (0.0, 0.15, -0.7, 0.1)
		//scale factor for distortion of base map coords
		//bump map scroll speed
		DirectX::XMFLOAT4 bumpSpeed;// (0.031, 0.04, -0.03, 0.02)
		DirectX::XMFLOAT4 piVector;// (4.0, 1.57079632, 3.14159265, 6.28318530)
		//Vectors for taylor's series expansion of sin and cos
		DirectX::XMFLOAT4 sin7;// (1, -0.16161616, 0.0083333, -0.00019841)
		DirectX::XMFLOAT4 cos8;// (-0.5, 0.041666666, -0.0013888889, 0.000024801587)
		//frcFixup.x is a fixup to make the edges of the clamped sin wave match up again due to numerical inaccuracy
		//frcFixup.y should be equal to the average of du/dx and dv/dy for the base texture coords.. this scales the warping of the normal
		DirectX::XMFLOAT4 frcFixup;// (1.02, 0.003, 0, 0)
		DirectX::XMFLOAT4 psCommonConst;// (0, 0.5, 1, 0.25)
		DirectX::XMFLOAT4 highlightColor;// (0.8, 0.76, 0.62, 1)
		DirectX::XMFLOAT4 waterColor;// (0.50, 0.6, 0.7, 1)
		DirectX::XMFLOAT2 time; // (t, sin(t))
	};

public:
	SineTextureShaderClass();
	SineTextureShaderClass(const SineTextureShaderClass&);
	~SineTextureShaderClass();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, DirectX::XMMATRIX, DirectX::XMMATRIX, DirectX::XMMATRIX, ID3D11ShaderResourceView*);

private:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, DirectX::XMMATRIX worldMatrix, DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture);

	bool SetShaderParameters(ID3D11DeviceContext*, DirectX::XMMATRIX, DirectX::XMMATRIX, DirectX::XMMATRIX);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_sineBuffer;

	ID3D11SamplerState* m_samplerState;
};

#endif
