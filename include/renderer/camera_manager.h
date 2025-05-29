#pragma once
#include "raylib.h"

class CameraManager
{
private:
    Camera3D camera;
    Vector3 player_start;
    float moveSpeed;
    bool locked;

protected:
    int currentMode; // Moved from private to protected

public:
    CameraManager();

    void init(const Vector3 &startPos);
    void update();
    void setMode(int mode);
    void setPosition(const Vector3 &pos);
    void toggleLock();
    void toggleMode()
    {
        currentMode = (currentMode == CAMERA_FIRST_PERSON) ? CAMERA_FREE : CAMERA_FIRST_PERSON;
    }

    int getCurrentMode() const { return currentMode; }

    Camera3D &getCamera() { return camera; }
};