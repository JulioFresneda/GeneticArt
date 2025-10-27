#pragma once
#include <cstdint>
#include <memory>
#include <algorithm>
#include "RandomHelper.h"

struct Color {
    uint8_t r, g, b, a;
};

struct Position {
    int x;
    int y;
};

// Shape selector to support multiple gene types
enum class ShapeType {
    Circle,
    Square
};

class Gene {
public:
    Gene(int x, int y, Color c, ShapeType type, int length)
        : position{x, y}, color(c), type(type), length(length) {}

    ShapeType getType() const { return type; }
    int getLength() const { return length; }
    Color getColor() const { return color; }
    Position getPosition() const { return position; }
    Gene clone() const {
        return Gene(position.x, position.y, color, type, length);
    }

    // Mutations split into focused methods; wrapper picks one randomly for compatibility
    void mutatePosition(Random& rand, int img_width, int img_height) {
        // Move the position max 10% of image dimensions and wrap around image bounds (toroidal)
        int dx = rand.getInt(-img_width / 10, img_width / 10);
        int dy = rand.getInt(-img_height / 10, img_height / 10);

        auto wrap = [](int v, int limit) {
            if (limit <= 0) return 0; // guard against invalid dimensions
            int m = v % limit;
            return m < 0 ? m + limit : m;
        };

        position.x = wrap(position.x + dx, img_width);
        position.y = wrap(position.y + dy, img_height);
    }

    void mutateColor(Random& rand) {
        // Preserve prior behavior using modulo wrap-around
        color.r = (color.r + rand.getInt(-10, 10)) % 256;
        color.g = (color.g + rand.getInt(-10, 10)) % 256;
        color.b = (color.b + rand.getInt(-10, 10)) % 256;
        color.a = (color.a + rand.getInt(-10, 10)) % 256;
    }

    void mutateLength(Random& rand) {
        // Change length by max 20% of current length
        int delta = length * 0.2;
        length = std::max(1, length + rand.getInt(-delta, delta));
    }

    

private:
    Position position;
    Color color;
    ShapeType type;
    int length; // radius for circles, side length for squares
};