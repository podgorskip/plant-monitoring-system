package iot.pot.services;

import iot.pot.database.model.Device;
import iot.pot.database.model.SoilHumidity;
import iot.pot.database.repositories.DeviceRepository;
import iot.pot.database.repositories.SoilHumidityRepository;
import iot.pot.exceptions.DeviceException;
import iot.pot.model.MeasurementInterface;
import iot.pot.model.enums.MeasurementEnum;
import iot.pot.mqtt.MqttConnector;
import iot.pot.validation.ThresholdVerifier;
import lombok.RequiredArgsConstructor;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.stereotype.Service;
import java.nio.ByteBuffer;
import java.util.Objects;

@Service
@RequiredArgsConstructor
public class SoilHumidityService implements MeasurementInterface {
    private final SoilHumidityRepository soilHumidityRepository;
    private final ThresholdVerifier thresholdVerifier;
    private final MqttConnector mqttConnector;
    private final WaterRequestService waterRequestService;
    private final DeviceRepository deviceRepository;

    @Override
    public void save(byte[] message, Device device) {
        double value = ByteBuffer.wrap(message).getDouble();

        SoilHumidity soilHumidity = new SoilHumidity();
        soilHumidity.setDevice(device);
        soilHumidity.setValue(value);

        if (Objects.nonNull(device.getSoilHumidityLowerThreshold()) && value < device.getSoilHumidityLowerThreshold()) {
            sendWaterRequest(device, 30);
        }

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

    public void sendWaterRequest(Long id, Integer time) {
        Device device = deviceRepository.findById(id)
                .orElseThrow(() -> new DeviceException(DeviceException.ExceptionType.NOT_FOUND));
        sendWaterRequest(device, time);
    }

    public void sendWaterRequest(Device device, Integer time) {
        if (Objects.nonNull(time) && time > 0) {
            String topic = String.format("%s/%s/%s/request", device.getUser().getMac(), device.getMac(), MeasurementEnum.SOIL_HUMIDITY.getMeasurementDetails().getTopic());
            mqttConnector.publish(topic, String.valueOf(time));
            waterRequestService.createWaterRequest(device, time);
        }
    }
}