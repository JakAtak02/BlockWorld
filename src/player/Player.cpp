#include "player/Player.h"

Player::Player()
{
}

void Player::update(const PlayerInputState& input, float deltaTime)
{
    float moveAmount = m_moveSpeed * deltaTime;

    if (input.moveForward)
        m_camera.moveForward(moveAmount);

    if (input.moveBackward)
        m_camera.moveForward(-moveAmount);

    if (input.moveRight)
        m_camera.moveRight(moveAmount);

    if (input.moveLeft)
        m_camera.moveRight(-moveAmount);

    if (input.moveUp)
        m_camera.moveUp(moveAmount);

    if (input.moveDown)
        m_camera.moveUp(-moveAmount);

    m_camera.processMouseMovement(input.lookDeltaX, input.lookDeltaY);

    // Rising edge detection for block breaking.
    m_breakBlockRequested =
        input.breakBlockPressed &&
        !m_breakMousePreviouslyPressed;

    // Rising edge detection for block placing.
    m_placeBlockRequested =
        input.placeBlockPressed &&
        !m_placeMousePreviouslyPressed;

    m_breakMousePreviouslyPressed = input.breakBlockPressed;
    m_placeMousePreviouslyPressed = input.placeBlockPressed;
}

bool Player::wantsToBreakBlock() const
{
    return m_breakBlockRequested;
}

bool Player::wantsToPlaceBlock() const
{
    return m_placeBlockRequested;
}

Camera& Player::getCamera()
{
    return m_camera;
}

const Camera& Player::getCamera() const
{
    return m_camera;
}