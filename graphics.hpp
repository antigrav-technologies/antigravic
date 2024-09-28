#include <SDL2/SDL.h>

typedef struct {
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
} RGBColor;

class Window {
    SDL_Window* window;
    SDL_Renderer* renderer;
public:
    Window(const char* name, int w, int h, int scale) {
        if (SDL_Init(SDL_INIT_EVERYTHING) < 0) exit(1);

        window = SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_SHOWN);
        if (!window) exit(1);

        renderer = SDL_CreateRenderer(window, -1, 0);
        if (!renderer) exit(1);

        SDL_RenderSetScale(renderer, scale, scale);
    }

    void close() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(0);
    }

    void rect(int x, int y, int w, int h, RGBColor c) {
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
        SDL_Rect rect_ = {x, y, w, h};
        SDL_RenderFillRect(renderer, &rect_);
    }

    void pixel(int x, int y, RGBColor c) {
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
        SDL_RenderDrawPoint(renderer, x, y);
    }

    void circle(int x, int y, int radius, RGBColor c) {
        for (int i = 0; i < radius*2; i++) {
            for (int j = 0; j < radius*2; j++) {
                int dx = i - radius;
                int dy = j - radius;
                int distance = dx * dx + dy * dy;

                if ((radius - 1) * (radius - 1) <= distance && distance <= radius * radius) {
                    pixel(x - radius + i, y - radius + j, c);
                }
            }
        }
    }

    void line(int x1, int y1, int x2, int y2, RGBColor c) {
        /*
        if (SCALE == 1) {
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
            return;
        }
        int dx = x2 - x1;
        int dy = y2 - y1;
        float length = sqrt(dx * dx + dy * dy);

        for (int i = 0; i <= length; i++) {
            int x = x1;
            int y = y1;
            if (length > 0) {
                x += i * dx / length;
                y += i * dy / length;
            }
            pixel(x, y, c);
        }*/
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }

    void rectOutline(int x1, int y1, int w, int h, RGBColor c) {
        line(x1,     y1,     x1 + w, y1,     c);
        line(x1 + w, y1,     x1 + w, y1 + h, c);
        line(x1 + w, y1 + h, x1,     y1 + h, c);
        line(x1,     y1 + h, x1,     y1,     c);
    }

    void render(long double ms) {
        SDL_RenderPresent(renderer);
        SDL_Delay(ms);
    }
};
