#pragma once

typedef enum sessionStatus {
    READ_CONTINUE,
    READ_COMPLETE,
    WRITE_CONTINUE,
    WRITE_COMPLETE,
    CONNECTION_CLOSED,
    CONNECTION_ERROR
}  TypeSesStatus;

typedef enum event {
    READ_EVENT,
    WRITE_EVENT,
    EXCEPTION_EVENT
} TypeEvent;
