#include "engine.h"

enum state {start, easy, normal, hard, random_, over};
state screen;

// Colors
color originalFill, hoverFill, pressFill;

//int confettiCounter = 0; // counter for increasing confetti size

Engine::Engine() : keys() {
    this->initWindow();
    this->initShaders();
    this->initShapes();

    originalFill = {1, 0, 0, 1};
    hoverFill.vec = originalFill.vec + vec4{0.5, 0.5, 0.5, 0};
    pressFill.vec = originalFill.vec - vec4{0.5, 0.5, 0.5, 0};
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
    // load shader manager
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
    // red paddle at bottom middle of screen
    paddle = make_unique<Rect>(shapeShader, vec2{width / 2, height / 4}, vec2{200, 10}, color{1, 0, 0, 1});
    // red ball just above paddle
    ball = make_unique<Circle>(shapeShader, vec2{width / 2, height / 2.5}, 2,color{1, 1, 1, 1});
    ball->setVelocity(vec2{10, 100});

    // Create game state for easy
    int x = 950;
    int y = 725;
    color currColor = color(.7,0,.5,1);
    for (int i = 0; i < 29; ++i) {
        if (x > 25) {
            bricksEasy.push_back(make_unique<Rect>(shapeShader, vec2{x, y}, vec2{85, 40}, currColor));
            x -= 100;
        }
        else {
            y -= 50;
            if (y < 725 && y >= 675) {
                x = 900;
                currColor = color(.5,.9,0,1);
            }
            if (y < 675 && y >= 625) {
                currColor = color(0,.5,.7,1);
                x = 950;
            }
            --i;
        }
    }

    // Create game state for normal
    x = 950;
    y = 725;
    currColor = color(.7,0,.5,1);
    for (int i = 0; i < 38; ++i) {
        if (x > 25) {
            if (i % 2 == 0) {
                bricksNormal.push_back(make_unique<Rect>(shapeShader, vec2{x, y}, vec2{85, 40}, currColor));
            }
            x -= 100;
        }
        else {
            y -= 50;
            if (y < 725 && y >= 675) {
                currColor = color(.5,.9,0,1);
                x = 900;
            }
            if (y < 675 && y >= 625) {
                currColor = color(0,.5,.7,1);
                x = 950;
            }
            if (y < 625 && y >= 575) {
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
            y -= 50;
            if (y < 725 && y >= 675) {
                currColor = color(.5,.9,0,1);
                x = 950;
            }
            if (y < 675 && y >= 625) {
                currColor = color(0,.5,.7,1);
                x = 900;
            }
            if (y < 625 && y >= 575) {
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
            if (rand() % 5 == 0) {
                bricksRandom.push_back(make_unique<Rect>(shapeShader, vec2{x, y}, vec2{85, 40}, color(float(rand() % 10 / 10.0), float(rand() % 10 / 10.0), float(rand() % 10 / 10.0),1)));
            }
            x -= 100;
        }
        else {
            y -= 50;
            x = 950;
            --i;
        }
    }
    if (bricksRandom.size() == 0) {
        bricksRandom.push_back(make_unique<Rect>(shapeShader, vec2{500, 750}, vec2{85, 40}, color(float(rand() % 10 / 10.0), float(rand() % 10 / 10.0), float(rand() % 10 / 10.0),1)));
    }
}

void Engine::processInput() {
    glfwPollEvents();

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

    // If we're in the start screen and the user presses s, change screen to play
    // Hint: The index is GLFW_KEY_S
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

    // If we're in the play screen and an arrow key is pressed, move the paddle
    // Hint: one of the indices is GLFW_KEY_UP
    if (screen == easy) {
        float speed = 200.0f * deltaTime;
        // Make sure the paddle cannot go off the screen
        //if (keys[GLFW_KEY_UP] && paddle->getTop() < height) paddle->moveY(speed);
        //if (keys[GLFW_KEY_DOWN] && paddle->getBottom() > 0) paddle->moveY(-speed);

        if (keys[GLFW_KEY_LEFT] && paddle->getLeft() > 0) paddle->moveX(-speed);
        if (keys[GLFW_KEY_RIGHT] && paddle->getRight() < width) paddle->moveX(speed);

    }
    if (screen == normal) {
        float speed = 300.0f * deltaTime;

        if (keys[GLFW_KEY_LEFT] && paddle->getLeft() > 0) paddle->moveX(-speed);
        if (keys[GLFW_KEY_RIGHT] && paddle->getRight() < width) paddle->moveX(speed);
    }

    if (screen == hard) {
        float speed = 400.0f * deltaTime;

        if (keys[GLFW_KEY_LEFT] && paddle->getLeft() > 0) paddle->moveX(-speed);
        if (keys[GLFW_KEY_RIGHT] && paddle->getRight() < width) paddle->moveX(speed);

    }

    if (screen == random_) {
        float speed = 400.0f * deltaTime;

        if (keys[GLFW_KEY_LEFT] && paddle->getLeft() > 0) paddle->moveX(-speed);
        if (keys[GLFW_KEY_RIGHT] && paddle->getRight() < width) paddle->moveX(speed);

    }



    // Mouse position is inverted because the origin of the window is in the top left corner
    MouseY = height - MouseY; // Invert y-axis of mouse position
    //bool buttonOverlapsMouse = paddle->isOverlapping(vec2(MouseX, MouseY));
    //bool mousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    // When in play screen, if the user hovers or clicks on the button then change the paddle's color
    // Hint: look at the color objects declared at the top of this file
    if (screen == easy) {
//        if (buttonOverlapsMouse) {
//            paddle->setColor(hoverFill);
//            if (mousePressed) {
//                paddle->setColor(pressFill);
//            }
//        } else { // Make sure the spawn button is its original color when the user is not hovering or clicking on it.
//            paddle->setColor(originalFill);
//        }

        // When in play screen, if the button was released then spawn confetti
        // Hint: the button was released if it was pressed last frame and is not pressed now
//        if (mousePressedLastFrame && !mousePressed) {
//            spawnConfetti();
//        }
    }
    // Save mousePressed for next frame
    //mousePressedLastFrame = mousePressed;

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
        position.y = bubbleRadius;
        velocity.y = -velocity.y;
    }
    if (position.y + bubbleRadius >= height) {
        position.y = height - bubbleRadius;
        velocity.y = -velocity.y;
    }
    // TODO: make ball bounce if touching top of paddle
//    if (ball->getBottom() == )

    ball->setPos(position);
    ball->setVelocity(velocity);
}

void Engine::update() {
    // Calculate delta time
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    checkBounds(ball);
    if (ball->isOverlappingPaddle(*ball, *paddle)) {
        ball->bounce();
    }
    // End the game when the user spawns 100 confetti
    // If the size of the confetti vector reaches 100, change screen to over
//    if (confetti.size() == 100) {
//        screen = over;
//    }

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
            // (12 * message.length()) is the offset to center text.
            // 12 pixels is the width of each character scaled by 1.
            // NOTE: This line changes the shader being used to the font shader.
            //  If you want to draw shapes again after drawing text,
            //  you'll need to call shapeShader.use() again first.
            this->fontRenderer->renderText(message, width/2 - (12 * message.length()), height - 200, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText(easy, width/2 - (6 * message.length()), height - 300, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText(normal, width/2 - (6 * message.length()), height - 350, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText(hard, width/2 - (6 * message.length()), height - 400, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText(random, width/2 - (6 * message.length()), height - 450, projection, 1, vec3{1, 1, 1});
            break;
        }
        case easy: {
            //  call setUniforms and draw on the paddle and all of the confetti pieces
            //  Hint: make sure you draw the spawn button after the confetti to make it appear on top

//            for (const unique_ptr<Shape>& c : confetti) {
//                c->setUniforms();
//                c->draw();
//            }
            ball->setUniforms();
            ball->draw();
            paddle->setUniforms();
            paddle->draw();

            for (int i = 0; i < bricksEasy.size(); ++i) {
                bricksEasy[i]->setUniforms();
                bricksEasy[i]->draw();
            }
            // Render font on top of spawn button
//            fontRenderer->renderText("Spawn", paddle->getPos().x - 30, paddle->getPos().y - 5, projection, 0.5, vec3{1, 1, 1});
            break;
        }
        case normal: {
            //  call setUniforms and draw on the paddle and all of the confetti pieces
            //  Hint: make sure you draw the spawn button after the confetti to make it appear on top

//            for (const unique_ptr<Shape>& c : confetti) {
//                c->setUniforms();
//                c->draw();
//            }
            ball->setUniforms();
            ball->draw();
            paddle->setUniforms();
            paddle->draw();

            for (int i = 0; i < bricksNormal.size(); ++i) {
                bricksNormal[i]->setUniforms();
                bricksNormal[i]->draw();
            }

            // Render font on top of spawn button
//            fontRenderer->renderText("Spawn", paddle->getPos().x - 30, paddle->getPos().y - 5, projection, 0.5, vec3{1, 1, 1});
            break;
        }
        case hard: {
            //  call setUniforms and draw on the paddle and all of the confetti pieces
            //  Hint: make sure you draw the spawn button after the confetti to make it appear on top

//            for (const unique_ptr<Shape>& c : confetti) {
//                c->setUniforms();
//                c->draw();
//            }
            ball->setUniforms();
            ball->draw();
            paddle->setUniforms();
            paddle->draw();

            for (int i = 0; i < bricksHard.size(); ++i) {
                bricksHard[i]->setUniforms();
                bricksHard[i]->draw();
            }

            // Render font on top of spawn button
//            fontRenderer->renderText("Spawn", paddle->getPos().x - 30, paddle->getPos().y - 5, projection, 0.5, vec3{1, 1, 1});
            break;
        }
        case random_: {
            //  call setUniforms and draw on the paddle and all of the confetti pieces
            //  Hint: make sure you draw the spawn button after the confetti to make it appear on top

//            for (const unique_ptr<Shape>& c : confetti) {
//                c->setUniforms();
//                c->draw();
//            }
            //ball->setUniforms();
            //ball->draw();
            paddle->setUniforms();
            paddle->draw();

            for (int i = 0; i < bricksRandom.size(); ++i) {
                bricksRandom[i]->setUniforms();
                bricksRandom[i]->draw();
            }
            // Render font on top of spawn button
//            fontRenderer->renderText("Spawn", paddle->getPos().x - 30, paddle->getPos().y - 5, projection, 0.5, vec3{1, 1, 1});
            break;
        }
        case over: {
            string message = "You win!";
            // Display the message on the screen
            this->fontRenderer->renderText(message, width/2 - (12 * message.length()), height/2, projection, 1, vec3{1, 1, 1});
            break;
        }
    }

    glfwSwapBuffers(window);
}

//void Engine::spawnConfetti() {
//    ++confettiCounter;
//    vec2 pos = {rand() % (int)width, rand() % (int)height};
//    //  Make each piece of confetti a different size, getting bigger with each spawn.
//    //  The smallest should be a square of size 1 and the biggest should be a square of size 100
//    vec2 size = {confettiCounter, confettiCounter}; // placeholder
//    color color = {float(rand() % 10 / 10.0), float(rand() % 10 / 10.0), float(rand() % 10 / 10.0), 1.0f};
//    confetti.push_back(make_unique<Rect>(shapeShader, pos, size, color));
//}

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