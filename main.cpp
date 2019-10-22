#include "common.h"
#include "cmath"
#include "OpenSimplexNoise.h"

bool Init();
void CleanUp();
void Run();
double ScaleNum(double n, double minN, double maxN, double min, double max);
void NewParticles();
void Particle();

SDL_Window *window;
SDL_GLContext glContext;
SDL_Surface *gScreenSurface = nullptr;
SDL_Renderer *renderer = nullptr;

int screenWidth = 500;
int screenHeight = 500;
double resolution = 1;
double featureSize = 20;
double zoff = 0;
int pSize = 3;
int gridSize = 10;
int numParticles = 1000;
double vectMag = .01;
double maxSpeed = 1;
OpenSimplexNoise *noise1 = nullptr;
OpenSimplexNoise *noise2 = nullptr;
const double PI = 3.1415926;

vector<vector<double>> vectors;
vector<vector<double>> particles;

bool Init()
{
    if (SDL_Init(SDL_INIT_NOPARACHUTE & SDL_INIT_EVERYTHING) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s\n", SDL_GetError());
        return false;
    }
    else
    {
        //Specify OpenGL Version (4.2)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_Log("SDL Initialised");
    }

    //Create Window Instance
    window = SDL_CreateWindow(
        "Game Engine",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        screenWidth,
        screenHeight,   
        SDL_WINDOW_OPENGL);

    //Check that the window was succesfully created
    if (window == NULL)
    {
        //Print error, if null
        printf("Could not create window: %s\n", SDL_GetError());
        return false;
    }
    else{
        gScreenSurface = SDL_GetWindowSurface(window);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        SDL_Log("Window Successful Generated");
    }
    //Map OpenGL Context to Window
    glContext = SDL_GL_CreateContext(window);

    return true;
}

int main()
{
    //Error Checking/Initialisation
    if (!Init())
    {
        printf("Failed to Initialize");
        return -1;
    }

    // Clear buffer with black background
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    //Swap Render Buffers
    SDL_GL_SwapWindow(window);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    Run();

    CleanUp();
    return 0;
}

void CleanUp()
{
    //Free up resources
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Run()
{
    bool gameLoop = true;
    srand(time(NULL));
    long rand1 = rand() * (RAND_MAX + 1) + rand();
    srand(time(NULL));
    long rand2 = rand() * (RAND_MAX + 2) + rand();
    noise1 = new OpenSimplexNoise{rand1};
    noise2 = new OpenSimplexNoise{rand2};
    
    NewParticles();
    SDL_Rect pos;
    pos.x = 0;
    pos.y = 0;
    pos.w = screenWidth;
    pos.h = screenHeight;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &pos);

    while (gameLoop)
    {   
        Particle();
        SDL_RenderPresent(renderer);

        zoff += .1;
        SDL_Rect pos;
        pos.x = 0;
        pos.y = 0;
        pos.w = screenWidth;
        pos.h = screenHeight;
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &pos);

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                gameLoop = false;
            }
            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    gameLoop = false;
                    break;
                default:
                    break;
                }
            }

            if (event.type == SDL_KEYUP)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_w:
                    featureSize++;
                    break;
                case SDLK_s:
                    featureSize--;
                    break;
                case SDLK_SPACE:
                    rand1 = rand() * (RAND_MAX + 1) + rand();
                    noise1 = new OpenSimplexNoise{rand1};
                default:
                    break;
                }
            }
        }
    }
}

void NewParticles(){
    vector<double> parts;
    for(int i = 0; i < numParticles; i++){
        parts.push_back(rand() % screenWidth); //X
        parts.push_back(rand() % screenHeight); //Y
        parts.push_back(0); //vel
        parts.push_back(0); //velAng
        parts.push_back(0); //acc
        parts.push_back(0); //accAng

        particles.push_back(parts);
        parts.clear();
    }
}

void Particle(){
    vector<double> pos;
    for(int i = 0; i < particles.size(); i++){
        pos.clear();

        particles[i][0] += particles[i][2] * cos(particles[i][3]);
        particles[i][1] += particles[i][2] * sin(particles[i][3]);

        if(particles[i][0] > screenWidth){
            particles[i][0] = 0;
        }
        if(particles[i][0] < 0){
            particles[i][0] = screenWidth;
        }
        if(particles[i][1] > screenHeight){
            particles[i][1] = 0;
        }
        if(particles[i][1] < 0){
            particles[i][1] = screenHeight;
        }

        int col;
        for(int indx = 0; indx < screenWidth / gridSize; indx++){
            if(particles[i][0] <= indx * gridSize + gridSize){
                col = indx;
                break;
            }
        }
        int row;
        for(int indy = 0; indy < screenHeight / gridSize; indy++){
            if(particles[i][1] <= indy * gridSize + gridSize){
                row = indy;
                break;
            }
        }
        double ang = ((*noise2).eval(col/featureSize, row/featureSize, zoff/featureSize) + 1) * PI/2 + PI/2;

        double accX = vectMag * cos(ang);
        double accY = vectMag * sin(ang);
        particles[i][4] = sqrt(accX * accX + accY * accY);
        particles[i][5] = atan2(accY, accX);

        double velx = particles[i][2] * cos(particles[i][3]);
        double vely = particles[i][2] * sin(particles[i][3]);
        
        if((velx >= 0 && particles[i][4] * cos(particles[i][5]) < 0) || (velx < 0 && particles[i][4] * cos(particles[i][5]) >= 0))
            velx +=  particles[i][4] * cos(particles[i][5]);
        else
            velx += particles[i][4] * cos(particles[i][5]) * (-1 * pow(particles[i][2] * cos(particles[i][3]), 2) + 1);
        
        if((vely >= 0 && particles[i][4] * sin(particles[i][5]) < 0) || (vely < 0 && particles[i][4] * sin(particles[i][5]) >= 0))
            vely +=  particles[i][4] * sin(particles[i][5]);
        else
            vely += particles[i][4] * sin(particles[i][5]) * (-1 * pow(particles[i][2] * sin(particles[i][3]), 2) + 1);

        if(sqrt(velx * velx + vely * vely) > maxSpeed)
            particles[i][2] = maxSpeed;
        else
            particles[i][2] = sqrt(velx * velx + vely * vely);
        particles[i][3] = atan2(vely, velx);

        SDL_Rect pos;
        pos.x = particles[i][0];
        pos.y = particles[i][1];
        pos.w = pSize;
        pos.h = pSize;
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &pos);

    }
}

double ScaleNum(double n, double minN, double maxN, double min, double max){
    return (((n - minN) / (maxN - minN)) * (max - min)) + min;
}