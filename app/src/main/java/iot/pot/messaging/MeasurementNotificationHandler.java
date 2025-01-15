package iot.pot.messaging;

import iot.pot.database.model.User;

public interface MeasurementNotificationHandler {
    void sendMessage(User user, String message);
}