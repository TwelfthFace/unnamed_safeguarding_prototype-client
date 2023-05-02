#pragma once
#include <cstddef>

enum DataType {
	TEXT = 0,
	SCREENSHOT = 1,
	SCREENSHOT_REQ = 2
};

struct Header {
	DataType type;
	std::size_t size;
};