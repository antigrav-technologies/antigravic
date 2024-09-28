#include <complex>
#include <vector>
#include <stdexcept>
#include <variant>
#include "utils.hpp"

typedef enum {
    INT_TOKEN,
    FLOAT_TOKEN,
    COMPLEX_TOKEN,
    STRING_TOKEN,
    BOOL_TOKEN,
    OPERATOR_TOKEN,
    KEYWORD_TOKEN,
    VARIABLE_TOKEN
} TokenType;

typedef enum {
    PLUS_OPERTATOR,
    MINUS_OPERTATOR,
    MUL_OPERTATOR,
    DIV_OPERTATOR,
    MOD_OPERTATOR,
    EQUAL_OPERTATOR
} Operator;

typedef enum {
    INPUT_KEYWORD,
    PRINT_KEYWORD,
    CLS_KEYWORD,
    GOTO_KEYWORD,
    IF_KEYWORD,
    THEN_KEYWORD,
    FOR_KEYWORD,
    TO_KEYWORD,
    STEP_KEYWORD,
    NEXT_KEYWORD,
    WHILE_KEYWORD,
    WEND_KEYWORD,
    END_KEYWORD,
    CHAIN_KEYWORD,
    COLOR_KEYWORD
} Keyword;

typedef struct {
    TokenType type;
    std::variant<long long, long double, std::complex<long double>, std::u32string, Operator, Keyword> value;
} Token;

class LexerException : public std::exception {
    const char* message_;
public:
    LexerException(const char* message) : message_(message) {}
    const char* what() const throw() {
        return message_;
    }
};

std::u32string __decode_string(const std::u32string& s, size_t& idx) {
    std::u32string s_;

    while (true) {
        int startIndex = idx;
        while (!(s[idx] == '"' || s[idx] == '\\' || s[idx] <= '\x1f')) {
            idx++;
            if (idx >= s.size()) throw LexerException("UNTERMINATED STRING LITERAL");
        }
        s_.append(s.substr(startIndex, idx-startIndex));
        char32_t terminator = s[idx++];
        if (terminator == '\"') break;
        if (terminator != '\\') throw LexerException("INVALID CONTROL CHARACTER");

        if (idx >= s.size()) throw LexerException("UNTERMINATED STRING LITERAL");
        char32_t esc = s[idx];
        if (!BACKSLASH.count(esc)) throw LexerException("INVALID \\ ESCAPE");
        s_.push_back(BACKSLASH[esc]);
    }
    return s_;
}

Token __decode_keyword(const std::u32string& s, size_t& idx) {
    int offset = 0;
    while (idx + offset < s.size() and s[idx + offset] != ' ') offset++;
    std::u32string k = s.substr(idx, offset);
    idx += offset;
    if (!u32s_is_alpha(k)) throw new LexerException("INVALID KEYWORD NAME");
    if (k == U"TRUE") return {BOOL_TOKEN, true};
    if (k == U"FALSE") return {BOOL_TOKEN, false};
    if (k == U"INPUT") return {KEYWORD_TOKEN, INPUT_KEYWORD};
    if (k == U"PRINT") return {KEYWORD_TOKEN, PRINT_KEYWORD};
    if (k == U"CLS") return {KEYWORD_TOKEN, CLS_KEYWORD};
    if (k == U"GOTO") return {KEYWORD_TOKEN, GOTO_KEYWORD};
    if (k == U"IF") return {KEYWORD_TOKEN, IF_KEYWORD};
    if (k == U"THEN") return {KEYWORD_TOKEN, THEN_KEYWORD};
    if (k == U"FOR") return {KEYWORD_TOKEN, FOR_KEYWORD};
    if (k == U"TO") return {KEYWORD_TOKEN, TO_KEYWORD};
    if (k == U"STEP") return {KEYWORD_TOKEN, STEP_KEYWORD};
    if (k == U"NEXT") return {KEYWORD_TOKEN, NEXT_KEYWORD};
    if (k == U"WHILE") return {KEYWORD_TOKEN, WHILE_KEYWORD};
    if (k == U"WEND") return {KEYWORD_TOKEN, WEND_KEYWORD};
    if (k == U"END") return {KEYWORD_TOKEN, END_KEYWORD};
    if (k == U"COLOR") return {KEYWORD_TOKEN, COLOR_KEYWORD};
    return {VARIABLE_TOKEN, k};
}

std::vector<Token> lex_string(std::u32string s) {
    size_t idx = 0;
    std::vector<Token> tokens;

    while (true) {
        while (idx < s.size() and s[idx] != U' ') idx++;
        if (idx == s.size()) return tokens;
        char32_t nextchar = s[idx];

        if (nextchar == U'"') {
            idx++;
            tokens.push_back({STRING_TOKEN, __decode_string(s, idx)});
            continue;
        }
        if (nextchar == U'+') {
            idx++;
            tokens.push_back({OPERATOR_TOKEN, PLUS_OPERTATOR});
            continue;
        }
        if (nextchar == U'-') {
            idx++;
            tokens.push_back({OPERATOR_TOKEN, MINUS_OPERTATOR});
            continue;
        }
        if (nextchar == U'*') {
            idx++;
            tokens.push_back({OPERATOR_TOKEN, MUL_OPERTATOR});
            continue;
        }
        if (nextchar == U'/') {
            idx++;
            tokens.push_back({OPERATOR_TOKEN, DIV_OPERTATOR});
            continue;
        }
        if (nextchar == U'%') {
            idx++;
            tokens.push_back({OPERATOR_TOKEN, MOD_OPERTATOR});
            continue;
        }
        if (nextchar == U'=') {
            idx++;
            tokens.push_back({OPERATOR_TOKEN, EQUAL_OPERTATOR});
            continue;
        }
        std::match_results<std::string::iterator> what;
        if (u32match(s, idx, complex_pattern, what)) {
            std::string real_sign = what[1];
            std::string real_rest = what[2];
            std::string imag_sign = what[4];
            std::string imag_rest = what[5];

            long double real, imag;

            if (real_rest == "INF") real = std::numeric_limits<long double>::infinity();
            else if (real_rest == "NAN") real = std::numeric_limits<long double>::quiet_NaN();
            else real = std::stold(real_rest);
            if (real_sign == "-") real *= -1;

            if (imag_rest == "INF") imag = std::numeric_limits<long double>::infinity();
            else if (imag_rest == "NAN") imag = std::numeric_limits<long double>::quiet_NaN();
            else imag = std::stold(imag_rest);
            if (imag_sign == "-") imag *= -1;

            idx += what.position() + what.length();
            tokens.push_back({COMPLEX_TOKEN, std::complex<long double>(real, imag)});
            continue;
        }
        if (u32match(s, idx, float_pattern, what)) {
            if (what[2].matched) {
                std::string sign = what[1];
                std::string rest = what[2];
                long double value;
                if (rest == "INF") value = std::numeric_limits<long double>::infinity();
                else if (rest == "NAN") value = std::numeric_limits<long double>::quiet_NaN();
                else value = std::stold(rest);
                if (sign == "-") value *= -1;
                idx += what.position() + what.length();
                tokens.push_back({FLOAT_TOKEN, value});
                continue;
            }
        }
        if (u32match(s, idx, int_pattern, what)) {
            if (!what.empty()) {
                idx += what.position() + what.length();
                tokens.push_back({INT_TOKEN, std::stoll(what[0])});
                continue;
            }
        }
        tokens.push_back(__decode_keyword(s, idx));
    }
}