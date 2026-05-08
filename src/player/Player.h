#pragma once

#include "core/Camera.h"

#include <cstdint>
#include <glm/glm.hpp>

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

    bool selectSlot1Pressed = false;
    bool selectSlot2Pressed = false;
    bool selectSlot3Pressed = false;
    bool selectSlot4Pressed = false;
    bool selectSlot5Pressed = false;
    bool selectSlot6Pressed = false;
    bool selectSlot7Pressed = false;
    bool selectSlot8Pressed = false;

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

    uint16_t getSelectedBlockId() const;

    bool wouldBlockOverlapPlayer(
        const glm::ivec3& blockPosition
    ) const;

    Camera& getCamera();
    const Camera& getCamera() const;

private:
    Camera m_camera;

    bool m_breakBlockRequested = false;
    bool m_placeBlockRequested = false;

    bool m_breakMousePreviouslyPressed = false;
    bool m_placeMousePreviouslyPressed = false;

    uint16_t m_selectedBlockId = 1;

    float m_playerWidth = 0.6f;
    float m_playerHeight = 1.8f;

    float m_moveSpeed = 5.0f;
};