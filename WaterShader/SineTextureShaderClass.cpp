#include "SineTextureShaderClass.h"
#include <stdio.h>
#include <fstream>
#include <iostream>

SineTextureShaderClass::SineTextureShaderClass()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_matrixBuffer = 0;
	m_sineBuffer = 0;
	m_samplerState = 0;
}


SineTextureShaderClass::SineTextureShaderClass(const SineTextureShaderClass& other)
{
}


SineTextureShaderClass::~SineTextureShaderClass()
{
}

bool SineTextureShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;

	WCHAR vs[] = L"./data/shaders/SimpleSinVS.hlsl";
	WCHAR ps[] = L"./data/shaders/SimpleSinPS.hlsl";
	result = InitializeShader(device, hwnd, vs, ps);
	if (!result)
	{
		return false;
	}

	return true;
}

void SineTextureShaderClass::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	ShutdownShader();

	return;
}

bool SineTextureShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, DirectX::XMMATRIX worldMatrix,
	DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)
{
	bool result;


	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture);
	if (!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, indexCount);

	return true;
}

bool SineTextureShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC sineBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;

	// Initialize the pointers this function will use to null.
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

	// Compile the vertex shader code.
	result = D3DCompileFromFile(vsFilename, NULL, NULL, "SimpleSinVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		else
		{
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// Compile the pixel shader code.
	result = D3DCompileFromFile(psFilename, NULL, NULL, "SimpleSinPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&pixelShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		else
		{
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	if (FAILED(result))
	{
		return false;
	}

	// Create the vertex shader from the buffer.
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
	{
		return false;
	}

	// Create the pixel shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result))
	{
		return false;
	}

	// Now setup the layout of the data that goes into the shader.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "NORMAL";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	polygonLayout[2].SemanticName = "TEXCOORD";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[2].InstanceDataStepRate = 0;

	// Get a count of the elements in the layout.
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
	{
		return false;
	}

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result))
	{
		return false;
	}

	sineBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	sineBufferDesc.ByteWidth = sizeof(SineBufferType);
	sineBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	sineBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	sineBufferDesc.MiscFlags = 0;
	sineBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&sineBufferDesc, NULL, &m_sineBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	result = device->CreateSamplerState(&samplerDesc, &m_samplerState);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

void SineTextureShaderClass::ShutdownShader()
{
	if (m_samplerState)
	{
		m_samplerState->Release();
		m_samplerState = 0;
	}

	// Release the matrix constant buffer.
	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	if (m_sineBuffer)
	{
		m_sineBuffer->Release();
		m_sineBuffer = 0;
	}
	
	// Release the layout.
	if (m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	// Release the pixel shader.
	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	// Release the vertex shader.
	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}

	return;
}

void SineTextureShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long bufferSize, i;
	ofstream fout;

	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write out the error message.
	for (i = 0; i < bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}

bool SineTextureShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, DirectX::XMMATRIX worldMatrix,
	DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)
{
	cout << "HERE";

	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResourceM;
	D3D11_MAPPED_SUBRESOURCE mappedResourceS;
	MatrixBufferType* dataMPtr;
	SineBufferType* dataSPtr;

	// Transpose the matrices to prepare them for the shader.
	worldMatrix = DirectX::XMMatrixTranspose(worldMatrix);
	viewMatrix = DirectX::XMMatrixTranspose(viewMatrix);
	projectionMatrix = DirectX::XMMatrixTranspose(projectionMatrix);

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResourceM);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataMPtr = (MatrixBufferType*)mappedResourceM.pData;

	// Copy the matrices into the constant buffer.
	dataMPtr->world = worldMatrix;
	dataMPtr->view = viewMatrix;
	dataMPtr->projection = projectionMatrix;

	// Unlock the constant buffer.
	deviceContext->Unmap(m_matrixBuffer, 0);

	// Current time
	FILETIME ft_now;
	GetSystemTimeAsFileTime(&ft_now);
	int ll_now = (LONGLONG)ft_now.dwLowDateTime + ((LONGLONG)(ft_now.dwHighDateTime) << 32LL);
	float st = (ll_now / 10000) % 86400000LL;
	
	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(m_sineBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResourceS);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataSPtr = (SineBufferType*)mappedResourceS.pData;

	dataSPtr->commonConst =		DirectX::XMFLOAT4(0.0, 0.5, 1.0, 2.0);
	dataSPtr->waveHeights =		DirectX::XMFLOAT4(5.0, 3.5, 1.5, 1);
	dataSPtr->waveLengths =		DirectX::XMFLOAT4(10, 3, 5, 4);
	dataSPtr->waveOffset =		DirectX::XMFLOAT4(-0.5f, 0.2f, 0.45f, 0.0f);
	dataSPtr->waveSpeed =		DirectX::XMFLOAT4(2.5, 0.75, 1, 1.5);
	dataSPtr->waveDirx =		DirectX::XMFLOAT4(0.25, 0.0, -0.7, -0.8);
	dataSPtr->waveDiry =		DirectX::XMFLOAT4(0.0, 0.45, -0.7, 0.1);
	dataSPtr->Q =				DirectX::XMFLOAT4(0.1, 0.2, 0.5, 1);
	dataSPtr->K =				DirectX::XMFLOAT4(2, 3, 2, 3);
	dataSPtr->bumpSpeed =		DirectX::XMFLOAT4(0.031, 0.04, -0.03, 0.02);
	dataSPtr->piVector =		DirectX::XMFLOAT4(4.0, 1.57079632, 3.14159265, 6.28318530);
	dataSPtr->sin7 =			DirectX::XMFLOAT4(1, -0.16161616, 0.0083333, -0.00019841);
	dataSPtr->cos8 =			DirectX::XMFLOAT4(-0.5, 0.041666666, -0.0013888889, 0.000024801587);
	dataSPtr->frcFixup =		DirectX::XMFLOAT4(1.02, 0.003, 0, 0);
	dataSPtr->psCommonConst =	DirectX::XMFLOAT4(0, 0.5, 1, 0.25);
	dataSPtr->highlightColor =	DirectX::XMFLOAT4(0.8, 0.76, 0.62, 1);
	dataSPtr->waterColor =		DirectX::XMFLOAT4(0.50, 0.6, 0.7, 1);
	dataSPtr->time =			DirectX::XMFLOAT4(st, sin(st), st/1000.0, sin(st / 1000.0));

	// Unlock the constant buffer.
	deviceContext->Unmap(m_sineBuffer, 0);

	// Finally set the constant buffer in the vertex shader with the updated values.
	deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);

	// Finally set the constant buffer in the vertex shader with the updated values.
	deviceContext->VSSetConstantBuffers(1, 1, &m_sineBuffer);


	deviceContext->PSSetShaderResources(0, 1, &texture);

	return true;
}

void SineTextureShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_layout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	deviceContext->PSSetSamplers(0, 1, &m_samplerState);

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}

