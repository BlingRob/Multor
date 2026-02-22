/// \file camera.h

#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include "entity.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <cmath>

namespace Multor
{

/// \brief Default cameras options
const float YAW         = -90.0f;
const float PITCH       = 0.0f;
const float SPEED       = 4.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM        = 45.0f;

/// \brief Class of camera
class Camera : public Entity
{
public:
    /// \brief Directions of camera movement
    enum class Camera_Movement
    {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };

    /// \brief Camera attributes
    glm::vec3 position_;
    glm::vec3 front_;
    glm::vec3 up_;
    glm::vec3 right_;
    glm::vec3 worldUp_;

    /// \brief Euler's angels
    float yaw_;
    float pitch_;

    /// \brief Camera's options
    float movementSpeed_;
    float mouseSensitivity_;
    float zoom_;

    /// \brief Limits
    static constexpr float pitchLimit_    = glm::radians(89.0f);
    static constexpr float zoomLowLimit_  = 1.0f;
    static constexpr float zoomHighLimit_ = 45.0f;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 target   = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW,
           float pitch = PITCH)
        : movementSpeed_(SPEED), mouseSensitivity_(SENSITIVITY), zoom_(ZOOM)
    {
        position_ = position;
        front_    = target;
        worldUp_  = glm::normalize(up);
        yaw_      = glm::radians(yaw);
        pitch_    = glm::radians(pitch);
        updateCameraVectors();
    }

    inline glm::mat4 GetViewMatrix() const
    {
        return glm::lookAt(position_, position_ + front_, up_);
    }

    inline glm::mat4 GetProjectionMatrix(float aspect, float nearPlane = 0.1f,
                                         float farPlane = 150.0f,
                                         bool vulkanClip = true) const
    {
        glm::mat4 proj =
            glm::perspective(glm::radians(zoom_), aspect, nearPlane, farPlane);
        if (vulkanClip)
            proj[1][1] *= -1.0f;
        return proj;
    }

    //Process key pressing
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = movementSpeed_ * deltaTime;
        switch (direction)
            {
                case Camera_Movement::FORWARD:
                    position_ += front_ * velocity;
                    break;
                case Camera_Movement::BACKWARD:
                    position_ -= front_ * velocity;
                    break;
                case Camera_Movement::LEFT:
                    position_ -= right_ * velocity;
                    break;
                case Camera_Movement::RIGHT:
                    position_ += right_ * velocity;
                    break;
                case Camera_Movement::UP:
                    position_ += worldUp_ * velocity;
                    break;
                case Camera_Movement::DOWN:
                    position_ -= worldUp_ * velocity;
                    break;
            }
    }

    // Process mouse movement
    void ProcessMouseMovement(float xoffset, float yoffset,
                              bool constrainPitch = true)
    {
        xoffset *= mouseSensitivity_;
        yoffset *= mouseSensitivity_;

        yaw_ += glm::radians(xoffset);
        pitch_ += glm::radians(yoffset);

        // Controll pitch range
        if (constrainPitch)
            if (glm::abs(pitch_) > pitchLimit_)
                pitch_ = std::copysignf(pitchLimit_, pitch_);

        // Update vectors
        updateCameraVectors();
    }

    // Process mouse scroll
    void ProcessMouseScroll(float yoffset)
    {
        zoom_ -= yoffset;
        zoom_ = glm::clamp(zoom_, zoomLowLimit_, zoomHighLimit_);
    }

private:
    // Process up vector based Euler's angels
    void updateCameraVectors()
    {
        // Process new right vector
        front_.x = cos(yaw_) * cos(pitch_);
        front_.y = sin(pitch_);
        front_.z = sin(yaw_) * cos(pitch_);
        front_   = glm::normalize(front_);
        // Reculculation right and up vectors
        right_ = glm::normalize(glm::cross(front_, worldUp_));
        up_    = glm::normalize(glm::cross(right_, front_));
    }
};

} // namespace Multor

#endif
