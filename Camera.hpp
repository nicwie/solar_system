#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


/**
 * @class Camera
 * @brief An abstract camera class that processes input and calculates corresponding Euler Angles,
 * Vectors, and Matrices for use in OpenGL.
 *
 * This class provides functionality for a "fly-through" style camera as well as
 * automated orbiting capabilities. It handles user input for movement and look,
 * and generates the corresponding view matrix.
 *
 * Heavily inspired by the camera class from https://learnopengl.com/
 */
class Camera {
public:
    enum Movement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };

    // Camera attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // Euler angles
    float Yaw;
    float Pitch;

    // Camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // Orbiting
    bool isOrbiting;


    static constexpr float DEFAULT_YAW          = -90.0f;
    static constexpr float DEFAULT_PITCH        = 0.0f;
    static constexpr float DEFAULT_SPEED        = 15.0f;
    static constexpr float DEFAULT_SENSITIVITY  = 0.1f;
    static constexpr float DEFAULT_ZOOM         = 45.0f;

    /**
     * @brief Constructs a camera with specified vectors.
     * @param position The initial position of the camera.
     * @param up The world's up vector.
     * @param yaw The initial yaw angle.
     * @param pitch The initial pitch angle.
     */
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = DEFAULT_YAW, float pitch = DEFAULT_PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(DEFAULT_SPEED), MouseSensitivity(DEFAULT_SENSITIVITY), Zoom(DEFAULT_ZOOM) {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        isOrbiting = false;
        storeInitialState();
        updateCameraVectors();
    }

    /**
    * @brief Constructs a camera with scalar values
    */
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Camera(glm::vec3(posX, posY, posZ), glm::vec3(upX, upY, upZ), yaw, pitch)
    {

    }

    /**
    * @brief Returns view matrix from camera's current state
    * @return The 4x4 view matrix
    */
    glm::mat4 GetViewMatrix() {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void lookAt(glm::vec3 target) {
        /**
         * @brief Forces camera to look at specific target without changing position
         * @param world-space position to look at
         */
        Front = glm::normalize(target - Position);

        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up    = glm::normalize(glm::cross(Right, Front));
    }

    /**
     * @brief Updates camera position to orbit a target.
     * @param target The world-space position of the object to orbit.
     * @param time A time value to drive the orbit animation.
     * @param radius The distance from the target to orbit at.
     * @param speed The speed of the orbit.
     * @param height The height offset from the target's position.
     */
    void upDateOrbit(glm::vec3 earthPos, float time, float radius = 15.0f, float speed = 0.2f, float height = 1.0f) {
            // Calculate the new camera position
            Position.x = earthPos.x + radius * cos(time * speed);
            Position.z = earthPos.z + radius * sin(time * speed);
            Position.y = earthPos.y + height;

            lookAt(earthPos);
    }

    /**
     * @brief Resets camero to inital position, orientation, and zoom
     */
    void Reset() {
        Position = m_initialPosition;
        Yaw = m_initialYaw;
        Pitch = m_initialPitch;
        Zoom = m_initialZoom;
        isOrbiting = false;
        updateCameraVectors();
    }

    /**
     * process input from keyboard-like
     * @param direction The movement direction
     * @param deltaTime time between current and last frame
     */
    void ProcessKeyboard(Movement direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
    }

    /**
     * @brief process input from mouse
     * @param xoffset offset in x direction
     * @param yoffset offset in y direction
     * @constrainPitch if true, pitch will be clamped to avoid flipping screen
     */
    void ProcessMouseMovements(float xoffset, float yoffset, bool constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;


        Yaw += xoffset;
        Pitch += yoffset;

        // Not make screen flip when pitch is out of bounds
        if (constrainPitch) {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        updateCameraVectors();
    }

    /**
     * @brief Process input from a mouse scroll-wheel event
     * @param yoffset vertical scroll offset
     */
    void ProcessMouseScroll(float yoffset) {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

private:
    glm::vec3 m_initialPosition;
    float m_initialYaw;
    float m_initialPitch;
    float m_initialZoom;

    /**
     * @brief Calculates front vector from updated euler angles
     */
    void updateCameraVectors() {
        // calculate new front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);

        // calculate right and up vectors
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }

    /**
    * @brief Stores the current camera state as initial state for later resets
    */
    void storeInitialState() {
        m_initialPosition   = Position;
        m_initialYaw        = Yaw;
        m_initialPitch      = Pitch;
        m_initialZoom       = Zoom;
    }

};

#endif // !CAMERA_H
