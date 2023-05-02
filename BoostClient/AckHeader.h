#pragma once

enum AckStatus {
	RECEIVED = 0,
	ERRORR = 1
};

struct Ack {
	AckStatus status;
};