//***************************************************************************************
// Camera.h by Frank Luna (C) 2011 All Rights Reserved.
//   
// Simple first person style camera class that lets the viewer explore the 3D scene.
//   -It keeps track of the camera coordinate system relative to the world space
//    so that the view matrix can be constructed.  
//   -It keeps track of the viewing frustum of the camera so that the projection
//    matrix can be obtained.
//***************************************************************************************

#ifndef CAMERA_H
#define CAMERA_H

#include "d3dUtil.h"

class Camera
{
public:
    Camera();
    ~Camera();

    // Get/Set world camera position.
    XMVECTOR    GetPositionXM() const { return XMLoadFloat3(&mPosition); }
    XMFLOAT3    GetPosition() const { return mPosition; }
    void        SetPosition(float x, float y, float z){ mPosition = XMFLOAT3(x, y, z); }
    void        SetPosition(const XMFLOAT3& v){ mPosition = v; }

    // Get camera basis vectors.
    XMVECTOR    GetRightXM() const { return XMLoadFloat3(&mRight); }
    XMFLOAT3    GetRight() const { return mRight; }
    XMVECTOR    GetUpXM() const { return XMLoadFloat3(&mUp); }
    XMFLOAT3    GetUp() const { return mUp; }
    XMVECTOR    GetLookXM() const { return XMLoadFloat3(&mLook); }
    XMFLOAT3    GetLook() const { return mLook; }

    // Get frustum properties.
    float       GetNearZ() const { return mNearZ; }
    float       GetFarZ() const { return mFarZ; }
    float       GetAspect(){ return mAspect; }
    float       GetFovY() const { return mFovY; }
    float       GetFovX() const
    {
        float halfWidth = 0.5f*GetNearWindowWidth();
        return 2.0f*atan(halfWidth / mNearZ);
    }

    // Get near and far plane dimensions in view space coordinates.
    float       GetNearWindowWidth() const { return mAspect * mNearWindowHeight; }
    float       GetNearWindowHeight() const { return mNearWindowHeight; }
    float       GetFarWindowWidth() const { return mAspect * mFarWindowHeight; }
    float       GetFarWindowHeight() const { return mFarWindowHeight; }

    // Get View/Proj matrices.
    XMMATRIX    View() const { return XMLoadFloat4x4(&mView); }
    XMMATRIX    Proj() const { return XMLoadFloat4x4(&mProj); }
    XMMATRIX    ViewProj() const { return XMMatrixMultiply(View(), Proj()); }

public:
    // Set frustum.
	void SetLens(float fovY, float aspect, float zn, float zf);
    void Update(float dt);

	// Define camera space via LookAt parameters.
	void LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp);
	void LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up);

	// Strafe/Walk the camera a distance d.
	void Strafe(float d);
    void Walk(float d);
    void Jump(float d);

	// Rotate the camera.
	void Pitch(float angle);
	void RotateY(float angle);

	// After modifying camera position/orientation, call to rebuild the view matrix.
	void UpdateViewMatrix();

private:
	// Camera coordinate system with coordinates relative to world space.
	XMFLOAT3 mPosition;
	XMFLOAT3 mRight;
	XMFLOAT3 mUp;
	XMFLOAT3 mLook;

	// Cache frustum properties.
	float mNearZ;
	float mFarZ;
	float mAspect;
	float mFovY;
	float mNearWindowHeight;
	float mFarWindowHeight;

	// Cache View/Proj matrices.
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;
};

#endif // CAMERA_H