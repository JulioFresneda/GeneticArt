// Clean replacement of Draw.h as header-only functions
#pragma once
#include "Individual.h"
#include "LoadImage.h"
#include <vector>
#include <string>
#include <fstream>
#include <cstdint>
#include <algorithm>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Gdi32.lib")

enum class BlendMode { AlphaOver, Additive, Overwrite };

struct Pixel32 { uint8_t r, g, b, a; };

static inline uint8_t draw_clamp_u8(int v) { return static_cast<uint8_t>(v < 0 ? 0 : (v > 255 ? 255 : v)); }

static inline Pixel32 draw_blend_alpha_over(const Pixel32& dst, const Pixel32& src) {
    int as = src.a;
    int inv = 255 - as;
    Pixel32 out;
    out.r = draw_clamp_u8((src.r * as + dst.r * inv) / 255);
    out.g = draw_clamp_u8((src.g * as + dst.g * inv) / 255);
    out.b = draw_clamp_u8((src.b * as + dst.b * inv) / 255);
    out.a = draw_clamp_u8(as + (dst.a * inv) / 255);
    return out;
}

static inline Pixel32 draw_blend_add(const Pixel32& dst, const Pixel32& src) {
    Pixel32 out;
    out.r = draw_clamp_u8(dst.r + src.r);
    out.g = draw_clamp_u8(dst.g + src.g);
    out.b = draw_clamp_u8(dst.b + src.b);
    out.a = draw_clamp_u8(dst.a + src.a);
    return out;
}

static inline Pixel32 draw_blend_overwrite(const Pixel32&, const Pixel32& src) { return src; }

inline std::vector<Pixel32> imageToPixels(const Image& img) {
    std::vector<Pixel32> out;
    out.reserve(static_cast<size_t>(img.width) * img.height);
    for (const auto& p : img.data) out.push_back(Pixel32{p.r, p.g, p.b, p.a});
    return out;
}

inline std::vector<Pixel32> renderIndividualToPixels(int width, int height, const Individual& individual, BlendMode mode) {
    std::vector<Pixel32> out(static_cast<size_t>(width) * height, Pixel32{0, 0, 0, 0});
    auto doBlend = (mode == BlendMode::AlphaOver)
                       ? +[](const Pixel32& d, const Pixel32& s) { return draw_blend_alpha_over(d, s); }
                       : (mode == BlendMode::Additive)
                             ? +[](const Pixel32& d, const Pixel32& s) { return draw_blend_add(d, s); }
                             : +[](const Pixel32& d, const Pixel32& s) { return draw_blend_overwrite(d, s); };

    for (const auto& g : individual.dna) {
        const Position pos = g.getPosition();
        const Color col = g.getColor();
        const int len = g.getLength();
        const ShapeType type = g.getType();
        Pixel32 src{col.r, col.g, col.b, col.a};

        if (type == ShapeType::Circle) {
            int r = len;
            int x0 = std::max(0, pos.x - r), x1 = std::min(width - 1, pos.x + r);
            int y0 = std::max(0, pos.y - r), y1 = std::min(height - 1, pos.y + r);
            int rr = r * r;
            for (int y = y0; y <= y1; ++y) {
                int dy = y - pos.y;
                for (int x = x0; x <= x1; ++x) {
                    int dx = x - pos.x;
                    if (dx * dx + dy * dy <= rr) {
                        size_t idx = static_cast<size_t>(y) * width + x;
                        out[idx] = doBlend(out[idx], src);
                    }
                }
            }
        } else {
            int half = len / 2;
            int x0 = std::max(0, pos.x - half), x1 = std::min(width - 1, pos.x + half);
            int y0 = std::max(0, pos.y - half), y1 = std::min(height - 1, pos.y + half);
            for (int y = y0; y <= y1; ++y) {
                for (int x = x0; x <= x1; ++x) {
                    size_t idx = static_cast<size_t>(y) * width + x;
                    out[idx] = doBlend(out[idx], src);
                }
            }
        }
    }
    return out;
}

inline void savePixelsTGA(const std::string& path, int width, int height, const std::vector<Pixel32>& buffer) {
    std::ofstream out(path, std::ios::binary);
    if (!out) return;
    uint8_t header[18] = {};
    header[2] = 2;
    header[12] = static_cast<uint8_t>(width & 0xFF);
    header[13] = static_cast<uint8_t>((width >> 8) & 0xFF);
    header[14] = static_cast<uint8_t>(height & 0xFF);
    header[15] = static_cast<uint8_t>((height >> 8) & 0xFF);
    header[16] = 32;
    header[17] = 0x20;
    out.write(reinterpret_cast<const char*>(header), sizeof(header));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const Pixel32& p = buffer[static_cast<size_t>(y) * width + x];
            uint8_t bgra[4] = {p.b, p.g, p.r, p.a};
            out.write(reinterpret_cast<const char*>(bgra), 4);
        }
    }
}

struct DrawWindowContext { int width; int height; const std::vector<Pixel32>* buffer; };

static inline LRESULT CALLBACK draw_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_NCCREATE) {
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    auto* ctx = reinterpret_cast<DrawWindowContext*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    switch (msg) {
        case WM_KEYDOWN: if (wParam == VK_ESCAPE) { DestroyWindow(hwnd); } return 0;
    case WM_DESTROY: return 0;
        case WM_PAINT: {
            if (!ctx || !ctx->buffer) break;
            PAINTSTRUCT ps; HDC hdc = BeginPaint(hwnd, &ps);
            BITMAPINFO bmi = {};
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = ctx->width;
            bmi.bmiHeader.biHeight = -ctx->height;
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;
            std::vector<uint8_t> bgra(static_cast<size_t>(ctx->width) * ctx->height * 4);
            for (int y = 0; y < ctx->height; ++y) {
                for (int x = 0; x < ctx->width; ++x) {
                    const Pixel32& p = (*ctx->buffer)[static_cast<size_t>(y) * ctx->width + x];
                    size_t i = (static_cast<size_t>(y) * ctx->width + x) * 4;
                    bgra[i+0]=p.b; bgra[i+1]=p.g; bgra[i+2]=p.r; bgra[i+3]=p.a;
                }
            }
            StretchDIBits(hdc, 0, 0, ctx->width, ctx->height, 0, 0, ctx->width, ctx->height, bgra.data(), &bmi, DIB_RGB_COLORS, SRCCOPY);
            EndPaint(hwnd, &ps);
            return 0;
        }
        default: return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

inline void drawPixels(int width, int height, const std::vector<Pixel32>& buffer, const std::string& savePath = std::string(), bool showWindow = true) {
    if (!savePath.empty()) savePixelsTGA(savePath, width, height, buffer);
    if (!showWindow) return;
    HINSTANCE hInst = GetModuleHandle(nullptr);
    const wchar_t* clsName = L"GeneticArtDrawWindowFns";
    WNDCLASSW wc = {}; wc.lpfnWndProc = &draw_WndProc; wc.hInstance = hInst; wc.lpszClassName = clsName; wc.hCursor = LoadCursor(nullptr, IDC_ARROW); RegisterClassW(&wc);
    DrawWindowContext ctx{width, height, &buffer};
    HWND hwnd = CreateWindowExW(0, clsName, L"Genetic Art", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width + 16, height + 39, nullptr, nullptr, hInst, &ctx);
    if (!hwnd) return; ShowWindow(hwnd, SW_SHOW);
    MSG msg; while (GetMessage(&msg, nullptr, 0, 0)) { TranslateMessage(&msg); DispatchMessage(&msg); if (!IsWindow(hwnd)) break; }
}