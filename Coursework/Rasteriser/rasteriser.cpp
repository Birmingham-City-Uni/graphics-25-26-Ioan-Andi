// image_rasterizer.cpp
//
// Recreates an input image using a software triangle rasterizer.
// It builds a regular grid over the source image, splits each cell
// into two triangles, interpolates vertex colors, and writes the result.
//
// Requires:
//   stb_image.h
//   stb_image_write.h
//
// Compile:
//   g++ -std=c++17 -O2 image_rasterizer.cpp -o image_rasterizer
//
// Run:
//   ./image_rasterizer input.jpg output.png 8
//
// The last argument is the grid step in pixels.
// Smaller step = more triangles = closer reconstruction.
// Example:
//   ./image_rasterizer warhammer.jpg recreated.png 6

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

struct Color {
    float r, g, b;
};

struct Vec2 {
    float x, y;
};

struct Vertex {
    Vec2 pos;
    Color color;
};

static inline float clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

static inline uint8_t toByte(float v) {
    return static_cast<uint8_t>(std::round(clamp01(v) * 255.0f));
}

class Image {
public:
    int width = 0;
    int height = 0;
    int channels = 0;
    std::vector<uint8_t> pixels; // RGB

    bool load(const std::string& path) {
        int w, h, c;
        unsigned char* data = stbi_load(path.c_str(), &w, &h, &c, 3);
        if (!data) {
            return false;
        }

        width = w;
        height = h;
        channels = 3;
        pixels.assign(data, data + width * height * channels);
        stbi_image_free(data);
        return true;
    }

    Color sampleNearest(int x, int y) const {
        x = std::clamp(x, 0, width - 1);
        y = std::clamp(y, 0, height - 1);
        const int idx = (y * width + x) * 3;
        return {
            pixels[idx + 0] / 255.0f,
            pixels[idx + 1] / 255.0f,
            pixels[idx + 2] / 255.0f
        };
    }
};

class Framebuffer {
public:
    int width;
    int height;
    std::vector<uint8_t> pixels; // RGB

    Framebuffer(int w, int h) : width(w), height(h), pixels(w* h * 3, 0) {}

    void clear(const Color& c) {
        const uint8_t r = toByte(c.r);
        const uint8_t g = toByte(c.g);
        const uint8_t b = toByte(c.b);
        for (int i = 0; i < width * height; ++i) {
            pixels[i * 3 + 0] = r;
            pixels[i * 3 + 1] = g;
            pixels[i * 3 + 2] = b;
        }
    }

    void setPixel(int x, int y, const Color& c) {
        if (x < 0 || x >= width || y < 0 || y >= height) return;
        const int idx = (y * width + x) * 3;
        pixels[idx + 0] = toByte(c.r);
        pixels[idx + 1] = toByte(c.g);
        pixels[idx + 2] = toByte(c.b);
    }

    bool savePNG(const std::string& path) const {
        return stbi_write_png(path.c_str(), width, height, 3, pixels.data(), width * 3) != 0;
    }
};

static inline float edgeFunction(const Vec2& a, const Vec2& b, const Vec2& p) {
    return (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x);
}

class Rasterizer {
public:
    explicit Rasterizer(Framebuffer& fb) : fb_(fb) {}

    void drawTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2) {
        const float area = edgeFunction(v0.pos, v1.pos, v2.pos);
        if (std::abs(area) < 1e-8f) return;

        const float minXf = std::floor(std::min({ v0.pos.x, v1.pos.x, v2.pos.x }));
        const float maxXf = std::ceil(std::max({ v0.pos.x, v1.pos.x, v2.pos.x }));
        const float minYf = std::floor(std::min({ v0.pos.y, v1.pos.y, v2.pos.y }));
        const float maxYf = std::ceil(std::max({ v0.pos.y, v1.pos.y, v2.pos.y }));

        const int minX = std::max(0, static_cast<int>(minXf));
        const int maxX = std::min(fb_.width - 1, static_cast<int>(maxXf));
        const int minY = std::max(0, static_cast<int>(minYf));
        const int maxY = std::min(fb_.height - 1, static_cast<int>(maxYf));

        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                Vec2 p{ x + 0.5f, y + 0.5f };

                float w0 = edgeFunction(v1.pos, v2.pos, p);
                float w1 = edgeFunction(v2.pos, v0.pos, p);
                float w2 = edgeFunction(v0.pos, v1.pos, p);

                const bool inside =
                    (w0 >= 0 && w1 >= 0 && w2 >= 0) ||
                    (w0 <= 0 && w1 <= 0 && w2 <= 0);

                if (!inside) continue;

                w0 /= area;
                w1 /= area;
                w2 /= area;

                Color c;
                c.r = w0 * v0.color.r + w1 * v1.color.r + w2 * v2.color.r;
                c.g = w0 * v0.color.g + w1 * v1.color.g + w2 * v2.color.g;
                c.b = w0 * v0.color.b + w1 * v1.color.b + w2 * v2.color.b;

                fb_.setPixel(x, y, c);
            }
        }
    }

private:
    Framebuffer& fb_;
};

static Vertex makeVertex(const Image& img, int x, int y) {
    x = std::clamp(x, 0, img.width - 1);
    y = std::clamp(y, 0, img.height - 1);
    return {
        { static_cast<float>(x), static_cast<float>(y) },
        img.sampleNearest(x, y)
    };
}

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <input_image> <output_png> <grid_step>\n";
        return 1;
    }

    const std::string inputPath = argv[1];
    const std::string outputPath = argv[2];
    const int step = std::max(1, std::stoi(argv[3]));

    Image src;
    if (!src.load(inputPath)) {
        std::cerr << "Failed to load input image: " << inputPath << "\n";
        return 1;
    }

    Framebuffer fb(src.width, src.height);
    fb.clear({ 0.0f, 0.0f, 0.0f });

    Rasterizer rast(fb);

    // Build a regular grid and rasterize 2 triangles per cell.
    for (int y = 0; y < src.height - 1; y += step) {
        for (int x = 0; x < src.width - 1; x += step) {
            const int x0 = x;
            const int y0 = y;
            const int x1 = std::min(x + step, src.width - 1);
            const int y1 = std::min(y + step, src.height - 1);

            Vertex v00 = makeVertex(src, x0, y0);
            Vertex v10 = makeVertex(src, x1, y0);
            Vertex v01 = makeVertex(src, x0, y1);
            Vertex v11 = makeVertex(src, x1, y1);

            // Diagonal split:
            // v00 ---- v10
            //  |     /  |
            //  |   /    |
            // v01 ---- v11
            rast.drawTriangle(v00, v10, v11);
            rast.drawTriangle(v00, v11, v01);
        }
    }

    if (!fb.savePNG(outputPath)) {
        std::cerr << "Failed to save output image: " << outputPath << "\n";
        return 1;
    }

    const long long cellsX = (src.width + step - 1) / step;
    const long long cellsY = (src.height + step - 1) / step;
    const long long triangles = cellsX * cellsY * 2;

    std::cout << "Wrote: " << outputPath << "\n";
    std::cout << "Image: " << src.width << "x" << src.height << "\n";
    std::cout << "Grid step: " << step << "\n";
    std::cout << "Approx triangles: " << triangles << "\n";
    return 0;
}