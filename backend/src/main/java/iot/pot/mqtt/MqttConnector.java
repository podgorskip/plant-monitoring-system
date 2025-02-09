package iot.pot.mqtt;

import iot.pot.database.model.Device;
import iot.pot.database.model.User;
import iot.pot.exceptions.BrokerException;
import org.eclipse.paho.client.mqttv3.*;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;
import org.springframework.core.env.Environment;
import org.springframework.stereotype.Component;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

@Component
public class MqttConnector implements MqttCallback {
    private final MqttClient client;
    private int retryAttempts = 0;
    private boolean reconnectionInProgress = false;
    private List<SubscribeParam> subscribeParams;
    private final ExecutorService executorService;
    private Device device;

    public MqttConnector(Environment env) throws MqttException {
        String broker = env.getProperty("mqtt.broker.url");
        String username = env.getProperty("mqtt.broker.username");
        String password = env.getProperty("mqtt.broker.password");
        String clientId = env.getProperty("mqtt.broker.client");

        client = new MqttClient(broker, clientId, new MemoryPersistence());
        client.setCallback(this);

        MqttConnectOptions connOpts = new MqttConnectOptions();
        connOpts.setUserName(username);
        connOpts.setPassword(password.toCharArray());
        connOpts.setAutomaticReconnect(false);
        connOpts.setKeepAliveInterval(60);

        this.executorService = Executors.newCachedThreadPool();

        client.connect(connOpts);
    }

    @Override
    public void connectionLost(Throwable cause) {
        System.err.println("Connection lost: " + cause.getMessage());

        if (!reconnectionInProgress) {
            reconnectionInProgress = true;
            try {
                System.out.println("Attempting to reconnect...");

                int MAX_RETRY_ATTEMPTS = 5;
                while (!client.isConnected() && retryAttempts < MAX_RETRY_ATTEMPTS) {
                    try {
                        client.reconnect();
                        retryAttempts = 0;
                        System.out.println("Reconnected to broker.");
                        break;
                    } catch (MqttException e) {
                        retryAttempts++;
                        System.err.println("Reconnection failed (attempt " + retryAttempts + "): " + e.getMessage());
                        if (retryAttempts >= MAX_RETRY_ATTEMPTS) {
                            System.err.println("Max retry attempts reached, stopping reconnection attempts.");
                            break;
                        }
                        Thread.sleep(5000);
                    }
                }
            } catch (InterruptedException e) {
                System.err.println("Interrupted while waiting to reconnect: " + e.getMessage());
            } finally {
                reconnectionInProgress = false;
            }
        } else {
            System.out.println("Reconnect already in progress, waiting...");
        }
    }

    @Override
    public void messageArrived(String topic, MqttMessage message)  {
        System.out.printf("Message arrived: Topic=%s, Payload=%s\n", topic, new String(message.getPayload()));
        handleData(topic, message);
    }

    @Override
    public void deliveryComplete(IMqttDeliveryToken token) {
        System.out.println("Message delivery complete for message ID: " + token.getMessageId());
    }

    public void subscribe(Device device, List<SubscribeParam> params) {
        System.out.printf("Subscription started, device id=%s\n", device.getId());

        final User user = device.getUser();
        this.subscribeParams = params;
        this.device = device;

        params.forEach(param -> {
            String topic = String.format("%s/%s/%s", user.getMac(), device.getMac(), param.getMeasurementEnum().getMeasurementDetails().getTopic());

            try {
                client.subscribe(topic);
                System.out.printf("Subscribed to topic: %s\n", topic);
            } catch (MqttException e) {
                System.err.println("Broker exception: " + e.getMessage());
            }
        });
    }

    public void publish(String topic, String value) {
        MqttMessage message = new MqttMessage(value.getBytes());
        System.out.printf("Publishing to topic: %s, value: %s\n", topic, value);

        try {
            client.publish(topic, message);
        } catch (MqttException e) {
            throw new BrokerException("Broker exception: " + e.getMessage());
        }
    }

    private void handleData(String topic,  MqttMessage message) {
        executorService.submit(() -> {
            subscribeParams.stream()
                    .filter(param -> {
                        String measurement = topic.substring(topic.lastIndexOf("/") + 1);
                        return param.getMeasurementEnum().getMeasurementDetails().getTopic().equals(measurement);
                    })
                    .findFirst()
                    .ifPresent(param -> {
                        System.out.println(param);
                        param.getDataHandler().save(message.getPayload(), device);
                    });
        });
    }
}
