#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>
#include <random>
#include <sstream>
#include <cassert>
#include <fstream>

#include "StrafeMath.h"
#include "Animations.h"
#include "DrawUtil.h"
#include "Settings.h"
#include "MouseSharedDefs.h"
#include "convars.h"

int cur_anim = 0;
const double pt_smoothing = 0.7;

typedef bool (*sfFuncPtr)(sf::RenderTarget& window);
static sfFuncPtr ANIM_ARRAY[] =
{ 
  Animations::PerfAngleDemo,
  Animations::WishVelDemonstration,
  Animations::End
};

static const int NUM_ANIMS = sizeof(ANIM_ARRAY) / sizeof(ANIM_ARRAY[0]);

inline void SetMagnitude(Eigen::Vector2d& vec, double magnitude)
{
    double old_magnitude = std::sqrtf(vec[0] * vec[0] + vec[1] * vec[1]);
    vec[0] = (vec[0] * magnitude) / old_magnitude;
    vec[1] = (vec[1] * magnitude) / old_magnitude;
}

static void ActivatePoint(const sf::RenderTarget& window) 
{
    const Eigen::Vector2d p = DrawUtil::PixelsToWorld(window, mouse.pos * settings.video.render_scale);
    for (int i = 0; i < 2; ++i) 
    {
        const double diff = (Animations::moveablePts[i] - p).norm();
        if (diff * DrawUtil::scale < 10.0 * settings.video.render_scale)
        {
            mouse.select = i;
            return;
        }
    }
    mouse.select = -1;
}

inline double VecMagnitude(Eigen::Vector2d& vec)
{
    return std::sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
// windows main
INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) 
{
#else
int main(int argc, char *argv[]) {
#endif
    // GL-settings
    sf::ContextSettings cSettings;
    cSettings.depthBits = 24;
    cSettings.stencilBits = 8;
    cSettings.antialiasingLevel = 16;
    cSettings.majorVersion = 2;
    cSettings.minorVersion = 0;

    sf::VideoMode screenSize = sf::VideoMode::getDesktopMode();
    screenSize = sf::VideoMode(settings.video.start_w, settings.video.start_h, screenSize.bitsPerPixel);
    DrawUtil::render_scale = float(settings.video.render_scale) * 0.5f;

    // Create the window
    sf::RenderWindow window;
    sf::Uint32 window_style = (settings.video.start_fullscreen ? sf::Style::Fullscreen : sf::Style::Resize | sf::Style::Close);
    window.create(screenSize, "Strafe Visualizer by spicy", window_style, cSettings);
    window.setFramerateLimit(60);
    window.requestFocus();
    sf::View view = window.getDefaultView();

    // Setup OpenGL things
    glHint(GL_POINT_SMOOTH, GL_NICEST);
    glHint(GL_LINE_SMOOTH, GL_NICEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_SMOOTH);

    // Create the render texture 4 times larger than the window
    sf::RenderTexture renderTexture;
    renderTexture.create(window.getSize().x * settings.video.render_scale, window.getSize().y * settings.video.render_scale, cSettings);
    renderTexture.setSmooth(true);
    renderTexture.setActive(true);

    // Create and set up the player
    StrafeMath::player = new Player;

    DrawUtil::center = Eigen::Vector2d::Zero();
    DrawUtil::scale = 75 * settings.video.render_scale;

    Animations::unfilteredPts[0] = Eigen::Vector2d(1, 0);
    Animations::unfilteredPts[1] = Eigen::Vector2d(1, 0);

    for (int i = 0; i < 2; ++i)
    {
        Animations::moveablePts[i] = Animations::unfilteredPts[i];
    }

    bool screenshot = false;
    while (window.isOpen()) 
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
                break;
            }
            else if (event.type == sf::Event::KeyPressed)
            {
                sf::Keyboard::Key keycode = event.key.code;
                if (keycode == sf::Keyboard::Escape)
                {
                    window.close();
                    break;
                }
                else if (keycode == sf::Keyboard::Space)
                {
                    if (cur_anim > 0)
                    {
                        Animations::animate_out = true;
                    }
                    else
                    {
                        cur_anim += 1;
                    }
                }
                else if (keycode == sf::Keyboard::F1) 
                {
                    screenshot = true;
                }

                // Create local vars to handle counter strafing (W+S/A+D)
                double fwdmv = 0, sidemv = 0;

                switch (keycode)
                {
                case sf::Keyboard::W:
                    fwdmv += sv_walkspeed;
                    StrafeMath::player->forwardMove = fwdmv;
                    break;
                case sf::Keyboard::S:
                    fwdmv -= sv_walkspeed;
                    StrafeMath::player->forwardMove = fwdmv;
                    break;
                case sf::Keyboard::A:
                    sidemv -= sv_walkspeed;
                    StrafeMath::player->sideMove = sidemv;
                    break;
                case sf::Keyboard::D:
                    sidemv += sv_walkspeed;
                    StrafeMath::player->sideMove = sidemv;
                    break;
                }
            }
            else if (event.type == sf::Event::KeyReleased)
            {
                sf::Keyboard::Key keycode = event.key.code;

                // Create local vars to handle counter strafing (W+S/A+D)
                switch (keycode)
                {
                case sf::Keyboard::W:
                    StrafeMath::player->forwardMove = 0;
                    break;
                case sf::Keyboard::S:
                    StrafeMath::player->forwardMove = 0;
                    break;
                case sf::Keyboard::A:
                    StrafeMath::player->sideMove = 0;
                    break;
                case sf::Keyboard::D:
                    StrafeMath::player->sideMove = 0;
                    break;
                }
            }
            else if (event.type == sf::Event::Resized)
            {
                const sf::FloatRect visibleArea(0, 0, (float)event.size.width, (float)event.size.height);
                window.setView(sf::View(visibleArea));
                renderTexture.create(window.getSize().x * settings.video.render_scale, window.getSize().y * settings.video.render_scale, cSettings);
                renderTexture.setSmooth(true);
                renderTexture.setActive(true);
            }
            else if (event.type == sf::Event::MouseMoved)
            {
                 mouse.pos = sf::Vector2i(event.mouseMove.x, event.mouseMove.y);
            }
            else if (event.type == sf::Event::MouseButtonPressed)
            {
                mouse.pos = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
                mouse.pressed = true;
                ActivatePoint(renderTexture);
            }
            else if (event.type == sf::Event::MouseButtonReleased)
            {
                mouse.pos = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
                mouse.pressed = false;
                mouse.select = -1;
            }
        }

        // Get the mouse active point
        if (mouse.select >= 0)
        {
            Animations::unfilteredPts[mouse.select] = DrawUtil::PixelsToWorld(renderTexture, mouse.pos * settings.video.render_scale);
        }

        // Filter points
        for (int i = 0; i < 2; ++i) 
        {
            Animations::moveablePts[i] *= pt_smoothing;
            Animations::moveablePts[i] += Animations::unfilteredPts[i] * (1.0 - pt_smoothing);
        }

        // Cap the second moveablepoint (eyeangles) to the length 3
        SetMagnitude(Animations::moveablePts[1], 3);

        // Set the players viewangles corresponding to the point normalized
        StrafeMath::player->viewAngles[0] = Animations::moveablePts[1][0] / VecMagnitude(Animations::moveablePts[1]);
        StrafeMath::player->viewAngles[1] = -Animations::moveablePts[1][1] / VecMagnitude(Animations::moveablePts[1]);


        // Draw the background
        renderTexture.setActive(true);
        Animations::Background(renderTexture, cur_anim >= 0);

        // Draw the foreground
        if (cur_anim > 0 && cur_anim <= NUM_ANIMS && ANIM_ARRAY[cur_anim - 1](renderTexture)) 
        {
           Animations::animate_out = false;
           cur_anim += 1;
        }

        // Finish drawing to the texture
        renderTexture.display();

        // Draw texture to window
        window.setActive(true);
        const sf::Texture& texture = renderTexture.getTexture();
        sf::Sprite sprite(texture);
        sprite.setScale(1.0f / float(settings.video.render_scale), 1.0f / float(settings.video.render_scale));
        window.draw(sprite);

        if (screenshot) 
        {
            texture.copyToImage().saveToFile("visualizer.png");
            screenshot = false;
        }

        //Flip the screen buffer
        window.display();
        Animations::frame += 1;
    }

    return 0;
}
