#pragma once

#include <glm/glm.hpp>

class Camera
{
public:
    Camera();

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspectRatio) const;

    const glm::vec3& getPosition() const;
    const glm::vec3& getForward() const;

    void moveForward(float amount);
    void moveRight(float amount);
    void moveUp(float amount);

    void processMouseMovement(float xOffset, float yOffset);

private:
    void updateCameraVectors();

private:
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_right;
    glm::vec3 m_up;
    glm::vec3 m_worldUp;

    float m_yaw;
    float m_pitch;
    float m_fov;
};