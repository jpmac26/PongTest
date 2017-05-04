#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_system.h>
#include <SDL2/SDL_types.h>
#include <SDL2/SDL_video.h>


//Function prototypes
void processInput();
void updateBall();


//Variable declarations
bool done = false;
SDL_Window *window;
SDL_Renderer *sdlRenderer;
SDL_DisplayMode dispMode;

SDL_Rect player1;
SDL_Rect player2;
SDL_Rect ball;

int ballSpeedX;
int ballSpeedY;


int main()
{
    // initialize SDL video
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Unable to init SDL: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("PongTest", 0, 0, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_OPENGL);
    //gl = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(1);
    sdlRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_Init(SDL_INIT_VIDEO);

    // Get current display mode of all displays.
    for (int i = 0; i < SDL_GetNumVideoDisplays(); ++i)
    {
        int should_be_zero = SDL_GetCurrentDisplayMode(i, &dispMode);

        if (should_be_zero != 0)
        {
            // In case of error...
            SDL_Log("Could not get display mode for video display #%d: %s", i, SDL_GetError());
        }
        else
        {
            // On success, print the current display mode.
            SDL_Log("Display #%d: current display mode is %dx%dpx @ %dhz.", i, dispMode.w, dispMode.h, dispMode.refresh_rate);
        }
    }


    SDL_Event event;

    ballSpeedX = (dispMode.w / 90);
    ballSpeedY = 0;

    // make sure SDL cleans up before exit
    atexit(SDL_Quit);

    // create the player paddles and ball
    player1.w = 32;
    player1.h = 128;
    player1.x = (dispMode.w - player1.w) / 8;
    player1.y = (dispMode.h - player1.h) / 2;

    player2.w = 32;
    player2.h = 128;
    player2.x = (dispMode.w - player2.w) / 8 * 7;
    player2.y = (dispMode.h - player2.h) / 2;

    ball.w = 32;
    ball.h = 32;
    ball.x = (dispMode.w - ball.w) / 2;
    ball.y = (dispMode.h - ball.h) / 2;

    double timeStepMs = 1000.0f / dispMode.refresh_rate;
    double timeCurrentMs = 0.0f, timeLastMs = 0.0f, timeDeltaMs = 0.0f, timeAccumulatedMs = 0.0f;


    // program main loop
    while (!done)
    {
        // Event processing loop
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                done = true;
            }
        }

        if (done) break;


        // Process timing updates
        timeLastMs = timeCurrentMs;
        timeCurrentMs = SDL_GetTicks();
        timeDeltaMs = timeCurrentMs - timeLastMs;
        timeAccumulatedMs += timeDeltaMs;

        // Update everything
        while (timeAccumulatedMs >= timeStepMs)
        {
              processInput();
              updateBall();
              timeAccumulatedMs -= timeStepMs;

              SDL_PumpEvents();
        }


        /////////////////////
        //DRAWING STARTS HERE
        //===================
        
        //Clear the screen to black
        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
        SDL_RenderClear(sdlRenderer);

        //Draw players and ball
        SDL_SetRenderDrawColor(sdlRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderFillRect(sdlRenderer, &player1);
        SDL_RenderFillRect(sdlRenderer, &player2);
        SDL_RenderFillRect(sdlRenderer, &ball);
        
        //Finally, update the screen
        SDL_RenderPresent(sdlRenderer);

        ///////////////////
        //DRAWING ENDS HERE
        //=================
    }


    //Clean everything up
    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    printf("Exited cleanly.\n");
    return 0;
}


void processInput()
{
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);


    // check for keypresses
    if (keystate[SDL_SCANCODE_ESCAPE])
    {
        done = true;
        return;
    }


    if (keystate[SDL_SCANCODE_RETURN])
    {
        ball.w = 32;
        ball.h = 32;
        ball.x = (dispMode.w - ball.w) / 2;
        ball.y = (dispMode.h - ball.h) / 2;
    }


    if (keystate[SDL_SCANCODE_W] && (player1.y - dispMode.h / 90) >= 0)
    {
        player1.y -= dispMode.h / 90;
    }
    else if (keystate[SDL_SCANCODE_S] && (player1.y + player1.h + dispMode.h / 90) <= dispMode.h)
    {
        player1.y += dispMode.h / 90;
    }

    if (keystate[SDL_SCANCODE_I] && (player2.y - dispMode.h / 90) >= 0)
    {
        player2.y -= dispMode.h / 90;
    }
    else if (keystate[SDL_SCANCODE_K] && (player2.y + player2.h + dispMode.h / 90) <= dispMode.h)
    {
        player2.y += dispMode.h / 90;
    }
}

void updateBall()
{
    ball.x += ballSpeedX;
    ball.y += ballSpeedY;
    // we need to constantly reset this because otherwise the ball will shrink when it overlaps the top/bottom of the screen
    ball.h = 32;

    // check for collision with top/bottom of screen
    if (ball.y <= 0 || ball.y + ball.h >= dispMode.h)
    {
        ballSpeedY = -ballSpeedY;
    }

    // check for collision with player1's paddle
    if (ball.y + ball.h >= player1.y && ball.y <= player1.y + player1.h)
    {
        if (ball.x <= player1.x + player1.w && ball.x + ball.w >= player1.x)
        {
            if (ballSpeedX < 0)
            {
                ballSpeedX = -ballSpeedX;
                int oldBallSpeedX = ballSpeedX;
                int oldBallSpeedY = ballSpeedY;
                ballSpeedY += ((ball.y + (ball.h / 2)) - (player1.y + (player1.h / 2))) / 32;
                ballSpeedX += abs(oldBallSpeedY) - abs(ballSpeedY);

                if (ballSpeedX == 0)
                {
                    ballSpeedX = oldBallSpeedX;
                    ballSpeedY = oldBallSpeedY;
                }
            }
        }
    }

    // check for collision with player2's paddle
    if (ball.y + ball.h >= player2.y && ball.y <= player2.y + player2.h)
    {
        if (ball.x <= player2.x + player2.w && ball.x + ball.w >= player2.x)
        {
            if (ballSpeedX > 0)
            {
                ballSpeedX = -ballSpeedX;
                int oldBallSpeedX = ballSpeedX;
                int oldBallSpeedY = ballSpeedY;
                ballSpeedY += ((ball.y + (ball.h / 2)) - (player2.y + (player2.h / 2))) / 32;
                ballSpeedX -= abs(oldBallSpeedY) - abs(ballSpeedY);

                if (ballSpeedX == 0)
                {
                    ballSpeedX = oldBallSpeedX;
                    ballSpeedY = oldBallSpeedY;
                }
            }
        }
    }
}