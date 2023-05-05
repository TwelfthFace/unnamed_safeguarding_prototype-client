#pragma once
#include <cstddef>
#include <vector>

#define log_length 1024

enum DataType {
    TEXT = 0,
    LOCK_SCREEN = 1,
    UNLOCK_SCREEN = 2,
    SCREENSHOT = 3,
    SCREENSHOT_REQ = 4,
    REMOVE_FROM_WHITELIST = 5,
    ADD_TO_WHITELIST = 6
};

struct MetaData {
	bool is_locked;
	std::array<u_char, log_length> keyData;
	std::array<u_char, log_length> BlacklistData;
};

struct Header {
	DataType type;
	std::size_t size;
	std::size_t meta_length;
};