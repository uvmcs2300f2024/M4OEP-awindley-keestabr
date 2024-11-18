#include "engine.h"

enum state {start, easy, normal, hard, random_, win, lose};
state screen;

// Colors
color originalFill;

int deathCounter = 0;

Engine::Engine() : keys() {
    this->initWindow();
    this->initShaders();
    this->initShapes();

    originalFill = {1, 0, 0, 1};
}

Engine::~Engine() {}

unsigned int Engine::initWindow(bool debug) {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, false);

    window = glfwCreateWindow(width, height, "engine", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    // OpenGL configuration
    glViewport(0, 0, width, height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glfwSwapInterval(1);

    return 0;
}

void Engine::initShaders() {
    // Load shader manager
    shaderManager = make_unique<ShaderManager>();

    // Load shader into shader manager and retrieve it
    shapeShader = this->shaderManager->loadShader("../res/shaders/shape.vert", "../res/shaders/shape.frag",  nullptr, "shape");

    // Configure text shader and renderer
    textShader = shaderManager->loadShader("../res/shaders/text.vert", "../res/shaders/text.frag", nullptr, "text");
    fontRenderer = make_unique<FontRenderer>(shaderManager->getShader("text"), "../res/fonts/MxPlus_IBM_BIOS.ttf", 24);

    // Set uniforms
    textShader.setVector2f("vertex", vec4(100, 100, .5, .5));
    shapeShader.use();
    shapeShader.setMatrix4("projection", this->PROJECTION);
}

void Engine::initShapes() {
    srand(time(NULL));
    // Red paddle at bottom middle of screen
    paddle = make_unique<Rect>(shapeShader, vec2{width / 2, height / 4}, vec2{200, 15}, color{1, 0, 0, 1});
    // White ball just above paddle
    ball = make_unique<Circle>(shapeShader, vec2{width / 2, height / 3}, 2.25,color{1, 1, 1, 1});
    ball->setVelocity(vec2{0, 0});

    // Create game state for easy
    int x = 950;
    int y = 725;
    // Change color for each subsequent line
    color currColor = color(.7,0,.5,1);
    for (int i = 0; i < 29; ++i) {
        if (x > 25) {
            bricksEasy.push_back(make_unique<Rect>(shapeShader, vec2{x, y}, vec2{85, 40}, currColor));
            x -= 100;
        }
        else {
            // Else block for changing color based off of y position
            y -= 50;
            if (y < 725 && y >= 675) {
                x = 900;
                // Change color for each subsequent line
                currColor = color(.5,.9,0,1);
            }
            if (y < 675 && y >= 625) {
                // Change color for each subsequent line
                currColor = color(0,.5,.7,1);
                x = 950;
            }
            --i;
        }
    }

    // Create game state for normal
    x = 950;
    y = 725;
    // Change color for each subsequent line
    currColor = color(.7,0,.5,1);
    for (int i = 0; i < 38; ++i) {
        if (x > 25) {
            if (i % 2 == 0) {
                bricksNormal.push_back(make_unique<Rect>(shapeShader, vec2{x, y}, vec2{85, 40}, currColor));
            }
            x -= 100;
        }
        else {
            // Else block for changing color based off of y position
            y -= 50;
            if (y < 725 && y >= 675) {
                // Change color for each subsequent line
                currColor = color(.5,.9,0,1);
                x = 900;
            }
            if (y < 675 && y >= 625) {
                // Change color for each subsequent line
                currColor = color(0,.5,.7,1);
                x = 950;
            }
            if (y < 625 && y >= 575) {
                // Change color for each subsequent line
                currColor = color(.7,.3,.7,1);
                x = 900;
            }
            --i;
        }
    }

    // Create game state for hard
    x = 900;
    y = 725;
    currColor = color(.7,0,.5,1);
    for (int i = 0; i < 38; ++i) {
        if (x > 25) {
            bricksHard.push_back(make_unique<Rect>(shapeShader, vec2{x, y}, vec2{85, 40}, currColor));
            x -= 100;
        }
        else {
            // Else block for changing color based off of y position
            y -= 50;
            if (y < 725 && y >= 675) {
                // Change color for each subsequent line
                currColor = color(.5,.9,0,1);
                x = 950;
            }
            if (y < 675 && y >= 625) {
                // Change color for each subsequent line
                currColor = color(0,.5,.7,1);
                x = 900;
            }
            if (y < 625 && y >= 575) {
                // Change color for each subsequent line
                currColor = color(.7,.3,.7,1);
                x = 950;
            }
            --i;
        }
    }

    // Create game state for random
    x = 950;
    y = 725;
    for (int i = 0; i < 40; ++i) {
        if (x > 25) {
            // A one in five chance to add each block to the vector
            if (rand() % 5 == 0) {
                // Add the brick with a random color
                bricksRandom.push_back(make_unique<Rect>(shapeShader, vec2{x, y}, vec2{85, 40}, color(float(rand() % 10 / 10.0), float(rand() % 10 / 10.0), float(rand() % 10 / 10.0),1)));
            }
            x -= 100;
        }
        else {
            // Decrement y position and reset x to the left
            y -= 50;
            x = 950;
            --i;
        }
    }
    // If no bricks at all get added, add one brick
    if (bricksRandom.size() == 0) {
        bricksRandom.push_back(make_unique<Rect>(shapeShader, vec2{500, 750}, vec2{85, 40}, color(float(rand() % 10 / 10.0), float(rand() % 10 / 10.0), float(rand() % 10 / 10.0),1)));
    }
}

void Engine::processInput() {
    glfwPollEvents();
    srand(time(NULL));

    // Set keys to true if pressed, false if released
    for (int key = 0; key < 1024; ++key) {
        if (glfwGetKey(window, key) == GLFW_PRESS)
            keys[key] = true;
        else if (glfwGetKey(window, key) == GLFW_RELEASE)
            keys[key] = false;
    }

    // Close window if escape key is pressed
    if (keys[GLFW_KEY_ESCAPE])
        glfwSetWindowShouldClose(window, true);

    // Mouse position saved to check for collisions
    glfwGetCursorPos(window, &MouseX, &MouseY);

    // If we're in the start screen and press any of the modes; change screen to mode
    if (screen == start) {
        if (keys[GLFW_KEY_E])
            screen = easy;
        if (keys[GLFW_KEY_N])
            screen = normal;
        if (keys[GLFW_KEY_H])
            screen = hard;
        if (keys[GLFW_KEY_R])
            screen = random_;
    }

    // If three deaths you lose and reset blocks for all levels
    if ((screen == easy || screen == normal || screen == hard || screen == random_)
    && deathCounter == 3) {
        screen = lose;
        initShapes();
    }
    // If you win or lose; reset ball and blocks and press p to start over
    if ((screen == lose || screen == win)
        && keys[GLFW_KEY_P]) {
        ball->setVelocity(vec2{0,0});
        ball->setPos(vec2{width / 2, height / 3});
        deathCounter = 0;
        initShapes();
        screen = start;
    }

    // If we're in the play screen and an arrow key is pressed, move the paddle and
    if (screen == easy) {
        float speed = 300.0f * deltaTime;
        // start the ball on press of space
        if (keys[GLFW_KEY_SPACE] && ball->getVelocity() == vec2(0,0)) {
            // add some randomness for angle of start
            if (rand() % 2 == 0) {
                ball->setVelocity(vec2(-(rand() % 150), 300));
            }
            else {
                ball->setVelocity(vec2((rand() % 150), 300));
            }
        }

        // paddle will move left and right with arrow keys at different speeds for each mode
        if (keys[GLFW_KEY_LEFT] && paddle->getLeft() > 0) paddle->moveX(-speed);
        if (keys[GLFW_KEY_RIGHT] && paddle->getRight() < width) paddle->moveX(speed);

    }
    if (screen == normal) {
        // start the ball on press of space
        if (keys[GLFW_KEY_SPACE] && ball->getVelocity() == vec2(0,0)) {
            // add some randomness for angle of start
            if (rand() % 2 == 0) {
                ball->setVelocity(vec2(-(rand() % 200), 450));
            }
            else {
                ball->setVelocity(vec2((rand() % 200), 450));
            }
        }
        float speed = 300.0f * deltaTime;

        // paddle will move left and right with arrow keys at different speeds for each mode
        if (keys[GLFW_KEY_LEFT] && paddle->getLeft() > 0) paddle->moveX(-speed);
        if (keys[GLFW_KEY_RIGHT] && paddle->getRight() < width) paddle->moveX(speed);
    }

    if (screen == hard) {
        // start the ball on press of space
        if (keys[GLFW_KEY_SPACE] && ball->getVelocity() == vec2(0,0)) {
            // add some randomness for angle of start
            if (rand() % 2 == 0) {
                ball->setVelocity(vec2(-(rand() % 250), 550));
            }
            else {
                ball->setVelocity(vec2((rand() % 250), 550));
            }
        }
        float speed = 400.0f * deltaTime;

        // paddle will move left and right with arrow keys at different speeds for each mode
        if (keys[GLFW_KEY_LEFT] && paddle->getLeft() > 0) paddle->moveX(-speed);
        if (keys[GLFW_KEY_RIGHT] && paddle->getRight() < width) paddle->moveX(speed);
    }

    if (screen == random_) {
        // start the ball on press of space
        if (keys[GLFW_KEY_SPACE] && ball->getVelocity() == vec2(0,0)) {
            // add some randomness for angle of start
            if (rand() % 2 == 0) {
                ball->setVelocity(vec2(-(rand() % 300), 550));
            }
            else {
                ball->setVelocity(vec2((rand() % 300), 550));
            }
        }
        float speed = 400.0f * deltaTime;

        // paddle will move left and right with arrow keys at different speeds for each mode
        if (keys[GLFW_KEY_LEFT] && paddle->getLeft() > 0) paddle->moveX(-speed);
        if (keys[GLFW_KEY_RIGHT] && paddle->getRight() < width) paddle->moveX(speed);

    }

    // Mouse position is inverted because the origin of the window is in the top left corner
    MouseY = height - MouseY; // Invert y-axis of mouse position

    if (screen == easy) {
        int counter = 0;
        for (const unique_ptr<Shape> &brick : bricksEasy) {
            if (brick->getPosX() == -1000)
                counter++;
        }
        if (counter == bricksEasy.size()) {
            screen = win;
        }
    }

    if (screen == normal) {
        int counter = 0;
        for (const unique_ptr<Shape> &brick : bricksNormal) {
            if (brick->getPosX() == -1000)
                counter++;
        }
        if (counter == bricksNormal.size()) {
            screen = win;
        }
    }

}

void Engine::checkBounds(unique_ptr<Circle> &ball) const {
    vec2 position = ball->getPos();
    vec2 velocity = ball->getVelocity();
    float bubbleRadius = ball->getRadius();

    position += velocity * deltaTime;

    // If any bubble hits the edges of the screen, bounce it in the other direction
    if (position.x - bubbleRadius <= 0) {
        position.x = bubbleRadius;
        velocity.x = -velocity.x;
    }
    if (position.x + bubbleRadius >= width) {
        position.x = width - bubbleRadius;
        velocity.x = -velocity.x;
    }
    if (position.y - bubbleRadius <= 0) {
        position.x = width / 2;
        position.y = height / 3;
        velocity.x = 0;
        velocity.y = 0;
        deathCounter++;
    }
    if (position.y + bubbleRadius >= height) {
        position.y = height - bubbleRadius;
        velocity.y = -velocity.y;
    }

    ball->setPos(position);
    ball->setVelocity(velocity);
}

void Engine::update() {
    srand(time(NULL));
    // Calculate delta time
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    checkBounds(ball);
    if (screen == easy) {
        if (ball->isOverlappingPaddle(*ball, *paddle)) {
            // add randomness so that the ball might bounce slightly left or right
            ball->setVelocity(vec2{(ball->getVelocity()[0]), -1 * (ball->getVelocity()[1])});
        }
        for(const unique_ptr<Shape> &brick : bricksEasy) {
            if (ball->isOverlappingPaddle(*ball, *brick)) {
                ball->setVelocity(-ball->getVelocity());
                brick->setPos(vec2{-1000,-1000});
            }
        }
    }
    if (screen == normal) {
        if (ball->isOverlappingPaddle(*ball, *paddle)) {
            // add randomness so that the ball might bounce at a slightly different angle
            if (rand() % 2 == 0) {
                ball->setVelocity(vec2{(ball->getVelocity()[0]) - (rand() % 120), -1 * (ball->getVelocity()[1])});
            }
            else {
                ball->setVelocity(vec2{(ball->getVelocity()[0]) + (rand() % 120), -1 * (ball->getVelocity()[1])});
            }
        }
        for(const unique_ptr<Shape> &brick : bricksNormal) {
            if (ball->isOverlappingPaddle(*ball, *brick)) {
                ball->setVelocity(-ball->getVelocity());
                brick->setPos(vec2{-1000,-1000});
            }
        }
    }
    if (screen == hard) {
        if (ball->isOverlappingPaddle(*ball, *paddle)) {
            // add randomness so that the ball might bounce slightly left or right
            ball->setVelocity(vec2{(ball->getVelocity()[0] + 5), -1 * (ball->getVelocity()[1] + 5)});
        }
        for(const unique_ptr<Shape> &brick : bricksHard) {
            if (ball->isOverlappingPaddle(*ball, *brick)) {
                ball->setVelocity(-ball->getVelocity());
                brick->setPos(vec2{-1000,-1000});
            }
        }
    }

    if (screen == random_) {
        if (ball->isOverlappingPaddle(*ball, *paddle)) {
            ball->setVelocity(vec2{(ball->getVelocity()[0]), -1 * (ball->getVelocity()[1])});
        }
        for(const unique_ptr<Shape> &brick : bricksRandom) {
            if (ball->isOverlappingPaddle(*ball, *brick)) {
                ball->setVelocity(-ball->getVelocity());
                brick->setPos(vec2{-1000,-1000});
            }
        }
    }

}

void Engine::render() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color
    glClear(GL_COLOR_BUFFER_BIT);

    // Set shader to draw shapes
    shapeShader.use();

    // Render differently depending on screen
    switch (screen) {
        case start: {
            string message = "Choose a difficulty:";
            string easy = "Easy (e)";
            string normal = "Normal (n)";
            string hard = "Hard (h)";
            string random = "Random (r)";

            // text for each game mode
            this->fontRenderer->renderText(message, width/2 - (12 * message.length()), height - 200, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText(easy, width/2 - (6 * message.length()), height - 300, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText(normal, width/2 - (6 * message.length()), height - 350, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText(hard, width/2 - (6 * message.length()), height - 400, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText(random, width/2 - (6 * message.length()), height - 450, projection, 1, vec3{1, 1, 1});
            break;
        }
        case easy: {
            ball->setUniforms();
            ball->draw();
            paddle->setUniforms();
            paddle->draw();

            for (int i = 0; i < bricksEasy.size(); ++i) {
                bricksEasy[i]->setUniforms();
                bricksEasy[i]->draw();
            }

            string message1 = "Death Counts: " + std::to_string(deathCounter);
            // Display the message on the screen
            this->fontRenderer->renderText(message1, 10, 20, projection, .5, vec3{1, 1, 1});

            string message = "Press space to start";
            if (ball->getVelocity() == vec2(0,0)) {
                this->fontRenderer->renderText(message, width/2 - (12 * message.length()), height/2, projection, 1, vec3{1, 1, 1});
            }
            break;
        }
        case normal: {

            ball->setUniforms();
            ball->draw();
            paddle->setUniforms();
            paddle->draw();

            for (int i = 0; i < bricksNormal.size(); ++i) {
                bricksNormal[i]->setUniforms();
                bricksNormal[i]->draw();
            }

            string message1 = "Death Counts: " + std::to_string(deathCounter);
            // Display the message on the screen
            this->fontRenderer->renderText(message1, 10, 20, projection, .5, vec3{1, 1, 1});

            string message = "Press space to start";
            if (ball->getVelocity() == vec2(0,0)) {
                this->fontRenderer->renderText(message, width/2 - (12 * message.length()), height/2, projection, 1, vec3{1, 1, 1});
            }
            break;
        }
        case hard: {
            ball->setUniforms();
            ball->draw();
            paddle->setUniforms();
            paddle->draw();

            for (int i = 0; i < bricksHard.size(); ++i) {
                bricksHard[i]->setUniforms();
                bricksHard[i]->draw();
            }

            string message1 = "Death Counts: " + std::to_string(deathCounter);
            // Display the message on the screen
            this->fontRenderer->renderText(message1, 10, 20, projection, .5, vec3{1, 1, 1});

            string message = "Press space to start";
            if (ball->getVelocity() == vec2(0,0)) {
                this->fontRenderer->renderText(message, width/2 - (12 * message.length()), height/2, projection, 1, vec3{1, 1, 1});
            }
            break;
        }
        case random_: {
            ball->setUniforms();
            ball->draw();
            paddle->setUniforms();
            paddle->draw();

            for (int i = 0; i < bricksRandom.size(); ++i) {
                bricksRandom[i]->setUniforms();
                bricksRandom[i]->draw();
            }

            string message1 = "Death Counts: " + std::to_string(deathCounter);
            // Display the message on the screen
            this->fontRenderer->renderText(message1, 10, 20, projection, .5, vec3{1, 1, 1});

            string message = "Press space to start";
            if (ball->getVelocity() == vec2(0,0)) {
                this->fontRenderer->renderText(message, width/2 - (12 * message.length()), height/2, projection, 1, vec3{1, 1, 1});
            }
            break;
        }
        case win: {
            string message1 = "You win :)";
            string message2 = "Press p to play again!";
            string message3 = "Press esc to quit!";
            // Display the message on the screen
            this->fontRenderer->renderText(message1, width/2 - (12 * message1.length()), height/2, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText(message2, width/2 - (12 * message2.length()), height/2.5, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText(message3, width/2 - (12 * message3.length()), height/2.75, projection, 1, vec3{1, 1, 1});
            break;
        }
        case lose: {
            string message1 = "You lose :(";
            string message2 = "Press p to play again!";
            string message3 = "Press esc to quit!";
            // Display the message on the screen
            this->fontRenderer->renderText(message1, width/2 - (12 * message1.length()), height/2, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText(message2, width/2 - (12 * message2.length()), height/2.5, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText(message3, width/2 - (12 * message3.length()), height/2.75, projection, 1, vec3{1, 1, 1});
            break;
        }
    }
    glfwSwapBuffers(window);
}

bool Engine::shouldClose() {
    return glfwWindowShouldClose(window);
}

GLenum Engine::glCheckError_(const char *file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        string error;
        switch (errorCode) {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        cout << error << " | " << file << " (" << line << ")" << endl;
    }
    return errorCode;
}