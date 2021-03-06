#pragma once
#include <SFML/Graphics.hpp>
#include <Eigen/Dense>
#include <string>

class DrawUtil
{
public:
	static void DrawCircle(sf::RenderTarget& window, const Eigen::Vector2d& c, const sf::Color& color, double r, double thickness = 8.0f);
	static void DrawRect(sf::RenderTarget& window, const float x, const float y, const sf::Vector2f& size, const sf::Color& color);
	static void DrawLine(sf::RenderTarget& window, const Eigen::Vector2d& a, const Eigen::Vector2d& b, const sf::Color& color, bool extend, double thickness = 8.0f);
	static void DrawPoint(sf::RenderTarget& window, const Eigen::Vector2d& c, const sf::Color& color, double radius = 20.0f);
	static void DrawTextSF(sf::RenderTarget& window, const float x, const float y, sf::Font font, sf::String& string, int pixelSize, const sf::Color& color);
	static void DrawTextSF(sf::RenderTarget& window, const Eigen::Vector2d& point, sf::Font font, sf::String& string, int pixelSize, const sf::Color& color);
	static void DrawGrid(sf::RenderTarget& window, double t);

	static void DrawKeypresses(sf::RenderTarget& window, Eigen::Vector2d& point, sf::Font font, int pixelSize);

	static sf::Vector2f ToSF(const Eigen::Vector2d& v);
	static Eigen::Vector2d HalfSize(const sf::RenderTarget& window);

	static Eigen::Vector2d PixelsToWorld(const sf::RenderTarget& window, const sf::Vector2i& p);
	static Eigen::Vector2d AngleToWorld(const sf::RenderTarget& window, double thetaRad, double magnitude);
	static sf::Vector2i WorldToPixels(const sf::RenderTarget& window, const Eigen::Vector2d& w);

	static double SmoothBounce(double t, double trigger_t, double a);
	static double Snappy(double t);

	static Eigen::Vector2d center;
	static double scale;
	static float render_scale;
};
