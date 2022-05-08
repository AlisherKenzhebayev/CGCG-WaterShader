#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_

#include "inputclass.h"

#include "d3dclass.h"
#include "cameraclass.h"
#include "modelclass.h"
#include "lightclass.h"
#include "timerclass.h"
#include "positionclass.h"
#include "sinetextureshaderclass.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

// Wrapper object
class GraphicsClass
{
public:
	GraphicsClass();
	GraphicsClass(const GraphicsClass&);
	~GraphicsClass();

	bool Initialize(HINSTANCE, HWND, int, int);
	void Shutdown();
	bool Frame();

private:
	bool Render();
	bool HandleInput(float);

private:
	InputClass* m_Input;
	D3DClass* m_D3D;
	CameraClass* m_Camera;
	PositionClass* m_Position;
	ModelClass* m_Model;
	SineTextureShaderClass* m_Shader;
	TimerClass* m_Timer;
	LightClass* m_Light;
};

#endif
