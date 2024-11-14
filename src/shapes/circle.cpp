#include "circle.h"
#include "rect.h"


Circle::~Circle() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void Circle::setUniforms() const {
    Shape::setUniforms(); // Sets model and shapeColor uniforms
    shader.setFloat("radius", radius);
    shader.setVector2f("center", pos.x, pos.y);
}

void Circle::draw() const {
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 2); // +2 for center and last vertex
    glBindVertexArray(0);
}

void Circle::initVectors() {
    // Center of circle
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    for (int i = 0; i <= segments; ++i) {
        float theta = 2.0f * 3.1415926f * float(i) / float(segments);
        vertices.push_back(radius * cosf(theta)); // x = r*cos(theta)
        vertices.push_back(radius * sinf(theta)); // y = r*sin(theta)
    }
}

void Circle::setRadius(float radius) {
    this->radius = radius;
    size = vec2(radius * 2, radius * 2);
}

float Circle::getRadius() const { return radius; }

float Circle::getLeft() const   { return pos.x - radius; }
float Circle::getRight() const  { return pos.x + radius; }
float Circle::getTop() const    { return pos.y + radius; }
float Circle::getBottom() const { return pos.y - radius; }

bool Circle::isOverlapping(const Circle &c) const {
    // Check if the distance between the centers of the circles is less than the sum of their radii
    // distance = sqrt((x2 - x1)^2 + (y2 - y1)^2)
    float dist = distance(pos, c.getPos());
    float radiusSum = radius + c.getRadius();
    return dist < radiusSum;
}

bool Circle::isOverlappingPaddle(const Circle &c, const Shape &r) {
    if ((c.getRight() < r.getLeft()) || (r.getRight() < c.getLeft())) {
        return false;
    }
    else if ((c.getBottom() > r.getTop()) || (r.getBottom() > c.getTop())) {
        return false;
    }
    else {
        return true;
    }
}

bool Circle::isOverlapping(const Shape &other) const {
    // Dynamic cast to check if the other shape is a Rect
    const Rect* otherRect = dynamic_cast<const Rect*>(&other);
    if (otherRect) {
        return isOverlapping(*otherRect);
    }
    return false;
}

//bool Circle::isOverlapping(const Rect &other) const {
//    return isOverlappingPaddle(*this, other);
//}

void Circle::bounce() {
    glm::vec2 delta = this->getPos();
    float distance = glm::length(delta);
    //float overlap = 0.5f * (this->getRadius() - distance);

    // Check if circles are overlapping
    // Adjust positions based on radius (as a proxy for mass)
    float thisMass = this->getRadius() * this->getRadius() * M_PI;
    //float otherMass = other.getRadius() * other.getRadius() * M_PI;
    //float totalMass = thisMass + otherMass;

    this->setPos(this->getPos() - (thisMass) * delta / distance);
    //other.setPos(other.getPos() + overlap * (otherMass / totalMass) * delta / distance);

    // Velocity calculations for elastic collision
    glm::vec2 thisVelocity = this->getVelocity();
    //glm::vec2 otherVelocity = other.getVelocity();
    //glm::vec2 velocityDifference = thisVelocity;

    float dotProduct = glm::dot(thisVelocity, delta) / (distance * distance);
    glm::vec2 collisionNormal = dotProduct * delta;

    this->setVelocity(thisVelocity - (thisMass) * collisionNormal);
    //other.setVelocity(otherVelocity + (2 * thisMass / totalMass) * collisionNormal);
}