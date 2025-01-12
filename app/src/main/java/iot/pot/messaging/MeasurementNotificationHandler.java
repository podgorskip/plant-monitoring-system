package iot.pot.messaging;

import iot.pot.database.model.User;
import org.springframework.scheduling.annotation.Async;

public interface MeasurementNotificationHandler {
    @Async
    void sendMessage(User user, String message);
}