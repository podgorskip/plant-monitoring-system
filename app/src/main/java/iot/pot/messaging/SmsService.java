package iot.pot.messaging;

import iot.pot.database.model.User;
import org.springframework.stereotype.Service;

@Service
public class SmsService implements MeasurementNotificationHandler {

    @Override
    public void sendMessage(User user, String message) {

    }
}
