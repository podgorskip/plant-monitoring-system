package iot.pot.resource;

import iot.pot.database.model.*;
import iot.pot.endpoints.DeviceEndpoints;
import iot.pot.model.enums.MeasurementEnum;
import iot.pot.model.mapper.MeasurementMapper;
import iot.pot.model.request.FrequencyRequest;
import iot.pot.model.request.ThresholdRequest;
import iot.pot.model.response.FrequencyResponse;
import iot.pot.model.response.MeasurementResponse;
import iot.pot.model.response.ThresholdResponse;
import iot.pot.services.DeviceService;
import iot.pot.services.SoilHumidityService;
import lombok.RequiredArgsConstructor;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.PageRequest;
import org.springframework.data.domain.Pageable;
import org.springframework.data.domain.Sort;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/devices")
@RequiredArgsConstructor
public class DeviceResource implements DeviceEndpoints {
    private final DeviceService deviceService;
    private final SoilHumidityService soilHumidityService;

    @Override
    public ResponseEntity<Page<MeasurementResponse>> getAirHumidity(Long id, MeasurementEnum measurement, Integer size, Integer page, String sortBy, String sortDir) {
        Sort.Direction direction = Sort.Direction.fromString(sortDir);
        Sort sort = Sort.by(direction, sortBy);
        Pageable pageable = PageRequest.of(page, size, sort);
        Page<? extends Measurement> result = deviceService.getMeasurement(id, measurement, pageable);
        return ResponseEntity.status(HttpStatus.OK).body(result.map(MeasurementMapper::toMeasurementResponse));
    }

    @Override
    public ResponseEntity<String> setThreshold(Long id, ThresholdRequest request, MeasurementEnum measurementEnum) {
        deviceService.setThresholds(id, measurementEnum, request.getLower(), request.getUpper());
        return ResponseEntity.status(HttpStatus.OK).build();
    }

    @Override
    public ResponseEntity<String> sendWaterRequest(Long id, Integer time) {
        soilHumidityService.sendWaterRequest(id, time);
        return ResponseEntity.status(HttpStatus.OK).build();
    }

    @Override
    public ResponseEntity<ThresholdResponse> getThresholdForMeasurement(Long id, MeasurementEnum measurementEnum) {
        return ResponseEntity.ok(deviceService.getThresholdForMeasurement(id, measurementEnum));
    }

    @Override
    public ResponseEntity<FrequencyResponse> getFrequencyResponse(Long id, MeasurementEnum measurement) {
        return ResponseEntity.ok(deviceService.getFrequencyForMeasurement(id, measurement));
    }

    @Override
    public ResponseEntity<String> setFrequency(Long id, FrequencyRequest request, MeasurementEnum measurement) {
        deviceService.setFrequency(id, measurement, request.getFrequency());
        return ResponseEntity.status(HttpStatus.OK).build();
    }
}
