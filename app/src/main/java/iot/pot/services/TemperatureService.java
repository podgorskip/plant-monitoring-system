package iot.pot.services;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import iot.pot.database.model.Device;
import iot.pot.database.model.Temperature;
import iot.pot.database.repositories.TemperatureRepository;
import iot.pot.model.MeasurementInterface;
import iot.pot.model.enums.MeasurementEnum;
import iot.pot.validation.ThresholdVerifier;
import lombok.RequiredArgsConstructor;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.stereotype.Service;

@Service
@RequiredArgsConstructor
public class TemperatureService implements MeasurementInterface {
    private final TemperatureRepository temperatureRepository;
    private final ThresholdVerifier thresholdVerifier;
    private final ObjectMapper objectMapper;

    @Override
    public void save(byte[] message, Device device) {
        String messageString = new String(message);

        try {
            System.out.println(messageString);
            Temperature temperature = objectMapper.readValue(messageString, Temperature.class);
            temperature.setDevice(device);
            temperatureRepository.save(temperature);

            thresholdVerifier.verifyThreshold(
                    MeasurementEnum.TEMPERATURE,
                    device.getTemperatureLowerThreshold(),
                    device.getTemperatureUpperThreshold(),
                    temperature.getValue(),
                    device
            );

        } catch (JsonProcessingException e) {
            System.out.println(e);
            throw new RuntimeException();
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public Page<Temperature> getByDevice(Device device, Pageable pageable) {
        return temperatureRepository.findByDevice(device, pageable);
    }
}
