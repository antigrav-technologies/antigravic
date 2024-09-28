#ifndef ANTIGRAVIC_UTILS_HPP
#define ANTIGRAVIC_UTILS_HPP
#include <string>
#include <regex>
#include <unordered_map>
#include <SDL2/SDL.h>
#include "c32_to_upper.hpp"

std::regex int_pattern("-?\\d+", std::regex_constants::ECMAScript);
std::regex float_pattern("([-+]?)((\\d*(?:\\.\\d+|E[-+]?\\d+))|INF|NAN)", std::regex_constants::ECMAScript);
std::regex complex_pattern("([-+]?)((\\d+(?:\\.\\d+|E[-+]?\\d+))|INF|NAN)([+-])((\\d+(?:\\.\\d+|E[-+]?\\d+))|INF|NAN)i", std::regex_constants::ECMAScript);

std::unordered_map<char32_t, char32_t> BACKSLASH = {
    { U'\\', U'\\' },
    { U'"', U'"' },
    { U'n', U'\n' }
};

char32_t utf8_to_char32(const char c[SDL_TEXTINPUTEVENT_TEXT_SIZE]) {
    char32_t codePoint = 0;
    const char* utf8_str = c;

    if ((*utf8_str & 0x80) == 0) {
        codePoint = *utf8_str;
    }
    else if ((*utf8_str & 0xE0) == 0xC0) {
        codePoint = ((c[0] & 0x1F) << 6) | (c[1] & 0x3F);
    }
    else if ((*utf8_str & 0xF0) == 0xE0) {
        codePoint = (*utf8_str & 0x0F) << 12;
        utf8_str++;
        codePoint |= (*utf8_str & 0x3F) << 6;
        utf8_str++;
        codePoint |= *utf8_str & 0x3F;
    }
    else if ((*utf8_str & 0xF8) == 0xF0) {
        codePoint = (*utf8_str & 0x07) << 18;
        utf8_str++;
        codePoint |= (*utf8_str & 0x3F) << 12;
        utf8_str++;
        codePoint |= (*utf8_str & 0x3F) << 6;
        utf8_str++;
        codePoint |= *utf8_str & 0x3F;
    }
    else {
        return 0xFFFD;
    }

    return codePoint;
}

std::string u32string_to_string(const std::u32string& u32str) {
    std::string str;
    for (auto u32c : u32str) {
        if (u32c <= 0x7F) {
            str.push_back(static_cast<char>(u32c));
        }
        else if (u32c <= 0x7FF) {
            str.push_back(static_cast<char>(0xC0 | (u32c >> 6)));
            str.push_back(static_cast<char>(0x80 | (u32c & 0x3F)));
        }
        else if (u32c <= 0xFFFF) {
            str.push_back(static_cast<char>(0xE0 | (u32c >> 12)));
            str.push_back(static_cast<char>(0x80 | ((u32c >> 6) & 0x3F)));
            str.push_back(static_cast<char>(0x80 | (u32c & 0x3F)));
        }
        else {
            throw std::runtime_error("Invalid Unicode code point");
        }
    }
    return str;
}

std::u32string string_to_u32string(const std::string& str) {
    std::u32string u32str;
    for (auto it = str.begin(); it != str.end() ;) {
        char32_t codepoint;
        if ((*it & 0x80) == 0) {
            codepoint = *it;
            it++;
        }
        else if ((*it & 0xE0) == 0xC0) {
            codepoint = (*it & 0x1F) << 6;
            it++;
            codepoint |= *it & 0x3F;
            it++;
        }
        else if ((*it & 0xF0) == 0xE0) {
            codepoint = (*it & 0xF) << 12;
            it++;
            codepoint |= (*it & 0x3F) << 6;
            it++;
            codepoint |= *it & 0x3F;
            it++;
        }
        else {
            throw std::runtime_error("Invalid UTF-8 sequence");
        }
        u32str.push_back(codepoint);
    }
    return u32str;
}

bool contains(const char32_t& symbol, const std::u32string& str) {
    return (str.find(symbol) != std::u32string::npos);
}

bool isDigit(const std::u32string& str) {
    for (wchar_t c : str)
        if (!isdigit(c))
            return false;
    return true;
}

bool c32_is_alpha(char32_t c) {
    return (0x41 <= c && c <= 0x5a) || (0x3f7 <= c && c <= 0x481); // cyrrilic due the electric caterpillars, only checks capitilised ascii
}


bool u32s_is_alpha(std::u32string s) {
    for (char32_t c : s) {
        if (!c32_is_alpha(c)) return false;
    }
    return true;
}

int u32stoi(const std::u32string& u32str, std::size_t* pos = nullptr, int base = 10 ) {
    std::string str;
    for (char32_t c : u32str) {
        if (c > 127) throw std::invalid_argument("u32stoi: non-ASCII character");
        str.push_back(static_cast<char>(c));
    }
    return std::stoi(str, pos, base);
}

bool u32match(const std::u32string& str, size_t idx, const std::regex& regex, std::match_results<std::string::iterator>& what) {
    std::string substr;
    for (char32_t c : str.substr(idx)) {
        if (c > 0x7f) break;
        substr.push_back(static_cast<char>(c));
    }

    return std::regex_search(substr.begin(), substr.end(), what, regex);
}

bool starts_with_number(const std::u32string& str) {
    size_t space_pos = str.find_first_of(' ');
    std::u32string numStr = str.substr(0, space_pos);
    return isDigit(numStr);
}

std::u32string u32strip(const std::u32string& inpt) {
    auto start_it = inpt.begin();
    auto end_it = inpt.rbegin();
    while (isspace(*start_it))
        ++start_it;
    while (isspace(*end_it))
        ++end_it;
    return std::u32string(start_it, end_it.base());
}

std::u32string safe_substr(const std::u32string& str, size_t start, size_t length) {
	if (start >= str.size()) return U"";
	return str.substr(start, std::min(length, str.size() - start));
}

bool try_parse_hex(const std::u32string& s, int& codePoint) {
	try {
		codePoint = u32stoi(s, 0, 16);
		return true;
	}
	catch (const std::invalid_argument&) {
		return false;
	}
}
#endif