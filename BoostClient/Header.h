#pragma once
#include <cstddef>

enum DataType {
	TEXT = 0,
	LOCK_SCREEN = 1,
	UNLOCK_SCREEN = 2,
	SCREENSHOT = 3,
	SCREENSHOT_REQ = 4
};

struct Header {
	DataType type;
	std::size_t size;
};