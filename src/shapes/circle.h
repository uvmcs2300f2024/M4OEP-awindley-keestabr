#ifndef GRAPHICS_CIRCLE_H
#define GRAPHICS_CIRCLE_H

#include "shape.h"
#include "../framework/shader.h"
using std::vector, glm::vec2, glm::vec3, glm::normalize, glm::dot;


class Circle : public Shape {
private:

    /// @brief Number of x,y points to draw the circle
    const static int segments = 100;

    /// @brief Radius of the circle (half of screen width
    float radius;
    /// @brief The x and y velocities of the circle
    vec2 velocity;

public:
    /// @brief Construct a new Circle object
    /// @details This is the main constructor for the Circle class.
    /// @details All other constructors call this constructor.
    Circle(Shader &shader, vec2 pos, vec2 size, vec2 velocity, vec4 color)
        : Shape(shader, pos, size, color), radius(size.x / 2.0f), velocity(velocity) {
        initVectors();
        initVAO();
        initVBO();
    }

    Circle(Shader & shader, vec2 pos, vec2 size, struct color color)
        : Circle(shader, pos, size, vec2(0, 0), vec4(color.red, color.green, color.blue, 1.0f)) {}

    Circle(Shader &shader, vec2 pos, float radius, struct color color)
        : Circle(shader, pos, vec2(radius * 2, radius * 2), vec2(0, 0),
                  vec4(color.red, color.green, color.blue, 1.0f)) {}

    Circle(Shader &shader, vec2 pos, float radius, vec2 velocity, vec4 color)
        : Circle(shader, pos, vec2(radius * 2, radius * 2), velocity, color) {}

    // override setUniforms to set the radius uniform
    void setUniforms() const override;

    /// @brief Destroy the Circle object
    /// @details destroys the VAO and VBO associated with the circle
    ~Circle() override;

    /// @brief Draws the circle
    void draw() const override;

    /// @brief Computes the border of the circle, and stores the vertices in the circleVertices array.
    void initVectors();

    /// @brief Returns the radius of the circle
    float getRadius() const;

    /// @brief Sets the radius of the circle
    void setRadius(float radius);

    // --------------------------------------------------------
    // Overloaded functions
    // --------------------------------------------------------
    // Position/Movement Functions
    float getLeft() const override;
    float getRight() const override;
    float getTop() const override;
    float getBottom() const override;


    // Collision Functions

    /// @brief Checks if two circles are overlapping
    /// @details This function is called in Engine's update function to check if any two circles are overlapping.
    bool isOverlapping(const Circle &c) const;

    /// @brief Handles the collision between two circles
    /// @details This function is called when two circles are overlapping (in Engine's update function).
    void bounce(Circle &other);
};


#endif //GRAPHICS_CIRCLE_H
