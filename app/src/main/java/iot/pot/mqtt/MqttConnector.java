package iot.pot.mqtt;

import iot.pot.database.model.Device;
import iot.pot.database.model.User;
import lombok.RequiredArgsConstructor;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;
import org.springframework.core.env.Environment;
import org.springframework.stereotype.Component;
import java.util.List;
import java.util.Objects;

@Component
@RequiredArgsConstructor
public class MqttConnector {
    private final Environment env;

    public void subscribe(Device device, List<SubscribeParam> params) {
        final String broker = env.getProperty("mqtt.broker.url");
        final String username = env.getProperty("mqtt.broker.username");
        final String password = env.getProperty("mqtt.broker.password");

        final User user = device.getUser();
        final String userMac = user.getMac();

        try {
            MqttClient client = new MqttClient(broker, userMac, new MemoryPersistence());
            MqttConnectOptions connOpts = new MqttConnectOptions();
            connOpts.setUserName(username);
            connOpts.setPassword(password.toCharArray());
            connOpts.setKeepAliveInterval(60);

            client.connect(connOpts);

            System.out.println("Connected to broker: " + broker);

            params.forEach(param -> {
                try {
                    System.out.println("Subscribing to: " + String.format("%s/%s/%s", user.getMac(), device.getMac(), param.getMeasurement().getMeasurementDetails().getTopic()));
                    client.subscribe(String.format("%s/%s/%s", user.getMac(), device.getMac(), param.getMeasurement().getMeasurementDetails().getTopic()) , (t, message) -> {
                        System.out.println("Received message on topic: " + t);
                        System.out.println("Payload: " + new String(message.getPayload()));

                        // Call the data handler's save method
                        param.getDataHandler().save(message.getPayload(), device);
                    });

                } catch (MqttException e) {
                    throw new RuntimeException(e);
                } catch (Exception e) {
                    System.out.println(e);
                }
            });

        } catch (MqttException e) {
            throw new RuntimeException(e);
        }
    }
}
