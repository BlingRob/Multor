/// \file Camera.h
#ifndef CAMERA_H
#define CAMERA_H

#include "Entity.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <math.h>

namespace Multor
{

// Default cameras options
const float YAW         = -90.0f;
const float PITCH       = 0.0f;
const float SPEED       = 4.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM        = 45.0f;

class Camera:public Entity
{
public:
    enum class Camera_Movement
    {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };
    // Camera attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // Euler's angels
    float Yaw;
    float Pitch;
    // Camera's options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    //Limits
    static constexpr float PitchLimit    = glm::radians(89.0f);
    static constexpr float ZoomLowLimit  = 1.0f;
    static constexpr float ZoomHighLimit = 45.0f;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH): MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        Front = target;
        WorldUp = glm::normalize(up);
        Yaw = glm::radians(yaw);
        Pitch = glm::radians(pitch);
        updateCameraVectors();
    }

    inline glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    //Process key pressing
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        switch (direction) 
        {
            case Camera_Movement::FORWARD:
                Position += Front * velocity;
            break;
            case Camera_Movement::BACKWARD:
                Position -= Front * velocity;
            break;
            case Camera_Movement::LEFT:
                Position -= Right * velocity;
            break;
            case Camera_Movement::RIGHT:
                Position += Right * velocity;
            break;
        }
    }

    // Process mouse movement
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw   += glm::radians(xoffset);
        Pitch += glm::radians(yoffset);

        // Controll pitch range
        if (constrainPitch)
            if(glm::abs(Pitch) > PitchLimit)
                Pitch = std::copysignf(PitchLimit, Pitch);

        // Update vectors
        updateCameraVectors();
    }

    // Process mouse scroll
    void ProcessMouseScroll(float yoffset)
    {
        if (Zoom >= ZoomLowLimit && Zoom <= ZoomHighLimit)
            Zoom -= yoffset;
        else if (Zoom <= ZoomLowLimit)
            Zoom = ZoomLowLimit;
        else if (Zoom >= ZoomHighLimit)
            Zoom = ZoomHighLimit;
    }

private:
    // Process up vector based Euler's angels
    void updateCameraVectors()
    {
        // Process new right vector
        Front.x = cos(Yaw) * cos(Pitch);
        Front.y = sin(Pitch);
        Front.z = sin(Yaw) * cos(Pitch);
        Front   = glm::normalize(Front);
        // Reculculation right and up vectors
        Right   = glm::cross(Front, WorldUp);
        Up      = glm::cross(Right, Front);
    }
};
#endif

} // namespace Multor