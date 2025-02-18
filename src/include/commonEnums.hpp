#pragma once

typedef enum EnumSessionStatus {
	READ_CONTINUE,
	READ_COMPLETE,
	WRITE_CONTINUE,
	WRITE_COMPLETE,
	CONNECTION_CLOSED,
	CONNECTION_ERROR
} EnumSesStatus;

typedef enum event {
    READ_EVENT,
    WRITE_EVENT,
    EXCEPTION_EVENT,
	UNKNOWN_EVENT
} TypeEvent;
