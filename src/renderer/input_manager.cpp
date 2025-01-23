#include "input_manager.h"
#include <raymath.h>

InputManager::InputManager()
{
    // Set default key bindings
    keyBindings[InputAction::MOVE_FORWARD] = KEY_W;
    keyBindings[InputAction::MOVE_BACKWARD] = KEY_S;
    keyBindings[InputAction::MOVE_LEFT] = KEY_A;
    keyBindings[InputAction::MOVE_RIGHT] = KEY_D;
    keyBindings[InputAction::JUMP] = KEY_SPACE;
    keyBindings[InputAction::INTERACT] = KEY_E;
    keyBindings[InputAction::TOGGLE_DEBUG] = KEY_F1;
    keyBindings[InputAction::TOGGLE_CAMERA_MODE] = KEY_C; // Add camera mode toggle

    // Initialize states for all actions
    for (const auto &[action, key] : keyBindings)
    {
        inputStates[action] = InputState{};
    }
}

void InputManager::update()
{
    // Update mouse state
    Vector2 currentMousePos = GetMousePosition();
    mouseDelta = Vector2Subtract(currentMousePos, mousePosition);
    mousePosition = currentMousePos;

    // Update keyboard states
    for (auto &[action, state] : inputStates)
    {
        state.wasPressed = state.isPressed;
        state.isPressed = IsKeyDown(keyBindings[action]);
    }
}

bool InputManager::isActionPressed(InputAction action) const
{
    auto it = inputStates.find(action);
    return it != inputStates.end() ? it->second.isPressed : false;
}

bool InputManager::isActionJustPressed(InputAction action) const
{
    auto it = inputStates.find(action);
    return it != inputStates.end() ? (it->second.isPressed && !it->second.wasPressed) : false;
}

void InputManager::setKeyBinding(InputAction action, int key)
{
    keyBindings[action] = key;
}

void InputManager::toggleMouseLock()
{
    mouseLocked = !mouseLocked;
    if (mouseLocked)
    {
        DisableCursor();
    }
    else
    {
        EnableCursor();
    }
}