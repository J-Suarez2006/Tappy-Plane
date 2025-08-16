/* Headers */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <vector>  
#include <chrono>
#include <random>
#include <fstream>

using namespace std::string_literals;

// #define SHOW_COLLIDERS

// Screen width and height
constexpr int kScreenWidth {640};
constexpr int kScreenHeight {480};

// Deltatime
Uint64 last_tick {0};
Uint64 current_tick {0};
float delta_time;

/* Global variables */
SDL_Window* globalWindow {nullptr};
SDL_Renderer* globalRenderer {nullptr};

/* Collision Box 2D */
struct CollisionBox2D
{
    float x1;
    float x2;
    float y1;
    float y2;
};

/* Function Prototypes */
SDL_Texture* getTextureFromFile(std::string path);
void spawnRocks();
bool isBetween(float between, float a, float b);
bool isCollided(CollisionBox2D box1, CollisionBox2D box2);
SDL_Texture* createTextSurface(std::string text, SDL_Color textColor, std::string fontPath, int fontSize);

template<typename T>
void clamp(T& toClamp, T minimum, T maximum);

bool isCollided(CollisionBox2D box1, CollisionBox2D box2)
{
    if (isBetween(box1.x1, box2.x1, box2.x2) && isBetween(box1.y1, box2.y1, box2.y2))
    {
        return true;
    }
    else
    {
        return false;
    }
}
// GameObject class
class GameObject
{
    public:
    ~GameObject();
    float myWidth {};
    float myHeight {};

    double degrees {0};
    double rotationSpeed {40};

    float size {1};

    // Physics attributes
    std::vector<float> position {};
    std::vector<float> velocity {};

    bool isVisible {true};

    SDL_Texture* myTexture;

    CollisionBox2D myCollider;
    
    void render();
    void destroy();
    void updatePosition()
    {
        position[0] += velocity[0] * delta_time;
        position[1] += velocity[1] * delta_time;
    }
};
GameObject::~GameObject()
{
    destroy();
}
void GameObject::render()
{
    // Defines center of image and flipmode
    SDL_FPoint center {myWidth / 2.0f, myHeight / 2.0f};
    SDL_FPoint *centerPtr = &center;
    SDL_FlipMode flipmode = SDL_FLIP_NONE;

    // position[0] and position[1] are the x and y position of the Plane object.
    SDL_FRect dstRect{position[0], position[1], static_cast<float>(myWidth), static_cast<float>(myHeight)};
    // SDL_RenderTexture(globalRenderer, myTexture, nullptr, &dstRect);
    if (isVisible){
        SDL_RenderTextureRotated(globalRenderer, myTexture, nullptr, &dstRect, degrees, centerPtr, flipmode);
    
        #ifdef SHOW_COLLIDERS
        //rendering collider
        SDL_FRect colliderRect {myCollider.x1, myCollider.y1, myCollider.x2 - myCollider.x1, myCollider.y2 - myCollider.y1};
        SDL_FRect *cRectPtr {&colliderRect};
        SDL_RenderRect(globalRenderer, cRectPtr);
        #endif
    }
}
void GameObject::destroy()
{
    SDL_DestroyTexture(myTexture);
    myTexture = nullptr;
    myWidth = 0;
    myHeight = 0;
}

// Background class
class Background : public GameObject
{
    public:
    Background();
    ~Background();
};
Background::Background()
{
    myTexture = getTextureFromFile("../../assets/background.png");
    
    myWidth = myTexture->w * size;
    myHeight = myTexture->h * size;

    position = {static_cast<float>((kScreenWidth - myWidth) / 2), static_cast<float>((kScreenHeight - myHeight) / 2)};
    velocity = {0, 0};
    degrees = {0.0};

}
Background::~Background()
{
    destroy();
}

// Text
class TextMessage : public GameObject
{
    public:
    TextMessage();
    ~TextMessage();
    TextMessage(std::string newMessage, int newPointSize, bool visibility);

    std::string fontPath {""};
    std::string message {""};

    SDL_Color myColor;
    int pointSize {};
    
    void updateTexture();
};
TextMessage::TextMessage()
{
    myColor = {
        0x00,
        0x00,
        0x00,
        0xFF
    };

    degrees = 0.0;

    fontPath = "../../lazy.ttf";
    message = "Score: 0";
    pointSize = 28;

    myTexture = createTextSurface(message, myColor, fontPath, pointSize);

    myWidth = myTexture->w;
    myHeight = myTexture->h;

    
    position = {static_cast<float>((kScreenWidth - myWidth) / 2), static_cast<float>(myHeight)};
}
TextMessage::TextMessage(std::string newMessage, int newPointSize, bool visibility)
{
    isVisible = visibility;
    myColor = {
        0x00,
        0x00,
        0x00,
        0xFF
    };

    degrees = 0.0;

    fontPath = "../../lazy.ttf"s;
    message = newMessage;
    pointSize = newPointSize;

    myTexture = createTextSurface(message, myColor, fontPath, pointSize);

    myWidth = myTexture->w;
    myHeight = myTexture->h;

    
    position = {static_cast<float>((kScreenWidth - myWidth) / 2), static_cast<float>((kScreenHeight - myHeight) / 2)};
}
TextMessage::~TextMessage()
{
    destroy();
}
void TextMessage::updateTexture()
{
    myTexture = createTextSurface(message, myColor, fontPath, pointSize);
}

SDL_Texture* createTextSurface(std::string text, SDL_Color textColor, std::string fontPath, int fontSize)
{
    SDL_Texture* texture {nullptr};
    TTF_Font* myFont {nullptr};

    // loading font
    myFont = TTF_OpenFont(fontPath.c_str(), fontSize);
    if (myFont == nullptr)
    {
        SDL_Log("Unable to find font path! SDL error:%s\n", SDL_GetError());
    }
    else
    {
        SDL_Surface* textSurface = TTF_RenderText_Blended(myFont, text.c_str(), 0, textColor);
        if (textSurface == nullptr)
        {
            SDL_Log("Could not render text surface! SDL error:%s\n", SDL_GetError());
        }
        else
        {
            // Create texture from surface
            texture = SDL_CreateTextureFromSurface(globalRenderer, textSurface);
            if (texture == nullptr)
            {
                SDL_Log("Unable to create texture from text surface! SDL error:%s\n", SDL_GetError());
            }
        }
    }
    return texture;
}


// Plane Class
class Plane : public GameObject
{
    public:
    Plane();
    ~Plane();
    
    void accelerate(float x, float y)
    {
        velocity[0] += x * delta_time;
        if (velocity[1] > 0)
        {
            velocity[1] += y * delta_time;
            degrees += rotationSpeed * delta_time;
        }
        else
        {
            velocity[1] += y * delta_time * 1.3;
            degrees -= rotationSpeed * delta_time * 3;
        }
        
        clamp(degrees, -60.0, 60.0);
    }
    void updatePosition()
    {
        position[0] += velocity[0] * delta_time;
        position[1] += velocity[1] * delta_time;

        myCollider.x1 = position[0] + (myWidth / 7);
        myCollider.x2 = position[0] + (6 * myWidth / 7);

        myCollider.y1 = position[1] + (myWidth / 7);
        myCollider.y2 = position[1] + (6 * myHeight / 7);
    }
};
Plane::Plane()
{
    myTexture = getTextureFromFile("../../assets/Planes/PlaneRed1.png"s);
    
    size = .7;
    myWidth = myTexture->w * size;
    myHeight = myTexture->h * size;

    position = {static_cast<float>((kScreenWidth - myWidth) / 2), static_cast<float>((kScreenHeight - myHeight) / 2)};
    velocity = {0, 0};
    degrees = {0.0};

}
Plane::~Plane()
{
    destroy();
}

class Ground : public GameObject
{
    public:
    Ground();
    ~Ground();
};
Ground::Ground()
{
    myTexture = getTextureFromFile("../../assets/groundSnow.png");
    myWidth = myTexture->w * size;
    myHeight = myTexture->h * size;

    position = {static_cast<float>((kScreenWidth - myWidth) / 2), static_cast<float>(kScreenHeight - myHeight)};

    velocity = {0,0};
    degrees = {0.0};

    myCollider.x1 = position[0];
    myCollider.x2 = position[0] + kScreenWidth;

    myCollider.y1 = position[1] + myHeight / 5;
    myCollider.y2 = position[1] + (myHeight);
}
Ground::~Ground()
{
    destroy();
}

enum rockType
{
    dirt,
    grass,
    ice,
    snow
};
// Rock class
class Rock : public GameObject 
{
    public:
    Rock();
    Rock(bool onTop, rockType type, float yShift);

    ~Rock();   
    
    float yOffset {150.0f};
    float size {1.3f};

    void updatePosition()
    {
        position[0] += velocity[0] * delta_time;
        position[1] += velocity[1] * delta_time;

        myCollider.x1 = position[0] + (myWidth / 3);
        myCollider.x2 = position[0] + (2 * myWidth / 3);

        myCollider.y1 = position[1];
        myCollider.y2 = position[1] + (myHeight);
    }
};
Rock::Rock()
{
    myTexture = getTextureFromFile("../../assets/rock.png"s);

    myWidth = myTexture->w * size;
    myHeight = myTexture->h * size;

    position = {static_cast<float>(kScreenWidth / 2), static_cast<float>(kScreenHeight - myHeight)};

    velocity = {-400.0f,0};
    degrees = {0.0};

}
Rock::Rock(bool onTop, rockType type, float yShift)
{
    if (type == dirt)
    {
        myTexture = getTextureFromFile("../../assets/rock.png"s);
    }
    else if (type == grass)
    {
        myTexture = getTextureFromFile("../../assets/rockGrass.png"s);
    }
    else if (type == ice)
    {
        myTexture = getTextureFromFile("../../assets/rockIce.png"s);
    }
    else if (type == snow)
    {
        myTexture = getTextureFromFile("../../assets/rockSnow.png"s);
    }

    myWidth = myTexture->w * size;
    myHeight = myTexture->h * size;
    
    velocity = {0.0f,0.0f};
    
    if (onTop){
        position = {static_cast<float>(kScreenWidth), 0.0f + yShift - yOffset};
        degrees = {180.0};
    }
    else
    {
        position = {static_cast<float>(kScreenWidth), static_cast<float>(kScreenHeight - myHeight) + yShift + yOffset};
        degrees = {0.0};
    }
}
Rock::~Rock()
{
    destroy();
}
/*
void spawnRocks()
{
    float shift {0.0f};

    Rock rock1(false, snow, shift);
    Rock rock2(true, snow, shift);

    rock1.render();
    rock2.render();

    rock1.velocity[0] = -20;
    rock2.velocity[0] = -20;
}
    */

bool isBetween(float between, float a, float b)
{
    bool result {between < a && between > b};
    
    if (result == false)
    {
        result = {between > a && between < b};
    }
    
    return result;
}

template <typename T>
void clamp(T& toClamp, T minimum, T maximum)
{
    if (!std::is_arithmetic_v<T>)
    {
        SDL_Log("Invalid clamp function call - value passed is not arithmetic");
    }
    else
    {
        if (toClamp > maximum)
        {
            toClamp = maximum;
        }
        else if (toClamp < minimum)
        {
            toClamp = minimum;
        }
    }
    
}

SDL_Texture* getTextureFromFile(std::string path)
{
    SDL_Texture* texture {};

    // Load surface
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface == nullptr)
    {
        SDL_Log("Unable to load image%s! SDL image error:%s\n", path.c_str(), SDL_GetError());
        return nullptr;
    }
    else
    {
        // Set texture to surface
        texture = SDL_CreateTextureFromSurface(globalRenderer, loadedSurface);
        if (texture == nullptr)
        {
            SDL_Log("Unable to create texture from loaded surface! SDL error:%s\n", SDL_GetError());
            return nullptr;
        }
    }
    return texture;
}

// Initializing
bool init()
{
    bool success {true};

    if (SDL_Init(SDL_INIT_VIDEO) == false)
    {
        SDL_Log("SDL could not initialize! SDL error: %s\n", SDL_GetError());
        success = false;
    }
    else 
    {
        // Initializing TTF
        if (TTF_Init() == false)
        {
            SDL_Log("TTF could not initialize! SDL error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            // Create window
            if (SDL_CreateWindowAndRenderer("Tappy Plane", kScreenWidth, kScreenHeight, 0, &globalWindow, &globalRenderer) == false)
            {
                SDL_Log("SDL could not create window! SDL error: %s\n", SDL_GetError());
                success = false;
            }
        }
    }

    return success;
}

void close()
{
    // Destroy renderer and window
    SDL_DestroyRenderer(globalRenderer);
    globalRenderer = nullptr;
    SDL_DestroyWindow(globalWindow);
    globalWindow = nullptr;

    // Quit SDL subsystems
    SDL_Quit();
}

int main(int argc, char* args[])
{
    // Final exit code
    int exitCode {0};

    // Initialize 
    if (init() == false)
    {
        SDL_Log("Unable to initialize program!\n");
        exitCode = 1;
    }
    else
    {
        // Quit flag
        bool quit = false;

        // Game over flag
        bool gameOver = false;

        // Event data
        SDL_Event event;
        SDL_zero(event);

        // Vector of gameobjects
        std::vector<GameObject*> allGameObjects;

        // Creating the background
        Background gameBackground;

        // Creating the player
        Plane player;

        // Creating a rock
        Rock rockExample(false, snow, 30.0f);
        Rock rockExample2(true, snow, 30.0f);

        // Creating the ground
        Ground gameGround;

        // Creating scoreboard
        int score {0};
        int finalScore {};
        CollisionBox2D scoreChecker;
        float lastScoreTime{0};
        float timeSinceLastScore{0};

        TextMessage scoreboard;

        // Creating gameOver message
        TextMessage gameOverMessage("Game Over!"s, 40, false);
        TextMessage gameOverInstructions("press space to play again"s, 30, false);
        gameOverInstructions.position[1] = gameOverMessage.position[1] + gameOverMessage.myHeight;

        // Seeding random number generator
        std::mt19937 generator(std::chrono::high_resolution_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<int> distribution(-rockExample.myHeight / 3, rockExample.myHeight / 3);

        // Grabbing high score
        std::ifstream getHighScore {"../../highscore.txt"s};
        int highscore {0};
        getHighScore >> highscore;
        getHighScore.close();

        // Display high score
        TextMessage highScoreMessage("High score: "s, 28, true);
        highScoreMessage.position = {highScoreMessage.myWidth / 10, highScoreMessage.myHeight};

        // Main loop
        while (quit == false)
        {
            // Get event data
            if (SDL_PollEvent(&event) == true)
            {
                // if event is quit type, end main loop
                if (event.type == SDL_EVENT_QUIT)
                {
                    gameOver = true;
                    quit = true;
                }
                else if (event.type == SDL_EVENT_KEY_UP)
                {
                    if (event.key.key == SDLK_UP)
                    {
                        player.velocity[1] = -350.0f;
                    }
                    if (event.key.key == SDLK_LEFT)
                    {
                        SDL_Log("high score: %d", highscore);
                    }
                    if (event.key.key == SDLK_SPACE && gameOver)
                    {
                        gameOver = false;
                        player.position[1] = (kScreenHeight - player.myHeight) / 2;
                        player.velocity[1] = {0.0f};
                        rockExample.position[0] = kScreenWidth;
                        rockExample2.position[0] = kScreenWidth;
                        finalScore = 0;
                        score = 0;
                    }
                }
            }
            
            // update deltatime
            last_tick = current_tick;
            current_tick = SDL_GetTicks();
            delta_time = (current_tick - last_tick) / 1000.0f;

            // Fill the background
            SDL_SetRenderDrawColor(globalRenderer, 0x90, 0xB6, 0xFC, 0xFF);
            SDL_RenderClear(globalRenderer);
            
            // Background
            gameBackground.render();

            // Player
            player.render();
            player.updatePosition();
            player.accelerate(0.0f, 400.0f);
            clamp<float>(player.position[1], 0.0f, kScreenHeight - player.myHeight);

            // Rock
            rockExample.render();
            rockExample.updatePosition();
            rockExample2.render();
            rockExample2.updatePosition();

            // Ground
            gameGround.render();

            // Updating the player and rock according to the gameover flag
            if (!gameOver)
            {
                rockExample.velocity = {-200.0f, 0.0f};
                rockExample2.velocity = {-200.0f, 0.0f};
                rockExample.isVisible = true;
                rockExample2.isVisible = true;
                player.isVisible = true;
                gameOverMessage.isVisible = false;
                gameOverInstructions.isVisible = false;
                
                scoreboard.message = "Score: "s + std::to_string(score);
            }
            else
            {
                rockExample.isVisible = false;
                rockExample2.isVisible = false;
                player.isVisible = false;
                gameOverMessage.isVisible = true;
                gameOverInstructions.isVisible = true;

                scoreboard.message = "Final score: "s + std::to_string(finalScore);
            }

            // checking for collisions
            if (
                !gameOver &&
                (isCollided(player.myCollider, rockExample.myCollider) || 
                isCollided(player.myCollider, rockExample2.myCollider) ||
                isCollided(player.myCollider, gameGround.myCollider))
            )
            {
                SDL_Log("game over! your score was %d", score);
                finalScore = score;
                score = 0;
                gameOver = true;
            }

            if (score > highscore)
            {
                highscore = score;
                SDL_Log("new high score: %d", highscore);
            }

            
            // Checking if it passed the left side of the screen, then respawning it
            if (rockExample.position[0] < -rockExample.myWidth && rockExample2.position[0] < -rockExample2.myWidth)
            {
                float random_number = static_cast<float>(distribution(generator));

                rockExample.position[0] = kScreenWidth + rockExample.myWidth;
                rockExample2.position[0] = kScreenWidth + rockExample2.myWidth;

                // shifts the rock a random amount
                
                rockExample.position[1] = static_cast<float>(kScreenHeight - rockExample.myHeight) + random_number + 150.0f;
                rockExample2.position[1] = random_number - 150.0f;
                
            }

            // Rendering the scoreboard
            scoreboard.updateTexture();
            scoreboard.render();

            // scorechecker and updating the score
            scoreChecker = {rockExample.myCollider.x1, rockExample.myCollider.x2, rockExample.myCollider.y2, rockExample2.myCollider.y1};
            if (isCollided(player.myCollider, scoreChecker) && (timeSinceLastScore > 3 || score == 0))
            {
                lastScoreTime = {current_tick / 1000.0f};
                timeSinceLastScore = {0};
                score++;
                
            }
            timeSinceLastScore = {(current_tick / 1000.0f) - lastScoreTime};

            // The collider display for the collider that checks if the player passed between the rocks
            #ifdef SHOW_COLLIDERS
            SDL_FRect sCheckRect {scoreChecker.x1, scoreChecker.y1, scoreChecker.x2 - scoreChecker.x1, scoreChecker.y2 - scoreChecker.y1};
            SDL_FRect *sCheckPtr {&sCheckRect};
            SDL_RenderRect(globalRenderer, sCheckPtr);
            #endif

            // Rendering the gameover message
            gameOverMessage.render();
            gameOverInstructions.render();

            // Rendering the highscore message
            highScoreMessage.message = "High score:"s + std::to_string(highscore);
            highScoreMessage.updateTexture();
            highScoreMessage.render();

            //spawnRocks();
            // Update the screen
            SDL_RenderPresent(globalRenderer);
        }
        SDL_Log("Quitted!");
        std::ofstream writeHighScore {"../../highscore.txt"s};
        writeHighScore << highscore;
        writeHighScore.close();
    }
    close();
    return exitCode;

}

