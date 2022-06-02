////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "graphicsclass.h"

GraphicsClass::GraphicsClass()
{
	m_Input = 0;
	m_D3D = 0;
	m_Camera = 0;
	m_Shader = 0;
	m_Timer = 0;
	m_Light = 0;
	m_Position = 0;

	m_GroundModel = 0;
	m_WallModel = 0;
	m_BathModel = 0;
	m_WaterModel = 0;
	m_CausticGround = 0;
	m_RefractionTexture = 0;
	m_ReflectionTexture = 0;
	m_RefractionShader = 0;
	m_LightShader = 0;

	m_SunTexture = 0;
}


GraphicsClass::GraphicsClass(const GraphicsClass& other)
{
}


GraphicsClass::~GraphicsClass()
{
}


bool GraphicsClass::Initialize(HINSTANCE hInstance, HWND hwnd, int screenWidth, int screenHeight)
{
	bool result;
	float cameraX, cameraY, cameraZ;
	float cameraRotX, cameraRotY, cameraRotZ;

	m_Input = new InputClass;
	if (!m_Input)
	{
		return false;
	}
	m_Input->Initialize(hInstance, hwnd, screenWidth, screenHeight);

	m_D3D = new D3DClass;
	if (!m_D3D) {
		return false;
	}

	result = m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result) {
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}

	// Create the camera object.
	m_Camera = new CameraClass;
	if (!m_Camera)
	{
		return false;
	}

	// Set the initial position of the camera.
	cameraX = 0.0f;
	cameraY = 1.0f;
	cameraZ = -50.0f;

	cameraRotX = 0.0f;
	cameraRotY = 0.0f;
	cameraRotZ = 0.0f;

	m_Camera->SetPosition(cameraX, cameraY, cameraZ);

	// Create the water model object.
	m_WaterModel = new ModelClass;
	if (!m_WaterModel)
	{
		return false;
	}

	result = m_WaterModel->Initialize(m_D3D->GetDevice(), "./data/objects/water3.obj", (WCHAR*)L"./data/textures/rastertek/water01.dds");
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the water model object.", L"Error", MB_OK);
		return false;
	}

	// Create the bath model object.
	m_BathModel = new ModelClass;
	if (!m_BathModel)
	{
		return false;
	}

	result = m_BathModel->Initialize(m_D3D->GetDevice(), "./data/objects/bath2.obj", (WCHAR*)L"./data/textures/rastertek/marble01.dds");
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the bath model object.", L"Error", MB_OK);
		return false;
	}

	m_CausticGround = new ModelClass;
	if (!m_CausticGround)
	{
		return false;
	}

	result = m_CausticGround->Initialize(m_D3D->GetDevice(), "./data/objects/groundPlane.obj", (WCHAR*)L"./data/textures/test.dds");
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the caustic model object.", L"Error", MB_OK);
		return false;
	}

	// Create the ground model object.
	m_GroundModel = new ModelClass;
	if (!m_GroundModel)
	{
		return false;
	}

	result = m_GroundModel->Initialize(m_D3D->GetDevice(), "./data/objects/ground.obj", (WCHAR*)L"./data/textures/rastertek/ground01.dds");
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the ground model object.", L"Error", MB_OK);
		return false;
	}

	// Create the wall model object.
	m_WallModel = new ModelClass;
	if (!m_WallModel)
	{
		return false;
	}

	result = m_WallModel->Initialize(m_D3D->GetDevice(), "./data/objects/wall.obj", (WCHAR*)L"./data/textures/rastertek/wall01.dds");
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the wall model object.", L"Error", MB_OK);
		return false;
	}

	// Check for correct loading of the texture
#ifdef _DEBUG
	ID3D11Resource* res;
	m_WaterModel->GetTexture()->GetResource(&res);
	HRESULT status = DirectX::SaveDDSTextureToFile(m_D3D->GetDeviceContext(), res, L"./data/SCREENSHOT.dds");
#endif

	// Create the shader object.
	m_Shader = new SineTextureShaderClass;
	if (!m_Shader)
	{
		return false;
	}

	result = m_Shader->Initialize(m_D3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the shader object.", L"Error", MB_OK);
		return false;
	}

	m_Timer = new TimerClass;
	if (!m_Timer)
	{
		return false;
	}

	// Initialize the timer object.
	result = m_Timer->Initialize();
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the timer object.", L"Error", MB_OK);
		return false;
	}

	// Create the light object.
	m_Light = new LightClass;
	if (!m_Light)
	{
		return false;
	}
	
	// Initialize the light object.
	m_Light->SetAmbientColor(0.25f, 0.25f, 0.25f, 1.0f);
	//m_Light->SetDiffuseColor(1.0f, 0.35f, 0.32f, 1.0f);
	m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light->SetDirection(1.0f, -1.0f, 1.0f);
	m_Light->SetSpecularColor(1.0f, 0.85f, 0.64f, 1.0f);
	m_Light->SetSpecularPower(32.0f);

	// Create the refraction render to texture object.
	m_RefractionTexture = new RenderTextureClass;
	if (!m_RefractionTexture)
	{
		return false;
	}

	// Initialize the refraction render to texture object.
	result = m_RefractionTexture->Initialize(m_D3D->GetDevice(), screenWidth, screenHeight);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the refraction render to texture object.", L"Error", MB_OK);
		return false;
	}

	// Create the reflection render to texture object.
	m_ReflectionTexture = new RenderTextureClass;
	if (!m_ReflectionTexture)
	{
		return false;
	}

	// Initialize the reflection render to texture object.
	result = m_ReflectionTexture->Initialize(m_D3D->GetDevice(), screenWidth, screenHeight);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the reflection render to texture object.", L"Error", MB_OK);
		return false;
	}

	m_LightShader = new TextureShaderClass;
	if (!m_LightShader)
	{
		return false;
	}

	result = m_LightShader->Initialize(m_D3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the light shader object.", L"Error", MB_OK);
		return false;
	}


	// Create the refraction shader object.
	m_RefractionShader = new RefractionShaderClass;
	if (!m_RefractionShader)
	{
		return false;
	}

	// Initialize the refraction shader object.
	result = m_RefractionShader->Initialize(m_D3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the refraction shader object.", L"Error", MB_OK);
		return false;
	}

	// Set the height of the water.
	m_waterHeight = 10.0f;

	// Initialize the position of the water.
	m_waterTranslation = 0.0f;

	m_Position = new PositionClass;
	if (!m_Position)
	{
		return false;
	}

	// Set the initial position of the viewer to the same as the initial camera position.
	m_Position->SetPosition(cameraX, cameraY, cameraZ);
	m_Position->SetRotation(cameraRotX, cameraRotY, cameraRotZ);


	// Create the shader object.
	m_CausticShader = new CausticShaderClass;
	if (!m_CausticShader)
	{
		return false;
	}

	result = m_CausticShader->Initialize(m_D3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the caustic shader object.", L"Error", MB_OK);
		return false;
	}

	// Create the texture object.
	m_SunTexture = new TextureClass;
	if (!m_SunTexture)
	{
		return false;
	}

	// Initialize the texture object.
	result = m_SunTexture->Initialize(m_D3D->GetDevice(), (WCHAR*)L"./data/textures/sun.dds");
	if (!result)
	{
		return false;
	}

	return true;
}


void GraphicsClass::Shutdown()
{
	// Release the sun map texture
	if (m_SunTexture)
	{
		m_SunTexture->Shutdown();
		delete m_SunTexture;
		m_SunTexture = 0;
	}

	// Release the reflection render to texture object.
	if (m_ReflectionTexture)
	{
		m_ReflectionTexture->Shutdown();
		delete m_ReflectionTexture;
		m_ReflectionTexture = 0;
	}

	// Release the refraction render to texture object.
	if (m_RefractionTexture)
	{
		m_RefractionTexture->Shutdown();
		delete m_RefractionTexture;
		m_RefractionTexture = 0;
	}

	// Release the refraction shader object.
	if (m_RefractionShader)
	{
		m_RefractionShader->Shutdown();
		delete m_RefractionShader;
		m_RefractionShader = 0;
	}

	// Release the light shader object.
	if (m_LightShader)
	{
		m_LightShader->Shutdown();
		delete m_LightShader;
		m_LightShader = 0;
	}

	// Release the light object
	if (m_Light)
	{
		delete m_Light;
		m_Light = 0;
	}

	// Release the position object.
	if (m_Position)
	{
		delete m_Position;
		m_Position = 0;
	}

	// Release the timer object.
	if (m_Timer)
	{
		delete m_Timer;
		m_Timer = 0;
	}

	// Release the shader object.
	if (m_Shader)
	{
		m_Shader->Shutdown();
		delete m_Shader;
		m_Shader = 0;
	}

	// Release the model objects
	if (m_WaterModel)
	{
		m_WaterModel->Shutdown();
		delete m_WaterModel;
		m_WaterModel = 0;
	}
	if (m_CausticGround)
	{
		m_CausticGround->Shutdown();
		delete m_CausticGround;
		m_CausticGround = 0;
	}
	if (m_BathModel)
	{
		m_BathModel->Shutdown();
		delete m_BathModel;
		m_BathModel = 0;
	}
	if (m_GroundModel)
	{
		m_GroundModel->Shutdown();
		delete m_GroundModel;
		m_GroundModel = 0;
	}
	if (m_WallModel)
	{
		m_WallModel->Shutdown();
		delete m_WallModel;
		m_WallModel = 0;
	}

	// Release the camera object.
	if (m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	if (m_D3D)
	{
		m_D3D->Shutdown();
		delete m_D3D;
		m_D3D = 0;
	}

	if (m_Input) {
		delete m_Input;
		m_Input = 0;
	}

	return;
}


bool GraphicsClass::Frame()
{
	bool result;

	m_waterTranslation += 0.001f;
	if (m_waterTranslation > 1.0f)
	{
		m_waterTranslation -= 1.0f;
	}

	// Read the user input.
	result = m_Input->Frame();
	if (!result)
	{
		return false;
	}

	// Check if the user pressed escape and wants to exit the application.
	if (m_Input->IsEscapePressed() == true)
	{
		return false;
	}

	m_Timer->Frame();

	return true;
}

bool GraphicsClass::Render() 
{
	bool result;

	// Process inputs for free camera
	result = HandleInput(m_Timer->GetTime());
	if (!result) {
		return false;
	}
	
	// Render the refraction of the scene to a texture.
	result = RenderRefractionToTexture();
	if (!result)
	{
		return false;
	}

	// Render the reflection of the scene to a texture.
	result = RenderReflectionToTexture();
	if (!result)
	{
		return false;
	}

	// Render the graphics scene.
	result = RenderScene();
	if (!result)
	{
		return false;
	}
}

bool GraphicsClass::RenderRefractionToTexture()
{
	DirectX::XMFLOAT4 clipPlane;
	DirectX::XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;
	
	// Setup a clipping plane based on the height of the water to clip everything above it.
	clipPlane = DirectX::XMFLOAT4(0.0f, -1.0f, 0.0f, m_waterHeight +0.1f);

	// Set the render target to be the refraction render to texture.
	m_RefractionTexture->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());

	// Clear the refraction render to texture.
	m_RefractionTexture->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_D3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);
	
	// Put the bath model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	m_BathModel->Render(m_D3D->GetDeviceContext());

	// Render the bath model using the light shader.
	result = m_RefractionShader->Render(m_D3D->GetDeviceContext(), m_BathModel->GetIndexCount(), worldMatrix, viewMatrix,
		projectionMatrix, m_BathModel->GetTexture(), m_Light->GetDirection(),
		m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(), clipPlane);
	if (!result)
	{
		return false;
	}

	m_CausticGround->Render(m_D3D->GetDeviceContext());

	result = m_CausticShader->Render(m_D3D->GetDeviceContext(), m_CausticGround->GetIndexCount(), worldMatrix, viewMatrix,
		projectionMatrix,
		m_CausticGround->GetTexture(),
		m_SunTexture->GetTexture(),
		m_Light->GetDirection(), m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(),
		m_Camera->GetPosition(),
		m_Light->GetSpecularColor(), m_Light->GetSpecularPower());
	if (!result)
	{
		return false;
	}

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	m_D3D->SetBackBufferRenderTarget();

	return true;
}

bool GraphicsClass::RenderReflectionToTexture()
{
	DirectX::XMMATRIX reflectionViewMatrix, worldMatrix, projectionMatrix;
	bool result;

	// Set the render target to be the reflection render to texture.
	m_ReflectionTexture->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());

	// Clear the reflection render to texture.
	m_ReflectionTexture->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);
	
	// Use the camera to render the reflection and create a reflection view matrix.
	m_Camera->RenderReflection(m_waterHeight);

	// Get the camera reflection view matrix instead of the normal view matrix.
	reflectionViewMatrix = m_Camera->GetReflectionViewMatrix();

	// Get the world and projection matrices from the d3d object.
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	// Put the wall model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	m_WallModel->Render(m_D3D->GetDeviceContext());

	// Render the wall model using the light shader and the reflection view matrix.
	result = m_LightShader->Render(m_D3D->GetDeviceContext(), m_WallModel->GetIndexCount(), worldMatrix, reflectionViewMatrix,
		projectionMatrix, 
		m_Light->GetDirection(), m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(), 
		m_WallModel->GetTexture());
	if (!result)
	{
		return false;
	}

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	m_D3D->SetBackBufferRenderTarget();

	return true;
}

bool GraphicsClass::HandleInput(float frameTime)
{
	bool keyDown, result;
	float posX, posY, posZ, rotX, rotY, rotZ;

	// Set the frame time for calculating the updated position.
	m_Position->SetFrameTime(frameTime);

	// Handle the input.
	keyDown = m_Input->IsLeftPressed();
	m_Position->TurnLeft(keyDown);

	keyDown = m_Input->IsRightPressed();
	m_Position->TurnRight(keyDown);

	keyDown = m_Input->IsUpPressed();
	m_Position->MoveForward(keyDown);

	keyDown = m_Input->IsDownPressed();
	m_Position->MoveBackward(keyDown);

	keyDown = m_Input->IsAPressed();
	m_Position->MoveUpward(keyDown);

	keyDown = m_Input->IsZPressed();
	m_Position->MoveDownward(keyDown);

	keyDown = m_Input->IsCPressed();
	m_Position->MoveLeftSideways(keyDown);

	keyDown = m_Input->IsVPressed();
	m_Position->MoveRightSideways(keyDown);

	keyDown = m_Input->IsPgUpPressed();
	m_Position->LookUpward(keyDown);

	keyDown = m_Input->IsPgDownPressed();
	m_Position->LookDownward(keyDown);

	// Get the view point position/rotation.
	m_Position->GetPosition(posX, posY, posZ);
	m_Position->GetRotation(rotX, rotY, rotZ);

	// Set the position of the camera.
	m_Camera->SetPosition(posX, posY, posZ);
	m_Camera->SetRotation(rotX, rotY, rotZ);

	return true;
}

bool GraphicsClass::RenderScene()
{
	DirectX::XMMATRIX viewMatrix, projectionMatrix, worldMatrix, reflectionMatrix;
	bool result;

	// Clear the buffers to begin the scene.
	m_D3D->BeginScene(0.2f, 0.2f, 0.2f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);
	
	m_GroundModel->Render(m_D3D->GetDeviceContext());
	result = m_LightShader->Render(m_D3D->GetDeviceContext(), m_GroundModel->GetIndexCount(), worldMatrix, viewMatrix,
		projectionMatrix, 
		m_Light->GetDirection(), m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(),
		m_GroundModel->GetTexture());
	if (!result)
	{
		return false;
	}

	// Reset the world matrix.
	m_D3D->GetWorldMatrix(worldMatrix);

	m_WallModel->Render(m_D3D->GetDeviceContext());
	result = m_LightShader->Render(m_D3D->GetDeviceContext(), m_WallModel->GetIndexCount(), worldMatrix, viewMatrix,
		projectionMatrix, 
		m_Light->GetDirection(), m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(),
		m_WallModel->GetTexture());
	if (!result)
	{
		return false;
	}

	// Reset the world matrix.
	m_D3D->GetWorldMatrix(worldMatrix);

	m_BathModel->Render(m_D3D->GetDeviceContext());
	result = m_LightShader->Render(m_D3D->GetDeviceContext(), m_BathModel->GetIndexCount(), worldMatrix, viewMatrix,
		projectionMatrix, 
		m_Light->GetDirection(), m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(),
		m_BathModel->GetTexture());
	if (!result)
	{
		return false;
	}

	// Reset the world matrix.
	m_D3D->GetWorldMatrix(worldMatrix);


#ifdef _DEBUG1
	ID3D11Resource* res;
	m_RefractionTexture->GetShaderResourceView()->GetResource(&res);
	HRESULT status = DirectX::SaveDDSTextureToFile(m_D3D->GetDeviceContext(), res, L"./data/REFRACTION.dds");
	m_ReflectionTexture->GetShaderResourceView()->GetResource(&res);
	status = DirectX::SaveDDSTextureToFile(m_D3D->GetDeviceContext(), res, L"./data/REFLECTION.dds");
#endif
	
	// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	m_WaterModel->Render(m_D3D->GetDeviceContext());
	reflectionMatrix = m_Camera->GetReflectionViewMatrix();

	// Render the model using the color shader.
	result = m_Shader->Render(m_D3D->GetDeviceContext(), m_WaterModel->GetIndexCount(), worldMatrix, viewMatrix,
		projectionMatrix, reflectionMatrix,
		m_ReflectionTexture->GetShaderResourceView(),
		m_RefractionTexture->GetShaderResourceView(),
		m_WaterModel->GetTexture(), m_waterTranslation, 0.01f,
		m_Light->GetDirection(), m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(), 
		m_Camera->GetPosition(),
		m_Light->GetSpecularColor(), m_Light->GetSpecularPower());
	if (!result)
	{
		return false;
	}

	// Present the rendered scene to the screen.
	m_D3D->EndScene();

	return true;
}
