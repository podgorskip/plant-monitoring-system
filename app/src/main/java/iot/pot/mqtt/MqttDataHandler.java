package iot.pot.mqtt;

import iot.pot.database.model.Device;

public interface MqttDataHandler {
    void save(byte[] message, Device device);
}
