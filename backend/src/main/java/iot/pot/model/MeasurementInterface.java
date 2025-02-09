package iot.pot.model;

import iot.pot.database.model.Device;

public interface MeasurementInterface {
    void save(byte[] message, Device device);
}
