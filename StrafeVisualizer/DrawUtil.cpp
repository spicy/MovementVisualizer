#define NOGDI
#include "DrawUtil.h"
#include <SFML/OpenGL.hpp>

#ifndef M_PI 
#define M_PI 3.1415926535 
#endif

Eigen::Vector2d DrawUtil::center = Eigen::Vector2d::Zero();
double DrawUtil::scale = 75;
float DrawUtil::render_scale = 1.0f;


sf::Vector2f DrawUtil::ToSF(const Eigen::Vector2d& v)
{
    return sf::Vector2f((float)v.x(), (float)v.y());
}


Eigen::Vector2d DrawUtil::HalfSize(const sf::RenderTarget& window)
{
    return Eigen::Vector2d((double)window.getSize().x, (double)window.getSize().y) * 0.5;
}


void DrawUtil::DrawCircle(sf::RenderTarget& window, const Eigen::Vector2d& c, const sf::Color& color, double r, double thickness) 
{
    sf::CircleShape circle(float(r), 200);
    circle.setOrigin(circle.getRadius(), circle.getRadius());
    circle.setFillColor(sf::Color::Transparent);
    circle.setOutlineColor(color);
    circle.setOutlineThickness((float)thickness);
    circle.setPosition(ToSF((c - center)*scale + HalfSize(window)));
    window.draw(circle);
}


void DrawUtil::DrawRect(sf::RenderTarget& window, const float x, const float y, const sf::Vector2f& size, const sf::Color& color)
{
    sf::RectangleShape rect(size);
    rect.setPosition(x, y);
    rect.setFillColor(color);
    window.draw(rect);
}


void DrawUtil::DrawLine(sf::RenderTarget& window, const Eigen::Vector2d& _a, const Eigen::Vector2d& _b, const sf::Color& color, bool extend, double thickness)
{
    std::vector<sf::Vertex> vertex_array(2);
    Eigen::Vector2d a, b;
    if (extend) 
    {
        a = _a + (_a - _b).normalized()*100.0;
        b = _b + (_b - _a).normalized()*100.0;
    } 
    else 
    {
        a = _a;
        b = _b;
    }
    vertex_array[0] = sf::Vertex(ToSF((a - center)*scale + HalfSize(window)), color);
    vertex_array[1] = sf::Vertex(ToSF((b - center)*scale + HalfSize(window)), color);
    glLineWidth((float)thickness * render_scale);
    window.draw(vertex_array.data(), vertex_array.size(), sf::PrimitiveType::Lines);
}


void DrawUtil::DrawPoint(sf::RenderTarget& window, const Eigen::Vector2d& c, const sf::Color& color, double radius) 
{
    sf::CircleShape circle((float)radius * render_scale);
    circle.setOrigin(circle.getRadius(), circle.getRadius());
    circle.setFillColor(color);
    circle.setPosition(ToSF((c - center)*scale + HalfSize(window)));
    window.draw(circle);
}


void DrawUtil::DrawTextSF(sf::RenderTarget& window, const float x, const float y, sf::Font font, sf::String& string, int pixelSize, const sf::Color& color)
{
    sf::Text text;
    text.setFont(font);
    text.setString(string);
    text.setCharacterSize(pixelSize);
    text.setFillColor(color);
    text.setStyle(sf::Text::Style::Regular);
    text.setPosition(x, y);

    // inside the main loop, between window.clear() and window.display()
    window.draw(text);
}


void DrawUtil::DrawTextSF(sf::RenderTarget& window, const Eigen::Vector2d& point, sf::Font font, sf::String& string, int pixelSize, const sf::Color& color)
{
    sf::Text text;
    text.setFont(font);
    text.setString(string);
    text.setCharacterSize(pixelSize);
    text.setFillColor(color);
    text.setStyle(sf::Text::Style::Regular);
    text.setPosition(ToSF((point - center) * scale + HalfSize(window)));

    // inside the main loop, between window.clear() and window.display()
    window.draw(text);
}


void DrawUtil::DrawGrid(sf::RenderTarget& window, double tolerance)
{
    window.clear(sf::Color(0, 0, 0));

    const int min_x = int(center.x() - 0.5 * double(window.getSize().x) / scale) - 1;
    const int max_x = int(center.x() + 0.5 * double(window.getSize().x) / scale) + 1;
    const int min_y = int(center.y() - 0.5 * double(window.getSize().y) / scale) - 1;
    const int max_y = int(center.y() + 0.5 * double(window.getSize().y) / scale) + 1;

    for (int x = min_x; x < max_x; ++x)
    {
        for (int i = 0; i < 3; ++i) 
        {
            const double q = 0.25 * double((x - min_x)*4 + i) / double(max_x - min_x);
            const double bounce = SmoothBounce(tolerance, q*0.5 + 0.5, 20.0);

            if (bounce <= 0.0) { continue; }

            const Eigen::Vector2d a((double)x + i*0.33, (double)min_y);
            const Eigen::Vector2d b((double)x + i*0.33, (double)max_y);

            if (i == 0)
            {
                DrawLine(window, a, b, sf::Color(255, 255, 255, 128), false, 2.0 * bounce);
            } 
            else 
            {
                DrawLine(window, a, b, sf::Color(255, 255, 255, 64), false, bounce);
            }
        }
    }

    for (int y = min_y; y < max_y; ++y) 
    {
        for (int i = 0; i < 3; ++i) 
        {
            const double q = 0.33 * double((y - min_y) * 4 + i) / double(max_y - min_y);
            const double bounce = SmoothBounce(tolerance, q * 0.5, 20.0);

            if (bounce <= 0.0) { continue; }
            const Eigen::Vector2d a((double)min_x, (double)y + i * 0.33);
            const Eigen::Vector2d b((double)max_x, (double)y + i * 0.33);
            if (i == 0)
            {
                DrawLine(window, a, b, sf::Color::White, false, 2.0*bounce);
            } 
            else 
            {
                DrawLine(window, a, b, sf::Color(255, 255, 255, 128), false, bounce);
            }
        }
    }
}


//void DrawUtil::DrawKeypresses(sf::RenderTarget& window, Eigen::Vector2d& point, sf::Font font, int pixelSize, )
//{
//    Eigen::Vector2d wPoint(point[0] + 10, point[1] + 10);
//    DrawTextSF(window, point, font, "W", pixelSize, sf::Color::White)
//

Eigen::Vector2d DrawUtil::PixelsToWorld(const sf::RenderTarget& window, const sf::Vector2i& p)
{
    return Eigen::Vector2d(double(p.x) - window.getSize().x * 0.5, double(p.y) - window.getSize().y * 0.5) / scale + center;
}


sf::Vector2i DrawUtil::WorldToPixels(const sf::RenderTarget& window, const Eigen::Vector2d& w)
{
    return sf::Vector2i((w[0] + window.getSize().x * 0.5) / scale + center[0], (w[1] + window.getSize().y * 0.5) / scale + center[1]);
}


Eigen::Vector2d DrawUtil::AngleToWorld(const sf::RenderTarget& window, double thetaRad, double magnitude)
{
    Eigen::Vector2d point(1, 0);
    return Eigen::Vector2d(point[0] * std::cosf(-thetaRad) - point[1] * std::sinf(-thetaRad), point[0] * std::sinf(-thetaRad) + point[1] * std::cosf(-thetaRad)) * magnitude;
}


double DrawUtil::SmoothBounce(double tolerance, double trigger_tolerance, double a)
{
    return std::max(1.0 + a*(1.0 - tolerance)*std::exp(10*(trigger_tolerance - tolerance))*(1.0 - std::exp(trigger_tolerance - tolerance)), 0.0);
}


double DrawUtil::Snappy(double tolerance)
{
    return tolerance *(tolerance *(tolerance - 1.0) + 1.0);
}
