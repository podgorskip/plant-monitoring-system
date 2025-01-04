package iot.pot.exceptions;

import lombok.AllArgsConstructor;

public class UserException extends RuntimeException {
    public UserException(String message) {
        super(message);
    }

    public UserException(ExceptionType type, Long id) {
        super(type.message + id);
    }

    public UserException(ExceptionType type, String mac) {
        super(type.message + mac);
    }

    @AllArgsConstructor
    public enum ExceptionType {
        NOT_FOUND("User not found, id="),
        CONFLICT("User mac exists, id=");

        private final String message;
    }
}
