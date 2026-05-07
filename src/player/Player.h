#pragma once

#include "core/Camera.h"

struct PlayerInputState
{
    bool moveForward = false;
    bool moveBackward = false;
    bool moveLeft = false;
    bool moveRight = false;
    bool moveUp = false;
    bool moveDown = false;

    bool breakBlockPressed = false;
    bool placeBlockPressed = false;

    float lookDeltaX = 0.0f;
    float lookDeltaY = 0.0f;
};

class Player
{
public:
    Player();

    void update(const PlayerInputState& input, float deltaTime);

    bool wantsToBreakBlock() const;
    bool wantsToPlaceBlock() const;

    Camera& getCamera();
    const Camera& getCamera() const;

private:
    Camera m_camera;

    bool m_breakBlockRequested = false;
    bool m_placeBlockRequested = false;

    bool m_breakMousePreviouslyPressed = false;
    bool m_placeMousePreviouslyPressed = false;

    float m_moveSpeed = 5.0f;
};