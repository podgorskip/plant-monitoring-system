package iot.pot.exceptions;

import lombok.AllArgsConstructor;

public class DeviceException extends RuntimeException {
    public DeviceException(String message) {
        super(message);
    }

    public DeviceException(ExceptionType type, Long id) {
        super(type.message + id);
    }

    @AllArgsConstructor
    public enum ExceptionType {
        NOT_FOUND("Device not found, id=");

        private final String message;
    }
}
