package iot.pot.services;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import iot.pot.database.model.AirHumidity;
import iot.pot.database.model.Device;
import iot.pot.database.repositories.AirHumidityRepository;
import iot.pot.model.MeasurementInterface;
import iot.pot.model.enums.MeasurementEnum;
import iot.pot.validation.ThresholdVerifier;
import lombok.RequiredArgsConstructor;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.scheduling.annotation.Async;
import org.springframework.stereotype.Service;

@Service
@RequiredArgsConstructor
public class AirHumidityService implements MeasurementInterface {
    private final AirHumidityRepository airHumidityRepository;
    private final ThresholdVerifier thresholdVerifier;
    private final ObjectMapper objectMapper;

    @Override
    public void save(byte[] message, Device device) {
        String messageString = new String(message);

        try {
            System.out.println(messageString);
            AirHumidity airHumidity = objectMapper.readValue(messageString, AirHumidity.class);
            airHumidity.setDevice(device);

            thresholdVerifier.verifyThreshold(
                    MeasurementEnum.AIR_HUMIDITY,
                    device.getAirHumidityLowerThreshold(),
                    device.getAirHumidityUpperThreshold(),
                    airHumidity.getValue(),
                    device
            );

            airHumidityRepository.save(airHumidity);

        } catch (JsonProcessingException e) {
            throw new RuntimeException();
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public Page<AirHumidity> getByDevice(Device device, Pageable pageable) {
        return airHumidityRepository.findByDevice(device, pageable);
    }
}
