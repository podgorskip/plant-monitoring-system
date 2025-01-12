package iot.pot.services;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import iot.pot.database.model.Device;
import iot.pot.database.model.Insolation;
import iot.pot.database.model.Temperature;
import iot.pot.database.repositories.InsolationRepository;
import iot.pot.model.MeasurementInterface;
import iot.pot.model.enums.MeasurementEnum;
import iot.pot.validation.ThresholdVerifier;
import lombok.RequiredArgsConstructor;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.scheduling.annotation.Async;
import org.springframework.stereotype.Service;
import java.nio.ByteBuffer;

@Service
@RequiredArgsConstructor
public class InsolationService implements MeasurementInterface {
    private final InsolationRepository insolationRepository;
    private final ThresholdVerifier thresholdVerifier;
    private final ObjectMapper objectMapper;

    @Override
    public void save(byte[] message, Device device) {
        String messageString = new String(message);

        try {
            System.out.println(messageString);
            Insolation insolation = objectMapper.readValue(messageString, Insolation.class);
            insolation.setDevice(device);

            insolationRepository.save(insolation);

            thresholdVerifier.verifyThreshold(
                    MeasurementEnum.INSOLATION,
                    device.getInsolationLowerThreshold(),
                    device.getInsolationUpperThreshold(),
                    insolation.getValue(),
                    device
            );

        } catch (JsonProcessingException e) {
            System.out.println(e);
            throw new RuntimeException();
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public Page<Insolation> getByDevice(Device device, Pageable pageable) {
        return insolationRepository.findByDevice(device, pageable);
    }
}
