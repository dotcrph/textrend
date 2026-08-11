#include "camera.hpp"

#include <cassert>
#include <cmath>

#include "args.hpp"
#include "math.hpp"
#include "terminal.hpp"

Camera::Camera()
{
    setFov(90.0f);
}

float Camera::getFov() const
{
    return fovDeg;
}

float Camera::getFovHalfRad() const
{
    return fovHalf;
}

float Camera::getFovHalfCot() const
{
    return fovHalfCot;
}

void Camera::setFov(float deg)
{
    assert(deg != 0);
    fovDeg     = deg;
    fovHalf    = deg / 2 * degToRad;
    fovHalfCot = 1 / tan(fovHalf);
}

void Camera::center(const Mesh *meshWS, float widthDivHeight)
{
    Vec3f base = (meshWS->bbMin + meshWS->bbMax) / 2;

    float dx = meshWS->bbMax.x - meshWS->bbMin.x;
    float dy = meshWS->bbMax.y - meshWS->bbMin.y;

    float yFovHalfCot = fovHalfCot * widthDivHeight;

    float xZ = dx *  fovHalfCot;
    float yZ = dy * yFovHalfCot;

    position = {base.x, base.y, base.z - max(xZ, yZ)};
}

void Camera::update(double deltaTime)
{
    // Movement
    float movementFactor = movementSpeed * deltaTime;

    Vec3f forward = rotation.rotate(Vec3f::forward()) * movementFactor;
    Vec3f left    = rotation.rotate(Vec3f::left())    * movementFactor;
    Vec3f up      = rotation.rotate(Vec3f::up())      * movementFactor;

    if (terminal::getKeyHeld('w'))
        position += forward;

    if (terminal::getKeyHeld('s'))
        position -= forward;

    if (terminal::getKeyHeld('a'))
        position += left;

    if (terminal::getKeyHeld('d'))
        position -= left;

    if (terminal::getKeyHeld('e'))
        position += up;

    if (terminal::getKeyHeld('q'))
        position -= up;

    // Rotation
    float rotationDelta = rotationSpeed * deltaTime;
    float yFactor       = args::getFlipY() ? 1 : -1;

    if (terminal::getKeyHeld('k') || terminal::getKeyHeld('4'))
        rotationEulerY += rotationDelta;

    if (terminal::getKeyHeld(';') || terminal::getKeyHeld('6'))
        rotationEulerY -= rotationDelta;

    if (terminal::getKeyHeld('o') || terminal::getKeyHeld('8'))
        rotationEulerX += rotationDelta * yFactor;

    if (terminal::getKeyHeld('l') || terminal::getKeyHeld('2'))
        rotationEulerX -= rotationDelta * yFactor;

    rotation = Quaternion::fromEuler(rotationEulerX, rotationEulerY, 0.0f);

    // Fov and speed
    if (terminal::getKeyDown('[')) {
        if (movementSpeed <= 10.0f)
            movementSpeed = 10.0f;
        else
            movementSpeed -= 10.0f;
    }

    if (terminal::getKeyDown(']'))
        movementSpeed += 20.0f;

    if (terminal::getKeyDown('{')) {
        if (rotationSpeed <= 50.0f)
            rotationSpeed = 50.0f;
        else
            rotationSpeed -= 50.0f;
    }

    if (terminal::getKeyDown('}'))
        rotationSpeed += 50.0f;

    if (terminal::getKeyDown('=') || terminal::getKeyDown('+')) {
        if (getFov() <= 10.0f)
            setFov(10.0f);
        else
            setFov(getFov() - 1.0f);
    }

    if (terminal::getKeyDown('-') || terminal::getKeyDown('_')) {
        if (getFov() >= 160.0f)
            setFov(160.0f);
        else
            setFov(getFov() + 1.0f);
    }
}
