#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_

#include <d3d11.h>
#include <DirectXMath.h>

#include "textureclass.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

class ModelClass
{
private:
	struct VertexType
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 textureUV;
	};

public:
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();

	bool Initialize(ID3D11Device*, std::string, WCHAR*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();

	ID3D11ShaderResourceView* GetTexture();

private:
	bool InitializeBuffers(ID3D11Device*, std::string);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadTexture(ID3D11Device*, WCHAR*);
	void ReleaseTexture();

private:
	ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
	int m_vertexCount, m_indexCount;
	TextureClass* m_Texture;
};

#endif