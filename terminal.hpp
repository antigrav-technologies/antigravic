#include <chrono>
#include <codecvt>
#include "C64Symbols.hpp"
#include "graphics.hpp"
#include "utils.hpp"

enum ShapeType {POINT, LINE, RECT, CIRCLE};
struct Shape {
    ShapeType type;
    RGBColor color;
    int x1, y1, x2 = 0, y2 = 0; // x2 and y2 can be used as width and height or not used or something idk
};

class C64Terminal {
    std::shared_ptr<Window> window;
    static constexpr int TEXT_WIDTH = 40;
    static constexpr int TEXT_HEIGHT = 25;

    static constexpr int SCALE = 2;
    static constexpr int BORDER_SIZE = 32;
    static constexpr int WIDTH = TEXT_WIDTH * 8 + BORDER_SIZE * 2;
    static constexpr int HEIGHT = TEXT_HEIGHT * 8 + BORDER_SIZE * 2;

    int history[TEXT_WIDTH][TEXT_HEIGHT];
    RGBColor colors[TEXT_WIDTH][TEXT_HEIGHT];

    RGBColor TEXT_COLOR = {177, 158, 255};
    RGBColor BG_COLOR = {105, 83, 245};
    RGBColor current_text_color = TEXT_COLOR;

    int text_pos_x, text_pos_y;

    int lines_moved = 0;
    std::chrono::time_point<std::chrono::steady_clock> cursor_time;

    RGBColor getColor(int n) {
        switch (n) {
            case 0:  return {0,   0,   0  }; // black
            case 1:  return {127, 0,   0  }; // maroon
            case 2:  return {0,   127, 0  }; // green
            case 3:  return {127, 127, 0  }; // olive
            case 4:  return {0,   0,   127}; // navy
            case 5:  return {127, 0,   127}; // PURPLE IS THIS AN ACE CULTURE???71171777ЩЩЩ
            case 6:  return {0,   127, 127}; // teal
            case 7:  return {127, 127, 127}; // silver

            case 8:  return {192, 192, 192}; // gray
            case 9:  return {255, 0,   0  }; // red
            case 10: return {0,   255, 0  }; // lime
            case 11: return {255, 255, 0  }; // yellow
            case 12: return {0,   0,   0  }; // blue
            case 13: return {255, 0,   255}; // pink
            case 14: return {0,   255, 255}; // cyan
            case 15: return {255, 255, 255}; // white
            default: return TEXT_COLOR;
        }
    }

    void drawShape(Shape shape) {
        if (shape.type == POINT) window->pixel(shape.x1, shape.y1, shape.color);
        else if (shape.type == LINE) window->line(shape.x1, shape.y1, shape.x2, shape.y2, shape.color);
        else if (shape.type == RECT) window->rectOutline(shape.x1, shape.y1, shape.x2, shape.y2, shape.color);
        else if (shape.type == CIRCLE) window->circle(shape.x1, shape.y1, shape.x2, shape.color);
    }

    void drawBG() {
        window->rect(0, 0, WIDTH, HEIGHT, TEXT_COLOR);

        window->rect(BORDER_SIZE,
                     BORDER_SIZE,
                     WIDTH - BORDER_SIZE * 2,
                     HEIGHT - BORDER_SIZE * 2,
                     BG_COLOR);
    }

    void drawCursor() {
        int x = text_pos_x;
        int y = text_pos_y;
        if (x >= TEXT_WIDTH) {
            x = 0;
            y++;
        }
        window->rect(x * 8 + BORDER_SIZE,
                     y * 8 + BORDER_SIZE,
                     8, 8, TEXT_COLOR);
    }

public:
    bool showCursor = true;
    std::vector<Shape> shapes = {{CIRCLE, {0, 255, 0}, 50, 50, 50, 0}};

    C64Terminal() {
        this->window = std::make_shared<Window>(Window("ANTIGRAVIC V0", WIDTH * SCALE, HEIGHT * SCALE, SCALE));
        this->clearScreen();
        *this << std::string((int)((TEXT_WIDTH - 21)/2), ' ') << "*** ANTIGRAVIC V0 ***\n";
        *this << "\n";
        *this << "USE LOAD OR WRITE YOUR PROGRAM\n";
    }

    bool escPressed() {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) window->close();
            return (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE);
        }
        return false;
    }

    void clearScreen() {
        for (int x = 0; x < TEXT_WIDTH; x++) {
            for (int y = 0; y < TEXT_HEIGHT; y++) {
                history[x][y] = ' ';
                colors[x][y] = TEXT_COLOR;
            }
        }
        text_pos_x = 0;
        text_pos_y = 0;
    }

    void moveUp() {
        for (int x = 0; x < TEXT_WIDTH; x++) {
            for (int y = 0; y < TEXT_HEIGHT - 1; y++) {
                history[x][y] = history[x][y + 1];
                colors[x][y] = colors[x][y + 1];
            }
            history[x][TEXT_HEIGHT - 1] = ' ';
            colors[x][TEXT_HEIGHT - 1] = TEXT_COLOR;
        }
    }

    void newlineCheck() {
        while (text_pos_x >= TEXT_WIDTH) {
            text_pos_x -= TEXT_WIDTH;
            text_pos_y++;
            lines_moved++;
        }
        while (text_pos_y >= TEXT_HEIGHT) {
            text_pos_y--;
            this->moveUp();
        }
    }

    void newline() {
        text_pos_x = 0;
        text_pos_y++;
        newlineCheck();
    }

    void draw_symbol(int x, int y) {
        bool (*symbol_matrix)[8] = get_matrix(history[x][y]);

        window->rect(x * 8 + BORDER_SIZE, y * 8 + BORDER_SIZE, 8, 8, BG_COLOR);
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                if (symbol_matrix[i][j]) {
                    window->pixel(x * 8 + BORDER_SIZE + j,
                                  y * 8 + BORDER_SIZE + i,
                                  colors[x][y]);
                }
            }
        }
    }

    void renderText() {
        for (int x = 0; x < TEXT_WIDTH; x++)
            for (int y = 0; y < TEXT_HEIGHT; y++)
                this->draw_symbol(x, y);
    }

    void render() {
        this->escPressed();
        drawBG();
        float elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - cursor_time).count();
        if (elapsedTime > 1000) {
            resetCursorTime();
            elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - cursor_time).count();
        }
        renderText();
        if (showCursor && elapsedTime < 500) this->drawCursor();
        for (Shape& shape : shapes)
            drawShape(shape);
        window->render(16);
    }

    void append_codepoint(int symbol) {
        if (symbol != '\n') {
            history[text_pos_x][text_pos_y] = symbol;
            colors[text_pos_x][text_pos_y] = current_text_color;
            text_pos_x++;
            newlineCheck();
        }
        else newline();
    }

    void append_string(std::string str) {
        for (char c : str)
            this->append_codepoint(c);
    }

    void append_string(std::u32string u32str) {
        for (char32_t c32 : u32str)
            this->append_codepoint(static_cast<int>(c32));
    }

    C64Terminal& operator<<(const char& c) {
        this->append_codepoint(static_cast<int>(c));
        return *this;
    }

    C64Terminal& operator<<(const char32_t& c32) {
        this->append_codepoint(static_cast<int>(c32));
        return *this;
    }

    C64Terminal& operator<<(const std::string& str) {
        this->append_string(str);
        return *this;
    }

    C64Terminal& operator<<(const std::u32string& u32str) {
        this->append_string(u32str);
        return *this;
    }

    C64Terminal& operator<<(std::wostream& (*)(std::wostream&)) {
        return *this << '\n';
    }

    C64Terminal& operator<<(const short& n) {
        return *this << std::to_string(n);
    }

    C64Terminal& operator<<(const unsigned short& n) {
        return *this << std::to_string(n);
    }

    C64Terminal& operator<<(const int& n) {
        return *this << std::to_string(n);
    }

    C64Terminal& operator<<(const unsigned int& n) {
        return *this << std::to_string(n);
    }

    C64Terminal& operator<<(const long int& n) {
        return *this << std::to_string(n);
    }

    C64Terminal& operator<<(const unsigned long int& n) {
        return *this << std::to_string(n);
    }

    C64Terminal& operator<<(const long long int& n) {
        return *this << std::to_string(n);
    }

    C64Terminal& operator<<(const unsigned long long int& n) {
        return *this << std::to_string(n);
    }

    void resetCursorTime() {
        cursor_time = std::chrono::steady_clock::now();
    }

    void setTextColor(int text_color) {
        this->current_text_color = getColor(text_color);
    }

    std::u32string input() {
        std::vector<char32_t> input;
        lines_moved = 0;

        SDL_Event e;
        while (true) {
            this->render();
            char32_t key = '\t';
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) window->close();
                else if (e.type == SDL_TEXTINPUT) {
                    this->resetCursorTime();
                    key = c32_to_upper(utf8_to_char32(e.text.text));
                }
                else if (e.type == SDL_KEYDOWN) {
                    this->resetCursorTime();
                    switch (e.key.keysym.sym) {
                        case SDLK_BACKSPACE:
                            key = '\b'; break;
                        case 13:
                        case SDLK_KP_ENTER:
                            key = '\r'; break;
                        default:
                            break;
                    }
                }
            }
            if (key == '\b') {
                text_pos_x--;
                if (text_pos_x < 0) {
                    text_pos_x = 0;
                    if (lines_moved > 0) {
                        lines_moved--;
                        text_pos_x = TEXT_WIDTH - 1;
                        text_pos_y--;
                        history[text_pos_x][text_pos_y] = ' ';
                        input.pop_back();
                    }
                }
                else {
                    history[text_pos_x][text_pos_y] = ' ';
                    input.pop_back();
                }
            }
            else if (key == '\r') { // enter
                lines_moved = text_pos_x = 0;
                text_pos_y++;
                this->newlineCheck();
                std::u32string s = std::u32string(input.begin(), input.end());
                return u32strip(s);
            }
            else if (key != '\t') {
                if (input.size() <= 128) {
                    this->append_codepoint(static_cast<int>(key));
                    input.push_back(key);
                }
            }
        }
    }

    void loadingScreen(int lines) {
        if (lines > 0) {
            this->clearScreen();
            int k = 0;
            Uint8 r, g, b = 255;
            for (int i = 0; i < lines; i++) {
                int j = static_cast<int>(i * 6);
                float f = 255 * (i * 6 - j);

                switch (j % 6) {
                    case 0: r = 255;     g = f;       b = 0;       break;
                    case 1: r = 255 - f; g = 255;     b = 0;       break;
                    case 2: r = 0;       g = 255;     b = f;       break;
                    case 3: r = 0;       g = 255 - f; b = 255;     break;
                    case 4: r = f;       g = 0;       b = 255;     break;
                    case 5: r = 255;     g = 0;       b = 255 - f; break;
                }
                escPressed();
                for (int i = -2; i <= HEIGHT / BORDER_SIZE; i++) {
                    RGBColor color = {r, g, b};
                    if ((i + 2) % 2 == 0) color = {
                        static_cast<Uint8>(r/2),
                        static_cast<Uint8>(g/2),
                        static_cast<Uint8>(b/2)
                    };
                    window->rect(0,
                           0 + i * BORDER_SIZE + k/5,
                           WIDTH,
                           BORDER_SIZE + 1,
                           color);
                    k++;
                }
                k %= BORDER_SIZE * 2 * 5;
                window->rect(BORDER_SIZE,
                       BORDER_SIZE,
                       WIDTH - BORDER_SIZE * 2,
                       HEIGHT - BORDER_SIZE * 2,
                       BG_COLOR);
                text_pos_x = TEXT_WIDTH - 13;
                text_pos_y = TEXT_HEIGHT - 2;
                *this << "LOADING " << 100*i/lines << "%";
                this->renderText();
                window->render(16);
            }
            this->clearScreen();
            *this << "LOADED!\n";
        }
    }
};
