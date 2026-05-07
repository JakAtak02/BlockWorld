#include "core/Camera.h"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
{
    m_position = glm::vec3(0.0f, 0.0f, 3.0f);
    m_worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    m_yaw = -90.0f;
    m_pitch = 0.0f;
    m_fov = 70.0f;

    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(
        m_position,
        m_position + m_front,
        m_up
    );
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const
{
    return glm::perspective(
        glm::radians(m_fov),
        aspectRatio,
        0.1f,
        100.0f
    );
}

const glm::vec3& Camera::getPosition() const
{
    return m_position;
}

const glm::vec3& Camera::getForward() const
{
    return m_front;
}

void Camera::moveForward(float amount)
{
    m_position += m_front * amount;
}

void Camera::moveRight(float amount)
{
    m_position += m_right * amount;
}

void Camera::moveUp(float amount)
{
    m_position += m_worldUp * amount;
}

void Camera::processMouseMovement(float xOffset, float yOffset)
{
    float sensitivity = 0.1f;

    xOffset *= sensitivity;
    yOffset *= sensitivity;

    m_yaw += xOffset;
    m_pitch += yOffset;

    if (m_pitch > 89.0f)
        m_pitch = 89.0f;

    if (m_pitch < -89.0f)
        m_pitch = -89.0f;

    updateCameraVectors();
}

void Camera::updateCameraVectors()
{
    glm::vec3 front;

    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

    m_front = glm::normalize(front);
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}