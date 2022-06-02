#include "modelclass.h"

#ifdef _DEBUG
	#include <stdio.h>
	#include <iostream>
	#include <fstream>
#endif

//TODO: Probably storing the models[] and exchanging the VBO each time the ith object is requested

ModelClass::ModelClass()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	m_Texture = 0;
	m_Model = 0;
	m_Indices = 0;
}

ModelClass::ModelClass(const ModelClass& other)
{
}

ModelClass::~ModelClass()
{
}

bool ModelClass::Initialize(ID3D11Device * device, std::string fileNameModel, WCHAR* fileNameTexture)
{
	bool result;

	// Load the model mesh 
	result = LoadModel(fileNameModel);
	if (!result)
	{
		return false;
	}

	// Initialize the vertex and index buffer that hold the geometry for the triangle.
	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}

	// Load the model texture
	result = LoadTexture(device, fileNameTexture);
	if (!result) 
	{
		return false;
	}

	return true;
}

void ModelClass::Shutdown()
{
	ShutdownBuffers();

	ReleaseTexture();

	ReleaseModel();

	return;
}

void ModelClass::Render(ID3D11DeviceContext * deviceContext)
{
	RenderBuffers(deviceContext);
	return;
}

int ModelClass::GetIndexCount()
{
	return m_indexCount;
}

ID3D11ShaderResourceView* ModelClass::GetTexture()
{
	return m_Texture->GetTexture();
}


bool ModelClass::InitializeBuffers(ID3D11Device* device)	// TODO: , int indexToLoad)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int i;

	//m_vertexCount = 3;
	//m_indexCount = 3;

	// Create the vertex array.
	vertices = new VertexType[m_vertexCount];
	if (!vertices)
	{
		return false;
	}

	// Create the index array.
	indices = new unsigned long[m_indexCount];
	if (!indices)
	{
		return false;
	}

	//vertices[0].position = DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f);  // Bottom left.
	//vertices[0].textureUV = DirectX::XMFLOAT2(0.0f, 1.0f);

	//vertices[1].position = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);  // Top middle.
	//vertices[1].textureUV = DirectX::XMFLOAT2(0.5f, 0.0f);

	//vertices[2].position = DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f);  // Bottom right.
	//vertices[2].textureUV = DirectX::XMFLOAT2(1.0f, 1.0f);

	//// Load the index array with data.
	//indices[0] = 0;  // Bottom left.
	//indices[1] = 1;  // Top middle.
	//indices[2] = 2;  // Bottom right.

	for (i = 0; i < m_vertexCount; i++)
	{
		vertices[i].position = m_Model[i].position;
		vertices[i].normal = m_Model[i].normal;
		vertices[i].textureUV = m_Model[i].textureUV;
	}

	for (i = 0; i < m_indexCount; i++)
	{
		indices[i] = m_Indices[i].v0;
	}

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return true;
}

bool ModelClass::LoadTexture(ID3D11Device* device, WCHAR* filename)
{
	bool result;

	// Create the texture object.
	m_Texture = new TextureClass;
	if (!m_Texture)
	{
		return false;
	}

	// Initialize the texture object.
	result = m_Texture->Initialize(device, filename);
	if (!result)
	{
		return false;
	}

	return true;
}

bool ModelClass::LoadModel(std::string fileNameModel) {
	int vertexCount = 0;
	float scale = 1;

	Assimp::Importer importer;

	const aiScene* model = importer.ReadFile(fileNameModel,
		aiProcess_Triangulate
		| aiProcess_JoinIdenticalVertices);
	
	if (model == NULL)
	{
#ifdef _DEBUG
		std::ofstream fout;
		fout.open("model-error.txt");
		fout << "Model Error " + (std::string)
			importer.GetErrorString() + " in file " + fileNameModel;
		fout.close();
#endif
		return false;
	}

	if (model->mNumMeshes == 0) {
		return false;
	}

	//for(int i = 0; i < model->mNumMeshes; i++)
	{
		int i = 0;
		const auto mesh = model->mMeshes[i];
		//TODO: somehow extend to multiple objects support 
		//TODO: separate indices from vertices
		m_vertexCount = mesh->mNumVertices;
		m_indexCount = mesh->mNumFaces*3;
		
		m_Model = new ModelType[m_vertexCount];
		m_Indices = new IndiceType[m_indexCount];
		
		if (!m_Model)
		{
			return false;
		}

		for (unsigned int v = 0; v < m_vertexCount; v++)
		{
			const auto& vertex = mesh->mVertices[v];
			const auto& normal = mesh->mNormals[v];
			const auto& texUV = mesh->mTextureCoords[0][v];
			m_Model[v] = {
				{vertex.x * scale, vertex.y * scale, vertex.z * scale},
				{normal.x, normal.y, normal.z},
				{texUV.x, texUV.y}
			};
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			const auto& face = mesh->mFaces[i];
			assert(face.mNumIndices == 3);
			m_Indices[3 * i] = { face.mIndices[0] };
			m_Indices[3 * i + 1] = { face.mIndices[1] };
			m_Indices[3 * i + 2] = { face.mIndices[2] };
		}

		// Checking the data is being read and transferred
#ifdef _DEBUG
		std::ofstream fout;
		fout.open("model-error.txt");
		fout << "Model DATA " + std::to_string(m_vertexCount) + " " + std::to_string(mesh->mNumFaces);
		for (unsigned int v = 0; v < m_vertexCount; v++)
		{
			fout << "\n " + std::to_string(m_Model[v].position.x)
				+ " " + std::to_string(m_Model[v].position.y)
				+ " " + std::to_string(m_Model[v].position.z);
			if(mesh->mTextureCoords[0][v].x && mesh->mTextureCoords[0][v].y){
				fout << " " + std::to_string(mesh->mTextureCoords[0][v].x) + " " + std::to_string(mesh->mTextureCoords[0][v].y);
			}

		}

		for (unsigned int v = 0; v < m_indexCount; v+=3)
		{
			fout << " " + std::to_string(m_Indices[v].v0)
				+ " " + std::to_string(m_Indices[v + 1].v0)
				+ " " + std::to_string(m_Indices[v + 2].v0) + "\n";
		}
		fout.close();
#endif
	}

	return true;
}

void ModelClass::ReleaseTexture()
{
	// Release the texture object.
	if (m_Texture)
	{
		m_Texture->Shutdown();
		delete m_Texture;
		m_Texture = 0;
	}

	return;
}


void ModelClass::ReleaseModel()
{
	if (m_Model)
	{
		delete[] m_Model;
		m_Model = 0;
	}

	if (m_Indices)
	{
		delete[] m_Indices;
		m_Indices = 0;
	}

	return;
}

void ModelClass::ShutdownBuffers()
{
	// Release the index buffer.
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// Release the vertex buffer.
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	return;
}

void ModelClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;


	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}
