package iot.pot.mqtt;

import iot.pot.database.model.Device;
import iot.pot.database.model.User;
import iot.pot.exceptions.BrokerException;
import iot.pot.model.enums.MeasurementEnum;
import org.eclipse.paho.client.mqttv3.*;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;
import org.springframework.core.env.Environment;
import org.springframework.stereotype.Component;
import java.util.List;

@Component
public class MqttConnector {
    private final MqttClient client;

    public MqttConnector(Environment env) throws MqttException {
        String broker = env.getProperty("mqtt.broker.url");
        String username = env.getProperty("mqtt.broker.username");
        String password = env.getProperty("mqtt.broker.password");
        String clientId = env.getProperty("mqtt.broker.client");

        client = new MqttClient(broker, clientId, new MemoryPersistence());

        MqttConnectOptions connOpts = new MqttConnectOptions();
        connOpts.setUserName(username);
        connOpts.setPassword(password.toCharArray());
        connOpts.setKeepAliveInterval(60);

        client.connect(connOpts);
    }

    public void subscribe(Device device, List<SubscribeParam> params) {
        final User user = device.getUser();

        params.forEach(param -> {
            try {
                client.subscribe(String.format("%s/%s/%s", user.getMac(), device.getMac(), param.getMeasurementEnum().getMeasurementDetails().getTopic()) , (t, message) ->
                    param.getDataHandler().save(message.getPayload(), device)
                );
            } catch (MqttException e) {
                throw new BrokerException("Broker exception: " + e.getMessage());
            }
        });
    }

    public void publish(Device device, MeasurementEnum measurement, String value) {
        String topic = String.format("%s/%s/%s/app", device.getUser().getMac(), device.getMac(), measurement.getMeasurementDetails().getTopic());
        MqttMessage message = new MqttMessage(value.getBytes());

        try {
            client.publish(topic, message);
        } catch (MqttException e) {
            throw new BrokerException("Broker exception: " + e.getMessage());
        }
    }
}
