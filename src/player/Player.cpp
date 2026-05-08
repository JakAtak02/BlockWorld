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

    if (input.selectSlot1Pressed)
        m_selectedBlockId = 1;

    if (input.selectSlot2Pressed)
        m_selectedBlockId = 2;

    if (input.selectSlot3Pressed)
        m_selectedBlockId = 3;

    if (input.selectSlot4Pressed)
        m_selectedBlockId = 4;

    if (input.selectSlot5Pressed)
        m_selectedBlockId = 5;

    if (input.selectSlot6Pressed)
        m_selectedBlockId = 6;

    if (input.selectSlot7Pressed)
        m_selectedBlockId = 7;

    if (input.selectSlot8Pressed)
        m_selectedBlockId = 8;

    m_breakBlockRequested =
        input.breakBlockPressed &&
        !m_breakMousePreviouslyPressed;

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

uint16_t Player::getSelectedBlockId() const
{
    return m_selectedBlockId;
}

bool Player::wouldBlockOverlapPlayer(
    const glm::ivec3& blockPosition
) const
{
    glm::vec3 playerPosition =
        m_camera.getPosition();

    float playerHalfWidth =
        m_playerWidth * 0.5f;

    glm::vec3 playerMin(
        playerPosition.x - playerHalfWidth,
        playerPosition.y,
        playerPosition.z - playerHalfWidth
    );

    glm::vec3 playerMax(
        playerPosition.x + playerHalfWidth,
        playerPosition.y + m_playerHeight,
        playerPosition.z + playerHalfWidth
    );

    glm::vec3 blockMin(
        static_cast<float>(blockPosition.x),
        static_cast<float>(blockPosition.y),
        static_cast<float>(blockPosition.z)
    );

    glm::vec3 blockMax =
        blockMin + glm::vec3(1.0f);

    bool overlapX =
        playerMin.x < blockMax.x &&
        playerMax.x > blockMin.x;

    bool overlapY =
        playerMin.y < blockMax.y &&
        playerMax.y > blockMin.y;

    bool overlapZ =
        playerMin.z < blockMax.z &&
        playerMax.z > blockMin.z;

    return overlapX && overlapY && overlapZ;
}

Camera& Player::getCamera()
{
    return m_camera;
}

const Camera& Player::getCamera() const
{
    return m_camera;
}