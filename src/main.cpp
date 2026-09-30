#include <iostream>
#include <optional>
#include <vector>

#include <SFML/Graphics.hpp>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
const int FPS_LIMIT = 30;

using Point2D = sf::Vector2f;

// TODO: (Part 1) Define a function that samples a cubic Bezier curve at t in [0, 1].
Point2D getPoint(const std::vector<sf::Vector2f>& pts, float t) {
    static std::vector<Point2D> tmp;
    assert(pts.size() > 1);
    tmp.clear();
    for (int x = 0; x < pts.size()-1; x++) {
        tmp.push_back(pts[x]*(1.0f-t) + pts[x+1]*t);
    }
    
    while (tmp.size() > 1)
    {
        for (int x = 0; x < tmp.size()-1; x++) {
            tmp[x] = tmp[x]*(1.0f-t) + tmp[x+1]*t;
        }
        tmp.pop_back();
    }
    return tmp[0];
}

// TODO: (Part 2) Define a function that returns the curve's slope at t in [0, 1].
Point2D getSlope(const std::vector<sf::Vector2f>& pts, float t) {
    assert(pts.size() >= 4);
    auto p1 = pts[0];
    auto p2 = pts[1];
    auto p3 = pts[2];
    auto p4 = pts[3];

    auto x = 3*std::pow(1 - t, 2) * (p2.x - p1.x) + 6*(1 - t)*t * (p3.x - p2.x) + 3*std::pow(t, 2) * (p4.x - p3.x);
    auto y = 3*std::pow(1 - t, 2) * (p2.y - p1.y) + 6*(1 - t)*t * (p3.y - p2.y) + 3*std::pow(t, 2) * (p4.y - p3.y);

    return Point2D(x, y);
}

int getClosestPoint(const std::vector<sf::Vector2f>& pts, sf::Vector2i mouse) {
    int closestIdx = 0;
    float minDist = 100000.f;

    for (int i = 0; i < pts.size(); ++i) {
        float distance = std::pow(pts[i].x - mouse.x, 2) + std::pow(pts[i].y - mouse.y, 2);
        if (distance < minDist) {
            minDist = distance;
            closestIdx = i;
        }
    }

    return closestIdx;
}

// TODO: (Part 1) Store four control points for the curve.
std::vector<Point2D> points = {{100.0F, 500.0F}, {200.0F, 100.0F}, {550.0F, 150.0F}, {650.0F, 550.0F}};
// TODO: (Part 2) Track animation time for the square moving along the curve.
// TODO: (Part 3) Track the index of the control point being dragged.
int pointIdx = -1;

void handleInput(sf::Window& window, bool& shouldQuit) {
    while (const std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
            shouldQuit = true;
        } else if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
            // TODO: (Part 3) On left-click, select the closest control point
            // using mouse->position and start dragging it.
            if (mouse->button == sf::Mouse::Button::Left) {
                pointIdx = getClosestPoint(points, mouse->position);
                points[pointIdx] = {static_cast<float>(mouse->position.x), static_cast<float>(mouse->position.y)};
            }
        } else if (const auto* mouse = event->getIf<sf::Event::MouseButtonReleased>()) {
            // TODO: (Part 3) On left-button release, stop dragging.
            pointIdx = -1;
        } else if (const auto* mouse = event->getIf<sf::Event::MouseMoved>()) {
            // TODO: (Part 3) Move the selected control point to mouse->position.
            if (pointIdx != -1) {
                points[pointIdx] = {static_cast<float>(mouse->position.x), static_cast<float>(mouse->position.y)};
            }
            // TODO: (Part 4) Maintain matching slopes at shared endpoints.
            // When moving point 3, move point 5 without changing its distance
            // from point 4 (point numbers here start at 1).
        } else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            // TODO: (Part 4) '+' adds three control points; '-' removes three,
            // keeping at least four points.
            int len = points.size();
            if (key->code == sf::Keyboard::Key::Add || key->code == sf::Keyboard::Key::Equal) {
                Point2D endPoint = points[len - 1];
                Point2D direction = endPoint - points[len - 2];

                //direction with length of 1
                direction /= direction.length();

                Point2D new1 = endPoint + direction * 25.f;
                Point2D new2 = endPoint + direction * 50.f;
                Point2D new3 = endPoint + direction * 75.f;

                points.push_back(new1);
                points.push_back(new2);
                points.push_back(new3);
                pointIdx = -1;
            }
            else if (key->code == sf::Keyboard::Key::Hyphen) {
                if (len > 6) {
                    points.pop_back();
                    points.pop_back();
                    points.pop_back();
                    pointIdx = -1;
                }
            }
        }
    }
}

void render(sf::RenderWindow& window) {
    window.clear(sf::Color::Black);
    // ====== ====== ======
    // TODO: (Part 1) Sample GetPoint over t in [0, 1] and connect samples using the line-drawing
    // code from your project. Draw all four control points as circles after drawing the curve.
    // ====== ====== ======
    static float time(0);
    const float t2 = time / (FPS_LIMIT);
    if (time >= FPS_LIMIT) time = 0;

    float t = 0;
    int groupCount = (points.size() - 1) / 3;
    for (int l = 0; l < groupCount; ++l) {
        std::vector<Point2D> group = {points[l*3], points[l*3 + 1], points[l*3 + 2], points[l*3 + 3]};
        sf::VertexArray line(sf::PrimitiveType::LineStrip, FPS_LIMIT + 1);
        for (int i = 0; i <= FPS_LIMIT; ++i) {
            t = static_cast<float>(i) / FPS_LIMIT;
            line[i].position = getPoint(group, t);
            line[i].color = sf::Color::Blue;
        }
        window.draw(line);
    }

    for (int i = 0; i < points.size(); ++i) {
        sf::CircleShape circle(10.f);
        circle.setOrigin({10.f, 10.f});
        circle.setPosition(points[i]);
        circle.setFillColor(sf::Color::Red);
        window.draw(circle);
    }

    // ====== ====== ======
    // TODO: (Part 2) Draw a small square moving repeatedly along the curve.
    // Use GetSlope to orient it to the curve at each time step.
    // ====== ====== ======
    std::vector<Point2D> group1 = {points[0], points[1], points[2], points[3]};
    Point2D position = getPoint(group1, t2);
    Point2D slope = getSlope(group1, t2);

    sf::RectangleShape square({10.f, 10.f});
    square.setOrigin({5.f, 5.f});
    // https://www.sfml-dev.org/tutorials/3.0/graphics/transform/#rotation
    square.setRotation(sf::radians(std::atan2(slope.y, slope.x)));
    square.setPosition(position);
    square.setFillColor(sf::Color::Green);
    window.draw(square);

    // ====== ====== ======
    // TODO: (Part 3) Draw control handles from point 1 to 2 and point 3 to 4.
    // TODO: (Part 4) Draw all connected cubic Bezier segments and their handles.
    // ====== ====== ======

    // https://www.sfml-dev.org/tutorials/3.0/graphics/shape/#lines
    for (int i = 0; i < groupCount; ++i) {
        int temp = i * 3;
        std::array line1 =
        {
            sf::Vertex{points[temp]},
            sf::Vertex{points[temp + 1]}
        };
        std::array line2 =
        {
            sf::Vertex{points[temp+ 2]},
            sf::Vertex{points[temp+ 3]}
        };
    
        window.draw(line1.data(), line1.size(), sf::PrimitiveType::Lines);
        window.draw(line2.data(), line1.size(), sf::PrimitiveType::Lines);
    }


    // ====== ====== ======
    // TODO: (Bonus) Support multiple curves, a Galaga screen overlay at a 1:2 ratio, and exporting
    // curve points as C++ code for Project 1b.
    // ====== ====== ======
    time++;
    window.display();
}

int main() {
    sf::RenderWindow window;

    try {
        // Initialize window
        window.create(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Bezier Curve Editor");
        window.setFramerateLimit(FPS_LIMIT);
        // Prevent key repeats.
        window.setKeyRepeatEnabled(false);

        bool shouldQuit = false;
        // Main game loop
        while (window.isOpen()) {
            handleInput(window, shouldQuit);
            if (shouldQuit) {
                break;
            }
            render(window);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
