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
	
	struct LightBufferType
	{
		DirectX::XMFLOAT4 diffuseColor;
		DirectX::XMFLOAT3 lightDirection;
		float padding;
	};

	struct SineBufferType 
	{
		DirectX::XMFLOAT4 commonConst; // (0.0, 0.5, 1.0, 2.0)
		//heights for waves 4 different fronts
		DirectX::XMFLOAT4 waveHeights; //(80.0, 100.0, 5.0, 5.0)
		//lengths for waves 4 different fronts
		DirectX::XMFLOAT4 waveLengths; //(20.0, 40.0, 5.0, 2.0)
		//freqency of the waves (e.g. waves per unit time..)
		DirectX::XMFLOAT4 waveSpeed;// (0.2, 0.15, 0.4, 0.4)
		//diection of waves in tangent space (also controls frequency in space)
		DirectX::XMFLOAT4 waveDirx;// (0.25, 0.0, -0.7, -0.8)
		DirectX::XMFLOAT4 waveDiry;// (0.0, 0.15, -0.7, 0.1)
		
		DirectX::XMFLOAT4 Q;
		DirectX::XMFLOAT4 K;

		//bump map scroll speed
		DirectX::XMFLOAT4 bumpSpeed;// (0.031, 0.04, -0.03, 0.02)
		DirectX::XMFLOAT4 piVector;// (4.0, 1.57079632, 3.14159265, 6.28318530)
		
		DirectX::XMFLOAT4 psCommonConst;// (0, 0.5, 1, 0.25)
		DirectX::XMFLOAT4 highlightColor;// (0.8, 0.76, 0.62, 1)
		DirectX::XMFLOAT4 waterColor;// (0.50, 0.6, 0.7, 1)
		DirectX::XMFLOAT4 time; // (t, sin(t), t, sin(t))
	};

public:
	SineTextureShaderClass();
	SineTextureShaderClass(const SineTextureShaderClass&);
	~SineTextureShaderClass();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, DirectX::XMMATRIX, DirectX::XMMATRIX, DirectX::XMMATRIX, 
		ID3D11ShaderResourceView*, DirectX::XMFLOAT3, DirectX::XMFLOAT4);

private:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, DirectX::XMMATRIX worldMatrix, DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix, 
		ID3D11ShaderResourceView* texture, DirectX::XMFLOAT3 lightDirection, DirectX::XMFLOAT4 diffuseColor);

	bool SetShaderParameters(ID3D11DeviceContext*, DirectX::XMMATRIX, DirectX::XMMATRIX, DirectX::XMMATRIX);
	void RenderShader(ID3D11DeviceContext*, int);

	DirectX::XMFLOAT4 RandomizeWithTime(DirectX::XMFLOAT4 original, float ct, float rand);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_sineBuffer;
	ID3D11Buffer* m_lightBuffer;

	ID3D11SamplerState* m_samplerState;
};

#endif
