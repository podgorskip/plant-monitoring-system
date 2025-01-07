package iot.pot.services;

import iot.pot.database.model.Device;
import iot.pot.database.model.Insolation;
import iot.pot.database.repositories.InsolationRepository;
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
public class InsolationService implements MeasurementInterface {
    private final InsolationRepository insolationRepository;
    private final ThresholdVerifier thresholdVerifier;

    @Override
    public void save(byte[] message, Device device) {
        double value = ByteBuffer.wrap(message).getDouble();

        Insolation insolation = new Insolation();
        insolation.setDevice(device);
        insolation.setValue(value);

        thresholdVerifier.verifyThreshold(
                MeasurementEnum.SOIL_HUMIDITY,
                device.getAirHumidityLowerThreshold(),
                device.getAirHumidityUpperThreshold(),
                value,
                device
        );

        insolationRepository.save(insolation);
    }

    public Page<Insolation> getByDevice(Device device, Pageable pageable) {
        return insolationRepository.findByDevice(device, pageable);
    }
}
