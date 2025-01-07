package iot.pot.services;

import iot.pot.database.model.*;
import iot.pot.database.repositories.DeviceRepository;
import iot.pot.exceptions.DeviceException;
import iot.pot.model.enums.MeasurementEnum;
import iot.pot.model.response.ThresholdResponse;
import iot.pot.mqtt.MqttConnector;
import iot.pot.mqtt.SubscribeParam;
import lombok.RequiredArgsConstructor;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.stereotype.Service;

import java.util.EnumMap;
import java.util.List;
import java.util.Objects;
import java.util.function.BiFunction;

@Service
public class DeviceService {
    private final DeviceRepository deviceRepository;
    private final MqttConnector mqttConnector;
    private final AirHumidityService airHumidityService;
    private final TemperatureService temperatureService;
    private final SoilHumidityService soilHumidityService;
    private final InsolationService insolationService;
    private final WaterRequestService waterRequestService;

    private final EnumMap<MeasurementEnum, BiFunction<Device, Pageable, Page<? extends Measurement>>> measurementFetchers = new EnumMap<>(MeasurementEnum.class);

    public DeviceService(
            DeviceRepository deviceRepository,
            MqttConnector mqttConnector,
            AirHumidityService airHumidityService,
            TemperatureService temperatureService,
            SoilHumidityService soilHumidityService,
            InsolationService insolationService,
            WaterRequestService waterRequestService
    ) {
        this.deviceRepository = deviceRepository;
        this.mqttConnector = mqttConnector;
        this.airHumidityService = airHumidityService;
        this.temperatureService = temperatureService;
        this.soilHumidityService = soilHumidityService;
        this.insolationService = insolationService;
        this.waterRequestService = waterRequestService;

        measurementFetchers.put(MeasurementEnum.TEMPERATURE, temperatureService::getByDevice);
        measurementFetchers.put(MeasurementEnum.AIR_HUMIDITY, airHumidityService::getByDevice);
        measurementFetchers.put(MeasurementEnum.SOIL_HUMIDITY, soilHumidityService::getByDevice);
        measurementFetchers.put(MeasurementEnum.INSOLATION, insolationService::getByDevice);
    }

    public void assignDevice(User user, String mac) {
        Device device = Device.builder()
                .mac(mac)
                .user(user)
                .build();

        deviceRepository.save(device);
        mqttConnector.subscribe(device, List.of(
                new SubscribeParam(MeasurementEnum.AIR_HUMIDITY, airHumidityService),
                new SubscribeParam(MeasurementEnum.TEMPERATURE, temperatureService),
                new SubscribeParam(MeasurementEnum.SOIL_HUMIDITY, soilHumidityService),
                new SubscribeParam(MeasurementEnum.INSOLATION, insolationService)
        ));
    }

    public void setThresholds(Long id, MeasurementEnum measurementEnum, Double min, Double max) {
        Device device = findDeviceById(id);
        setMeasurementThresholds(device, measurementEnum, min, max);
        deviceRepository.save(device);
    }

    public Page<? extends Measurement> getMeasurement(Long id, MeasurementEnum measurement, Pageable pageable) {
        Device device = findDeviceById(id);
        BiFunction<Device, Pageable, Page<? extends Measurement>> fetcher = measurementFetchers.get(measurement);

        if (fetcher == null) {
            throw new DeviceException(DeviceException.ExceptionType.MEASUREMENT_NOT_FOUND);
        }

        return fetcher.apply(device, pageable);
    }

    public void sendWaterRequest(Long id, Integer time) {
        Device device = findDeviceById(id);
        if (Objects.nonNull(time) && time > 0) {
            mqttConnector.publish(device, MeasurementEnum.SOIL_HUMIDITY, String.valueOf(time));
            waterRequestService.createWaterRequest(device, time);
        }
    }

    public ThresholdResponse getThresholdForMeasurement(Long id, MeasurementEnum measurementEnum) {
        Device device = findDeviceById(id);

        return switch (measurementEnum) {
            case TEMPERATURE -> new ThresholdResponse(
                    device.getTemperatureLowerThreshold(),
                    device.getTemperatureUpperThreshold()
            );
            case AIR_HUMIDITY -> new ThresholdResponse(
                    device.getAirHumidityLowerThreshold(),
                    device.getAirHumidityUpperThreshold()
            );
            case SOIL_HUMIDITY -> new ThresholdResponse(
                    device.getSoilHumidityLowerThreshold(),
                    device.getSoilHumidityUpperThreshold()
            );
            case INSOLATION -> new ThresholdResponse(
                    device.getInsolationLowerThreshold(),
                    device.getInsolationUpperThreshold()
            );
        };
    }

    private Device findDeviceById(Long id) {
        return deviceRepository.findById(id)
                .orElseThrow(() -> new DeviceException(DeviceException.ExceptionType.NOT_FOUND, id));
    }

    private void setMeasurementThresholds(Device device, MeasurementEnum measurement, Double min, Double max) {
        switch (measurement) {
            case TEMPERATURE -> {
                device.setTemperatureLowerThreshold(min);
                device.setTemperatureUpperThreshold(max);
            }
            case AIR_HUMIDITY -> {
                device.setAirHumidityLowerThreshold(min);
                device.setAirHumidityUpperThreshold(max);
            }
            case SOIL_HUMIDITY -> {
                device.setSoilHumidityLowerThreshold(min);
                device.setSoilHumidityUpperThreshold(max);
            }
            case INSOLATION -> {
                device.setInsolationLowerThreshold(min);
                device.setInsolationUpperThreshold(max);
            }
        }
    }
}
