package iot.pot.messaging;

import com.twilio.Twilio;
import com.twilio.rest.api.v2010.account.Message;
import com.twilio.type.PhoneNumber;
import iot.pot.database.model.User;
import org.springframework.context.annotation.Primary;
import org.springframework.core.env.Environment;
import org.springframework.stereotype.Service;

@Primary
@Service
public class SmsService implements MeasurementNotificationHandler {
    private final String ACCOUNT_SID;
    private final String AUTH_TOKEN;
    private final String PHONE_NUMBER;
    private final String EXTENSION;

    public SmsService(Environment env) {
        ACCOUNT_SID = env.getProperty("sms.twilio.account-sid");
        AUTH_TOKEN = env.getProperty("sms.twilio.auth-token");
        PHONE_NUMBER = env.getProperty("sms.twilio.phone-number");
        EXTENSION = env.getProperty("sms.extension");
    }

    @Override
    public void sendMessage(User user, String message) {
        System.out.println("Sending measurement alert.");

        Twilio.init(ACCOUNT_SID, AUTH_TOKEN);
        Message.creator(
                new PhoneNumber(EXTENSION + user.getPhoneNumber()),
                new PhoneNumber(PHONE_NUMBER),
                message)
        .create();
    }
}
