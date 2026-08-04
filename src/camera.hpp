#pragma once

#include "mesh.hpp"

class Camera {
public:
    Vec3f      position = Vec3f::zero();
    Quaternion rotation = Quaternion::identity();

    float nearPlane = 0.1f;
    float farPlane  = 2048.0f;

    Camera();

    float getFov() const;
    float getFovHalfRad() const;
    float getFovHalfCot() const;
    void  setFov(float deg);

    void center(const Mesh *meshWS, float widthDivHeight);

    void update(double deltaTime);
private:
    // Horizontal fov
    float fovDeg     = 0;
    float fovHalf    = 0;
    float fovHalfCot = 0;

    // Rotation is calculated from Euler angles on every frame
    float rotationEulerX = 0;
    float rotationEulerY = 0;

    float movementSpeed = 200.0f;
    float rotationSpeed = 500.0f;
};

