package iot.pot.services;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import iot.pot.database.model.AirHumidity;
import iot.pot.database.model.Device;
import iot.pot.database.repositories.AirHumidityRepository;
import iot.pot.model.enums.Measurement;
import iot.pot.mqtt.MqttDataHandler;
import iot.pot.validation.ThresholdVerifier;
import lombok.RequiredArgsConstructor;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.stereotype.Service;

@Service
@RequiredArgsConstructor
public class AirHumidityService implements MqttDataHandler {
    private final AirHumidityRepository airHumidityRepository;
    private final ThresholdVerifier thresholdVerifier;
    private final ObjectMapper objectMapper;

    @Override
    public void save(byte[] message, Device device) {
        String messageString = new String(message);

        try {
            AirHumidity airHumidity = objectMapper.readValue(messageString, AirHumidity.class);
            airHumidity.setDevice(device);

            System.out.println(airHumidity);

//            thresholdVerifier.verifyThreshold(
//                    Measurement.AIR_HUMIDITY,
//                    device.getAirHumidityLowerThreshold(),
//                    device.getAirHumidityUpperThreshold(),
//                    airHumidity.getValue(),
//                    device.getUser()
//            );

            airHumidityRepository.save(airHumidity);

        } catch (JsonProcessingException e) {
            throw new RuntimeException();
        }
    }

    public Page<AirHumidity> findByDevice(Device device, Pageable pageable) {
        return airHumidityRepository.findByDevice(device, pageable);
    }
}
