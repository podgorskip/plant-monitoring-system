package iot.pot.services;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import iot.pot.database.model.Device;
import iot.pot.database.model.InsolationDigital;
import iot.pot.database.repositories.InsolationDigitalRepository;
import iot.pot.model.MeasurementInterface;
import iot.pot.model.enums.MeasurementEnum;
import iot.pot.utils.ExecutorsPool;
import iot.pot.validation.ThresholdVerifier;
import lombok.RequiredArgsConstructor;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.stereotype.Service;

@Service
@RequiredArgsConstructor
public class InsolationDigitalService implements MeasurementInterface {
    private final InsolationDigitalRepository insolationDigitalRepository;
    private final ThresholdVerifier thresholdVerifier;
    private final ObjectMapper objectMapper;

    @Override
    public void save(byte[] message, Device device) {
        String messageString = new String(message);

        try {
            System.out.println(messageString);
            InsolationDigital insolationDigital = objectMapper.readValue(messageString, InsolationDigital.class);
            insolationDigital.setDevice(device);
            insolationDigitalRepository.save(insolationDigital);

            ExecutorsPool.executorService.submit(() -> {
                thresholdVerifier.verifyThreshold(
                        MeasurementEnum.INSOLATION_DIGITAL,
                        device.getInsolationLowerThreshold(),
                        device.getInsolationUpperThreshold(),
                        insolationDigital.getValue(),
                        device);
            });

        } catch (JsonProcessingException e) {
            throw new RuntimeException();
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public Page<InsolationDigital> getByDevice(Device device, Pageable pageable) {
        return insolationDigitalRepository.findByDevice(device, pageable);
    }
}
