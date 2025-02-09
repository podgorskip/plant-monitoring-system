package iot.pot.services;

import iot.pot.database.model.Device;
import iot.pot.model.MeasurementInterface;
import iot.pot.validation.ThresholdVerifier;
import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Service;

@Service
@RequiredArgsConstructor
public class WaterLevelService implements MeasurementInterface {
    private final ThresholdVerifier thresholdVerifier;

    @Override
    public void save(byte[] message, Device device) {
        thresholdVerifier.sendWaterAlert(device);
    }
}
