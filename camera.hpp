#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "game_logic.hpp"

enum CameraMode {
    CAMERA_ORTHO,
    CAMERA_PERSPECTIVE
};
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    DOWN,
    UP
};

// Default camera values            //                          +y     
const float YAW         = 270.0f;	// towards +z               |  pitch (+x -> +y)
const float PITCH       = -45.0f;	// slightly down            |  
const float SPEED       =   1.0f;	//	                    	.------- +x
const float SENSITIVITY =   0.1f;	// 	                       /  yaw (+x -> +z)
const float ZOOM        =  60.0f;	//                        +z

class Camera
{
    public:
        CameraMode mode;
        glm::vec3 Position; 
        glm::vec3 Front;    // where the camera is pointing at (it's actually the reverse, because -z is the front); camera direction is then = Position - Front // NOTE: for ortho, should probably be center of Area
        glm::vec3 WorldUp;  // +y in *World* space
        glm::vec3 Right;    // +x in Camera space = WorldUp x Front
        glm::vec3 Up;       // +y in Camera space = Front x Right

        glm::ivec3 areaDimensions;

        glm::vec3 Center; //
        glm::vec3 RelativePosition; //
        float Yaw;
        float Pitch;
        float MovementSpeed;
        float MouseSensitivity;
        float Zoom;
        float Distance; //

        Camera() {}
        Camera(Area area) : MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
        {
            mode = CAMERA_ORTHO;
            areaDimensions = glm::ivec3(area.WIDTH, area.HEIGHT, area.WIDTH);
            Center = glm::vec3(areaDimensions.x / 2.0f - 0.5f, areaDimensions.y / 2.0f - 0.5f, areaDimensions.z / 2.0f - 0.5f);
            RelativePosition = glm::vec3(0.0f, 0.0f, 1.0f);
            Distance = 2 * areaDimensions.x;
            Position = Center + Distance * RelativePosition;

            Yaw = YAW;
            Pitch = PITCH;
            WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
            
            updateCameraVectors();
        }

        // Returns the view matrix calculated using Euler angles and the LookAt matrix
        glm::mat4 getViewMatrix()
        {
            return glm::lookAt(Position, Position + Front, Up); // NOTE: Up bi trebalo zaminit sa WorldUp
        }
        glm::mat4 getProjectionMatrix()
        {
            if (mode == CAMERA_ORTHO)
                return glm::ortho(-12.0f, 12.0f, -12.0f, 12.0f, 0.1f, 100.0f);
            else
                return glm::perspective(glm::radians(Zoom), 1.0f /* (float)currScrWidth / currScrHeight */, 0.1f, 100.0f); 
                // NOTE: aspect 1.0f works only for resolutions where width = height
        }

        void toggleMode()
        {
            if (mode == CAMERA_ORTHO)
            {
                mode = CAMERA_PERSPECTIVE;
                Distance = 2.6f * areaDimensions.x;
            }
            else
            {
                mode = CAMERA_ORTHO;
                Distance = 2 * areaDimensions.x;
            }

            updateCameraVectors(); // Distance is updated => Position needs to be updated
        }

        // Processes input received from any keyboard-like input system
        void processKeyboard(Camera_Movement direction, float deltaTime)
        {
            float rotSpeed = 6.0f * deltaTime;
            glm::vec3 clockwise = glm::normalize(glm::cross(RelativePosition, WorldUp));

            if (direction == FORWARD)
            {
                /* RelativePosition += glm::normalize(Up) * velocity;
                RelativePosition = glm::normalize(RelativePosition);
                Position = Center + Distance * RelativePosition; */
                Pitch -= rotSpeed * 10.0f;
                if (Pitch > 89.0f)
                    Pitch = 89.0f;
                else if (Pitch < -89.0f)
                    Pitch = -89.0f;
            }
            if (direction == BACKWARD)
            {
                /* RelativePosition -= glm::normalize(Up) * velocity;
                RelativePosition = glm::normalize(RelativePosition);
                Position = Center + Distance * RelativePosition; */
                Pitch += rotSpeed * 10.0f;
                if (Pitch > 89.0f)
                    Pitch = 89.0f;
                else if (Pitch < -89.0f)
                    Pitch = -89.0f;
            }
            if (direction == LEFT)
            {
                /* RelativePosition += clockwise * velocity;
                RelativePosition = glm::normalize(RelativePosition);
                Position = Center + Distance * RelativePosition; */
                Yaw += rotSpeed * 10.0f;
                if (Yaw >= 360.0f)
                    Yaw -= 360.0f;
            }
            if (direction == RIGHT)
            {
                /* RelativePosition -= clockwise * velocity;
                RelativePosition = glm::normalize(RelativePosition);
                Position = Center + Distance * RelativePosition; */
                    Yaw -= rotSpeed * 10.0f;
                    if (Yaw < 0.0f)
                        Yaw += 360.0f;
            }
            /* if (direction == DOWN)
                Position -= WorldUp * velocity;
            if (direction == UP)
                Position += WorldUp * velocity; */

            /* float dot, deg;
            dot = glm::dot(glm::normalize(glm::vec3(-1.0f, 0.0f, 0.0f)), glm::normalize(RelativePosition));
            deg = glm::degrees(glm::acos(dot)); */
            //Yaw = RelativePosition.z < 0 ? deg : 360 - deg;

            /* dot = glm::dot(glm::normalize(glm::vec3(RelativePosition.x, 0.0f, RelativePosition.z)), glm::normalize(RelativePosition));
            deg = glm::degrees(glm::acos(dot));
            Pitch = RelativePosition.y < 0 ? deg : 360 - deg; */

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

            Right = glm::normalize(glm::cross(WorldUp, Front));
            Up    = glm::normalize(glm::cross(Front, Right));

            RelativePosition = -Front;
            Position = Center + Distance * RelativePosition;
        }
};

#endif