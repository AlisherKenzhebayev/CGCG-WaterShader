#include "cameraclass.h"

CameraClass::CameraClass()
{
	m_positionX = 0.0f;
	m_positionY = 0.0f;
	m_positionZ = 0.0f;

	m_rotationX = 0.0f;
	m_rotationY = 0.0f;
	m_rotationZ = 0.0f;
}


CameraClass::CameraClass(const CameraClass& other)
{
}


CameraClass::~CameraClass()
{
}

void CameraClass::SetPosition(float x, float y, float z)
{
	m_positionX = x;
	m_positionY = y;
	m_positionZ = z;
	return;
}


void CameraClass::SetRotation(float x, float y, float z)
{
	m_rotationX = x;
	m_rotationY = y;
	m_rotationZ = z;
	return;
}

DirectX::XMFLOAT3 CameraClass::GetPosition()
{
	DirectX::XMFLOAT3 retVal{ m_positionX, m_positionY, m_positionZ };

	return retVal;
}

DirectX::XMVECTOR CameraClass::GetRotation()
{
	DirectX::XMVECTOR retVal{ m_rotationX, m_rotationY, m_rotationZ };

	return retVal;
}

void CameraClass::Render()
{
	DirectX::XMVECTOR up{0.0f, 1.0f, 0.0f}, position{m_positionX, m_positionY, m_positionZ}, lookAt{0.0f, 0.0f, 1.0f};
	float yaw, pitch, roll;
	DirectX::XMMATRIX rotationMatrix;

	// Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.
	pitch	 = m_rotationX * 0.0174532925f;
	yaw		 = m_rotationY * 0.0174532925f;
	roll	 = m_rotationZ * 0.0174532925f;

	// Create the rotation matrix from the yaw, pitch, and roll values.
	rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
	
	//DirectX::XMVECTOR xmLookAt		= DirectX::XMVECTOR(&lookAt);
	//DirectX::XMVECTOR xmUp			= DirectX::XMLoadFloat3(&up);
	//DirectX::XMVECTOR xmPosition	= DirectX::XMLoadFloat3(&position);

	// Transform the lookAt and up vector by the rotation matrix so the view is correctly rotated at the origin.
	lookAt = DirectX::XMVector3Transform(lookAt, rotationMatrix);
	up = DirectX::XMVector3Transform(up, rotationMatrix);

	// Translate the rotated camera position to the location of the viewer.
	lookAt = DirectX::XMVectorAdd(lookAt, position);

	assert(!DirectX::XMVector3Equal(lookAt, DirectX::XMVectorZero()));
	assert(!DirectX::XMVector3Equal(position, DirectX::XMVectorZero()));
	assert(!DirectX::XMVector3Equal(up, DirectX::XMVectorZero()));

	//DirectX::XMStoreFloat3(&lookAt, xmLookAt);
	//DirectX::XMStoreFloat3(&up, xmUp);
	//DirectX::XMStoreFloat3(&position, xmPosition);

	// Finally create the view matrix from the three updated vectors.
	m_viewMatrix = DirectX::XMMatrixLookAtLH(position, lookAt, up);

	return;
}

void CameraClass::RenderReflection(float height)
{
	DirectX::XMVECTOR up{ 0.0f, 1.0f, 0.0f }, position, lookAt{ 0.0f, 0.0f, 1.0f };
	DirectX::XMMATRIX rotationMatrix;
	float yaw, pitch, roll; 
	float radians;

	// Setup the position of the camera in the world.
	// For planar reflection invert the Y position of the camera.
	position = { m_positionX, -m_positionY + (height * 2.0f), m_positionZ };
	
	// Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.  Invert the X rotation for reflection.
	pitch = -m_rotationX * 0.0174532925f;
	yaw = m_rotationY * 0.0174532925f;
	roll = m_rotationZ * 0.0174532925f;

	// Create the rotation matrix from the yaw, pitch, and roll values.
	rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	// Transform the lookAt and up vector by the rotation matrix so the view is correctly rotated at the origin.
	lookAt = DirectX::XMVector3TransformCoord(lookAt, rotationMatrix);
	up = DirectX::XMVector3TransformCoord(up, rotationMatrix);

	// Translate the rotated camera position to the location of the viewer.
	lookAt = DirectX::XMVectorAdd(position, lookAt);

	// Create the view matrix from the three vectors.
	m_reflectionViewMatrix = DirectX::XMMatrixLookAtLH(position, lookAt, up);

	return;
}

DirectX::XMMATRIX CameraClass::GetReflectionViewMatrix()
{
	return m_reflectionViewMatrix;
}

void CameraClass::GetViewMatrix(DirectX::XMMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
	return;
}
