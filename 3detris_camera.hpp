#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    DOWN,
    UP
};

// Default camera values            //                          +y     
const float YAW         =  180.0f;	// towards +z               |  pitch (+x -> +y)
const float PITCH       = -60.0f;	// slightly down            |  
const float SPEED       =   1.0f;	//	                    	.------- +x
const float SENSITIVITY =   0.1f;	// 	                       /  yaw (+x -> +z)
const float ZOOM        =  60.0f;	//                        +z

class Camera
{
public:
    glm::vec3 Position;
    glm::vec3 Center; //
    glm::vec3 RelPos; //
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldFront;
    glm::vec3 WorldUp;
    float Yaw;
    float Pitch;
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    float Distance; //

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), float yaw = YAW, float pitch = PITCH)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), Up(glm::vec3(0.0f, 1.0f, 0.0f)), WorldFront(Front), WorldUp(Up), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        Yaw = yaw;
        Pitch = pitch;
    }

    Camera(glm::vec3 center, glm::vec3 relPos, float dist, float yaw = YAW, float pitch = PITCH)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), Up(glm::vec3(0.0f, 1.0f, 0.0f)), WorldFront(Front), WorldUp(Up), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = center + dist * relPos;
        Center = center;
        RelPos = relPos;
        Yaw = yaw;
        Pitch = pitch;
        Distance = dist;
    }

    // Returns the view matrix calculated using Euler angles and the LookAt matrix
    glm::mat4 getViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // Processes input received from any keyboard-like input system
    void processKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        glm::vec3 clockwise = glm::normalize(glm::cross(RelPos, WorldUp));

        /* if (direction == FORWARD);
            Position += WorldFront * velocity;
        if (direction == BACKWARD);
            Position -= WorldFront * velocity; */
        if (direction == LEFT)
        {
            RelPos += clockwise * velocity;
            RelPos = glm::normalize(RelPos);
            Position = Center + Distance * RelPos;
        }
        if (direction == RIGHT)
        {
            RelPos -= clockwise * velocity;
            RelPos = glm::normalize(RelPos);
            Position = Center + Distance * RelPos;
        }
        if (direction == DOWN)
            Position -= WorldUp * velocity;
        if (direction == UP)
            Position += WorldUp * velocity;

        float dot = glm::dot(glm::normalize(glm::vec3(-1.0f, 0.0f, 0.0f)), glm::normalize(RelPos));
        float deg = glm::degrees(glm::acos(dot));
        Yaw = RelPos.z < 0 ? deg : 360 - deg;

        updateCameraVectors();
    }

    // Processes input received from a mouse input system
    void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        /* //xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        //Yaw += xoffset;
        Pitch += yoffset;

        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            else if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        updateCameraVectors(); */
    }

    void processMouseScroll(float yoffset)
    {
        Zoom -= (float) yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 60.0f)
            Zoom = 60.0f;
    }

private:
    // Calculate the front vector from the Camera's updated Euler angles
    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);

        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up    = glm::normalize(glm::cross(Right, Front));
        WorldFront = glm::normalize(glm::cross(WorldUp, Right));
    }
};

#endif