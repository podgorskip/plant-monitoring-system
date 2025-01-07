package iot.pot.services;

import iot.pot.database.model.Device;
import iot.pot.database.model.SoilHumidity;
import iot.pot.database.repositories.SoilHumidityRepository;
import iot.pot.model.MeasurementInterface;
import iot.pot.model.enums.MeasurementEnum;
import iot.pot.validation.ThresholdVerifier;
import lombok.RequiredArgsConstructor;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.stereotype.Service;
import java.nio.ByteBuffer;

@Service
@RequiredArgsConstructor
public class SoilHumidityService implements MeasurementInterface {
    private final SoilHumidityRepository soilHumidityRepository;
    private final ThresholdVerifier thresholdVerifier;

    @Override
    public void save(byte[] message, Device device) {
        double value = ByteBuffer.wrap(message).getDouble();

        SoilHumidity soilHumidity = new SoilHumidity();
        soilHumidity.setDevice(device);
        soilHumidity.setValue(value);

        thresholdVerifier.verifyThreshold(
                MeasurementEnum.SOIL_HUMIDITY,
                device.getAirHumidityLowerThreshold(),
                device.getAirHumidityUpperThreshold(),
                value,
                device
        );

        soilHumidityRepository.save(soilHumidity);
    }

    public Page<SoilHumidity> getByDevice(Device device, Pageable pageable) {
        return soilHumidityRepository.findByDevice(device, pageable);
    }
}