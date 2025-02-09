package iot.pot.exceptions;

import lombok.AllArgsConstructor;

public class DeviceException extends RuntimeException {
    public DeviceException(String message) {
        super(message);
    }

    public DeviceException(ExceptionType type, Long id) {
        super(type.message + id);
    }

    public DeviceException(ExceptionType type) {
        super(type.message);
    }


    @AllArgsConstructor
    public enum ExceptionType {
        NOT_FOUND("Device not found, id="),
        MEASUREMENT_NOT_FOUND("Measurement not found");

        private final String message;
    }
}
