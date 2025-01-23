#pragma once
#include "raylib.h"
#include <unordered_map>
#include <functional>

enum class InputAction
{
    MOVE_FORWARD,
    MOVE_BACKWARD,
    MOVE_LEFT,
    MOVE_RIGHT,
    JUMP,
    INTERACT,
    TOGGLE_DEBUG,
    TOGGLE_CAMERA_MODE
};

class InputManager
{
public:
    static InputManager &getInstance()
    {
        static InputManager instance;
        return instance;
    }

    // Delete copy constructor and assignment operator
    InputManager(const InputManager &) = delete;
    InputManager &operator=(const InputManager &) = delete;

    void update();
    bool isActionPressed(InputAction action) const;
    bool isActionJustPressed(InputAction action) const;
    Vector2 getMouseDelta() const { return mouseDelta; }
    void setKeyBinding(InputAction action, int key);
    void toggleMouseLock();
    bool isMouseLocked() const { return mouseLocked; }

private:
    struct InputState
    {
        bool isPressed = false;
        bool wasPressed = false;
    };

    std::unordered_map<InputAction, int> keyBindings;
    std::unordered_map<InputAction, InputState> inputStates;
    Vector2 mousePosition{0, 0};
    Vector2 mouseDelta{0, 0};
    bool mouseLocked = false;

    InputManager(); // Constructor is now private
};