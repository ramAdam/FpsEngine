
#include "camera_manager.h"
#include "raymath.h"

CameraManager::CameraManager()
    : currentMode(CAMERA_FIRST_PERSON), moveSpeed(0.5f), locked(true)
{
    camera = {0};
}

void CameraManager::init(const Vector3 &startPos)
{
    camera.position = startPos;
    player_start = startPos;
    camera.target = Vector3{0.0f, 0.0f, 0.0f};
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    if (locked)
        DisableCursor();
}

void CameraManager::update()
{
    if (currentMode == CAMERA_FREE)
    {
        UpdateCamera(&camera, CAMERA_FREE);
    }
    else
    {
        UpdateCamera(&camera, CAMERA_FIRST_PERSON);
    }
}

void CameraManager::setMode(int mode)
{
    currentMode = mode;
    if (mode == CAMERA_FIRST_PERSON)
    {
        camera.position = player_start;
    }
}

void CameraManager::setPosition(const Vector3 &pos)
{
    camera.position = pos;
    player_start = pos;
}

void CameraManager::toggleLock()
{
    locked = !locked;
    if (locked)
    {
        DisableCursor();
    }
    else
    {
        EnableCursor();
    }
}