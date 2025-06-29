#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "game_logic.hpp"

//                          +y  
// towards +z               |  pitch (+x -> +y)   
// slightly down            |  
//	                    	.------- +x
// 	                       /  yaw (+x -> +z)
//                        +z

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

// Default camera values            
const float YAW         = 270.0f;	
const float PITCH       = -45.0f;
const float ZOOM        =  60.0f;	

class Camera
{
    private:
        glm::vec3 Position;
        glm::vec3 Front;    // where the camera is pointing at (it's actually the reverse, because -z is the front); camera direction is then = Position - Front // NOTE: for ortho, should probably be center of Area
        glm::vec3 WorldUp;  // +y in *World* space
        glm::vec3 Right;    // +x in Camera space = WorldUp x Front
        glm::vec3 Up;       // +y in Camera space = Front x Right

        glm::vec3 Center;
        glm::vec3 RelativePosition;

        float Yaw;
        float Pitch;
        float MovementSpeed;
        float MouseSensitivity;
        float Zoom;
        float Distance;

    public:
        Camera() {}
        Camera(Area area) : Yaw(YAW), Pitch(PITCH), Zoom(ZOOM)
        {
            //mode = CAMERA_PERSPECTIVE;
            Center = glm::vec3(area.WIDTH / 2.0f - 0.5f, area.HEIGHT / 2.0f - 0.5f, area.WIDTH / 2.0f - 0.5f);
            RelativePosition = glm::vec3(0.0f, 0.0f, 1.0f);
            Distance = 2.5f * area.WIDTH;
            Position = Center + Distance * RelativePosition;

            WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
            
            updateCameraVectors();
        }

        glm::vec3& getPosition() { return Position; }
        float getYaw() { return Yaw; }

        glm::mat4 getViewMatrix()
        {
            return glm::lookAt(Position, Center, Up); // NOTE: Up bi trebalo zaminit sa WorldUp
        }
        glm::mat4 getProjectionMatrix()
        {
            //if (mode == CAMERA_ORTHO)
            //    return glm::ortho(-12.0f, 12.0f, -12.0f, 12.0f, 0.1f, 100.0f);
            //else
                return glm::perspective(glm::radians(Zoom), 1.0f /* (float)currScrWidth / currScrHeight */, 0.1f, 100.0f); 
                // NOTE: aspect 1.0f works only for resolutions where width = height
        }

        /* void toggleMode()
        {
            if (mode == CAMERA_ORTHO)
            {
                mode = CAMERA_PERSPECTIVE;
                Distance = 2.8f * areaDimensions.x;
            }
            else
            {
                mode = CAMERA_ORTHO;
                Distance = 2 * areaDimensions.x;
            }

            updateCameraVectors(); // Distance is updated => Position needs to be updated
        } */

        // Processes input received from any keyboard-like input system
        void processKeyboard(Camera_Movement direction, float deltaTime)
        {
            float rotSpeed = 6.0f * deltaTime;

            if (direction == FORWARD)
            {
                Pitch -= rotSpeed * 10.0f;
                if (Pitch > 89.0f)
                    Pitch = 89.0f;
                else if (Pitch < -89.0f)
                    Pitch = -89.0f;
            }
            if (direction == BACKWARD)
            {
                Pitch += rotSpeed * 10.0f;
                if (Pitch > 89.0f)
                    Pitch = 89.0f;
                else if (Pitch < -89.0f)
                    Pitch = -89.0f;
            }
            if (direction == LEFT)
            {
                Yaw += rotSpeed * 10.0f;
                if (Yaw >= 360.0f)
                    Yaw -= 360.0f;
            }
            if (direction == RIGHT)
            {
                    Yaw -= rotSpeed * 10.0f;
                    if (Yaw < 0.0f)
                        Yaw += 360.0f;
            }

            updateCameraVectors();
        }

    private:
        // Calculate the front vector from the Camera's updated Euler angles
        void updateCameraVectors()
        {
            glm::vec3 front;
            front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
            front.y = -sin(glm::radians(Pitch));
            front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
            Front = glm::normalize(front);

            Right = glm::normalize(glm::cross(WorldUp, Front));
            Up    = glm::normalize(glm::cross(Front, Right));

            RelativePosition = Front;
            Position = Center + Distance * RelativePosition;

            std::cout << "Position: (" << Position.x << ", " << Position.y << ", " << Position.z << ")" << std::endl
                    << "Center: (" << Center.x << ", " << Center.y << ", " << Center.z << ")" << std::endl
                    << "RelativePosition: (" << RelativePosition.x << ", " << RelativePosition.y << ", " << RelativePosition.z << ")" << std::endl
                    << "Right: (" << Right.x << ", " << Right.y << ", " << Right.z << ")" << std::endl
                    << "Up: (" << Up.x << ", " << Up.y << ", " << Up.z << ")" << std::endl
                    << "Front: (" << Front.x << ", " << Front.y << ", " << Front.z << ")" << std::endl
                    << "Yaw: " << Yaw << std::endl
                    << "Pitch: " << Pitch << std::endl;
        }
};

#endif