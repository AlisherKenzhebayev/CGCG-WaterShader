#include "causticShaderClass.h"

CausticShaderClass::CausticShaderClass()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_matrixBuffer = 0;
	m_sineBuffer = 0;
	m_lightBuffer = 0;
	m_cameraBuffer = 0;

	m_samplerState = 0;
}


CausticShaderClass::CausticShaderClass(const CausticShaderClass& other)
{
}


CausticShaderClass::~CausticShaderClass()
{
}

bool CausticShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;

	WCHAR vs[] = L"./data/shaders/CausticVS.hlsl";
	WCHAR ps[] = L"./data/shaders/CausticPS.hlsl";
	result = InitializeShader(device, hwnd, vs, ps);
	if (!result)
	{
		return false;
	}

	return true;
}

void CausticShaderClass::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	ShutdownShader();

	return;
}

bool CausticShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, DirectX::XMMATRIX worldMatrix,
	DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix,
	ID3D11ShaderResourceView* sunTexture,
	DirectX::XMFLOAT3 lightDirection, DirectX::XMFLOAT4 ambientColor, DirectX::XMFLOAT4 diffuseColor,
	DirectX::XMFLOAT3 cameraPosition, DirectX::XMFLOAT4 specularColor, float specularPower)
{
	bool result;

	result = SetShaderParameters(deviceContext, worldMatrix, 
		viewMatrix, projectionMatrix,
		sunTexture,
		lightDirection, ambientColor, diffuseColor,
		cameraPosition, specularColor, specularPower);
	if (!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, indexCount);

	return true;
}

bool CausticShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;

	D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
	unsigned int numElements;

	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC sineBufferDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC reflectionBufferDesc;
	D3D11_BUFFER_DESC waterBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;

	D3D11_SAMPLER_DESC samplerDesc;

	// Initialize the pointers this function will use to null.
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

	// Compile the vertex shader code.
	result = D3DCompileFromFile(vsFilename, NULL, NULL, "CausticVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
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
	result = D3DCompileFromFile(psFilename, NULL, NULL, "CausticPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
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

	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Setup the description of the dynamic sine wave constant buffer
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

	// Setup the description of the dynamic light constant buffer
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Setup the description of the camera dynamic constant buffer that is in the vertex shader.
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;

	// Create the camera constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&cameraBufferDesc, NULL, &m_cameraBuffer);
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

void CausticShaderClass::ShutdownShader()
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

	if (m_lightBuffer)
	{
		m_lightBuffer->Release();
		m_lightBuffer = 0;
	}

	if (m_cameraBuffer)
	{
		m_cameraBuffer->Release();
		m_cameraBuffer = 0;
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

void CausticShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
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

bool CausticShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, DirectX::XMMATRIX worldMatrix,
	DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix,
	ID3D11ShaderResourceView* sunTexture,
	DirectX::XMFLOAT3 lightDirection, DirectX::XMFLOAT4 ambientColor, DirectX::XMFLOAT4 diffuseColor,
	DirectX::XMFLOAT3 cameraPosition, DirectX::XMFLOAT4 specularColor, float specularPower)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	MatrixBufferType* dataV0Ptr;
	SineBufferType* dataV1Ptr;
	CameraBufferType* dataV2Ptr;

	LightBufferType* dataP0Ptr;
	
	// Transpose the matrices to prepare them for the shader.
	worldMatrix = DirectX::XMMatrixTranspose(worldMatrix);
	viewMatrix = DirectX::XMMatrixTranspose(viewMatrix);
	projectionMatrix = DirectX::XMMatrixTranspose(projectionMatrix);

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataV0Ptr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataV0Ptr->world = worldMatrix;
	dataV0Ptr->view = viewMatrix;
	dataV0Ptr->projection = projectionMatrix;

	// Unlock the constant buffer.
	deviceContext->Unmap(m_matrixBuffer, 0);

	// Finally set the constant buffer in the vertex shader with the updated values.
	deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);

	// Current time
	FILETIME ft_now;
	GetSystemTimeAsFileTime(&ft_now);
	int ll_now = (LONGLONG)ft_now.dwLowDateTime + ((LONGLONG)(ft_now.dwHighDateTime) << 32LL);
	float st = (ll_now / 10000) % 86400000LL;

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(m_sineBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataV1Ptr = (SineBufferType*)mappedResource.pData;

	dataV1Ptr->commonConst = DirectX::XMFLOAT4(0.0, 0.5, 1.0, 2.0);
	dataV1Ptr->waveHeights = DirectX::XMFLOAT4(0.71, 0.42, 0.25, 0.15);
	dataV1Ptr->waveLengths = RandomizeWithTime(DirectX::XMFLOAT4(500, 250, 120, 60), st, rand() % 10);
	dataV1Ptr->waveSpeed = DirectX::XMFLOAT4(2.5, 0.75, 1, 3.5);
	dataV1Ptr->waveDirx = DirectX::XMFLOAT4(0.25, 0.0, -0.7, -0.8);
	dataV1Ptr->waveDiry = DirectX::XMFLOAT4(0.0, 0.15, -0.7, 0.1);
	dataV1Ptr->Q = DirectX::XMFLOAT4(0.2, 0.4, 0.2, 0.1);
	dataV1Ptr->K = DirectX::XMFLOAT4(2, 3, 2, 3);
	dataV1Ptr->bumpSpeed = DirectX::XMFLOAT4(0.031, 0.04, -0.03, 0.02);
	dataV1Ptr->piVector = DirectX::XMFLOAT4(4.0, 1.57079632, 3.14159265, 6.28318530);
	dataV1Ptr->psCommonConst = DirectX::XMFLOAT4(0, 0.5, 1, 0.25);
	dataV1Ptr->highlightColor = DirectX::XMFLOAT4(0.8, 0.76, 0.62, 1);
	dataV1Ptr->waterColor = DirectX::XMFLOAT4(0.50, 0.6, 0.7, 1);
	dataV1Ptr->time = DirectX::XMFLOAT4(st, sin(st), st / 1000.0, sin(st / 1000.0));

	// Unlock the constant buffer.
	deviceContext->Unmap(m_sineBuffer, 0);

	// Finally set the constant buffer in the vertex shader with the updated values.
	deviceContext->VSSetConstantBuffers(1, 1, &m_sineBuffer);

	// Lock the reflection constant buffer so it can be written to.
	result = deviceContext->Map(m_cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataV2Ptr = (CameraBufferType*)mappedResource.pData;

	// Copy the reflection matrix into the constant buffer.
	dataV2Ptr->cameraPosition = cameraPosition;
	dataV2Ptr->padding = 0.0f;

	// Unlock the constant buffer.
	deviceContext->Unmap(m_cameraBuffer, 0);

	// Finally set the reflection constant buffer in the vertex shader with the updated values.
	deviceContext->VSSetConstantBuffers(2, 1, &m_cameraBuffer);

	// Lock the light constant buffer so it can be written to.
	result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataP0Ptr = (LightBufferType*)mappedResource.pData;

	// Copy the lighting variables into the constant buffer.
	dataP0Ptr->ambientColor = ambientColor;
	dataP0Ptr->diffuseColor = diffuseColor;
	dataP0Ptr->lightDirection = lightDirection;
	dataP0Ptr->specularPower = specularPower;
	dataP0Ptr->specularColor = specularColor;

	// Unlock the constant buffer.
	deviceContext->Unmap(m_lightBuffer, 0);

	// Finally set the light constant buffer in the pixel shader with the updated values.
	deviceContext->PSSetConstantBuffers(0, 1, &m_lightBuffer);

	// Set the texture resources in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &sunTexture);
	
	return true;
}

void CausticShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
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

DirectX::XMFLOAT4 CausticShaderClass::RandomizeWithTime(DirectX::XMFLOAT4 original, float ct, float rand)
{
	// TODO: add randomized wave interpolation/fade-in
	return original;
}

