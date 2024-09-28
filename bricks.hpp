#include <string>
#include <regex>
#include <unordered_map>

class Bricks {
	enum BrickType {INT_BRICK, FLOAT_BRICK, STRING_BRICK};

	struct Brick {
		BrickType type;
		std::wstring value;
	};

	static std::unordered_map<std::wstring, Brick> bricks;
public:
	long long int getINTbrick(std::wstring name) {
	    return std::stoll(bricks[name].value);
	}

	long double getFLOATbrick(std::wstring name) {
	    return std::stold(bricks[name].value);
	}

	std::wstring getSTRINGbrick(std::wstring name) {
	    return bricks[name].value;
	}

	void setINTbrick(std::wstring name, long long int value) {
        bricks[name] = {INT_BRICK, std::to_wstring(value)};
	}

	void setFLOATbrick(std::wstring name, long double value) {
	    bricks[name] = {FLOAT_BRICK, std::to_wstring(value)};
	}

	void setSTRINGbrick(std::wstring name, std::wstring value) {
	    bricks[name] = {STRING_BRICK, value};
	}
};
