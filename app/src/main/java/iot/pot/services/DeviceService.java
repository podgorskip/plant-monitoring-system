package iot.pot.services;

import iot.pot.database.model.*;
import iot.pot.database.repositories.DeviceRepository;
import iot.pot.exceptions.DeviceException;
import iot.pot.exceptions.UserException;
import iot.pot.model.enums.Measurement;
import iot.pot.model.request.DeviceRequest;
import iot.pot.mqtt.MqttConnector;
import iot.pot.mqtt.SubscribeParam;
import lombok.RequiredArgsConstructor;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.stereotype.Service;
import java.util.List;

@Service
@RequiredArgsConstructor
public class DeviceService {
    private final DeviceRepository deviceRepository;
    private final MqttConnector mqttConnector;
    private final AirHumidityService airHumidityService;
    private final TemperatureService temperatureService;
    private final SoilHumidityService soilHumidityService;
    private final InsolationService insolationService;

    public void assignDevice(User user, String mac) {
        Device device = Device.builder()
                        .mac(mac)
                        .user(user)
                        .build();

        deviceRepository.save(device);
        mqttConnector.subscribe(device, List.of(
                new SubscribeParam(Measurement.AIR_HUMIDITY, airHumidityService),
                new SubscribeParam(Measurement.TEMPERATURE, temperatureService),
                new SubscribeParam(Measurement.SOIL_HUMIDITY, soilHumidityService),
                new SubscribeParam(Measurement.INSOLATION, insolationService)
        ));
    }

    public Page<AirHumidity> getAirHumidity(Long id, Pageable pageable) {
        Device device = deviceRepository.findById(id)
                .orElseThrow(() -> new DeviceException(DeviceException.ExceptionType.NOT_FOUND, id));
        return airHumidityService.findByDevice(device, pageable);
    }

    public Page<Temperature> getTemperature(Long id, Pageable pageable) {
        Device device = deviceRepository.findById(id)
                .orElseThrow(() -> new DeviceException(DeviceException.ExceptionType.NOT_FOUND, id));
        return temperatureService.getByDevice(device, pageable);
    }

    public Page<SoilHumidity> getSoilHumidity(Long id, Pageable pageable) {
        Device device = deviceRepository.findById(id)
                .orElseThrow(() -> new DeviceException(DeviceException.ExceptionType.NOT_FOUND, id));
        return soilHumidityService.getByDevice(device, pageable);
    }

    public Page<Insolation> getInsolation(Long id, Pageable pageable) {
        Device device = deviceRepository.findById(id)
                .orElseThrow(() -> new DeviceException(DeviceException.ExceptionType.NOT_FOUND, id));
        return insolationService.getByDevice(device, pageable);
    }
}
