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

void Circle::bounce() {
    srand(time(NULL));
    glm::vec2 delta = this->getPos();
    float distance = glm::length(delta);

    float thisMass = this->getRadius() * this->getRadius() * M_PI;
    this->setPos(this->getPos() - (thisMass) * delta / distance);
    glm::vec2 thisVelocity = this->getVelocity();

    float dotProduct = glm::dot(thisVelocity, delta) / (distance * distance);
    glm::vec2 collisionNormal = dotProduct * delta;

    if (rand() % 3 == 0) {
        this->setVelocity(-thisVelocity);
    }
    if (rand() % 3 == 1) {
        this->setVelocity(vec2(-thisVelocity[0] + (rand() % 10),-thisVelocity[1]));
    }
    if (rand() % 3 == 2) {
        this->setVelocity(vec2(-thisVelocity[0] - (rand() % 10),-thisVelocity[1]));
    }
}