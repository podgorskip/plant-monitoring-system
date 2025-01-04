package iot.pot.resource;

import iot.pot.database.model.*;
import iot.pot.endpoints.DeviceEndpoints;
import iot.pot.model.mapper.DeviceMapper;
import iot.pot.model.mapper.MeasurementMapper;
import iot.pot.model.request.DeviceRequest;
import iot.pot.model.response.DeviceResponse;
import iot.pot.model.response.MeasurementResponse;
import iot.pot.services.DeviceService;
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

    @Override
    public ResponseEntity<Page<MeasurementResponse>> getAirHumidity(Long id, Integer size, Integer page, String sortBy, String sortDir) {
        Sort sort = Sort.by(sortBy, sortDir);
        Pageable pageable = PageRequest.of(page, size, sort);
        Page<AirHumidity> result = deviceService.getAirHumidity(id, pageable);
        return ResponseEntity.status(HttpStatus.OK).body(result.map(MeasurementMapper::toMeasurementResponse));
    }

    @Override
    public ResponseEntity<Page<MeasurementResponse>> getSoilHumidity(Long id, Integer size, Integer page, String sortBy, String sortDir) {
        Sort sort = Sort.by(sortBy, sortDir);
        Pageable pageable = PageRequest.of(page, size, sort);
        Page<SoilHumidity> result = deviceService.getSoilHumidity(id, pageable);
        return ResponseEntity.status(HttpStatus.OK).body(result.map(MeasurementMapper::toMeasurementResponse));
    }

    @Override
    public ResponseEntity<Page<MeasurementResponse>> getTemperature(Long id, Integer size, Integer page, String sortBy, String sortDir) {
        Sort sort = Sort.by(sortBy, sortDir);
        Pageable pageable = PageRequest.of(page, size, sort);
        Page<Temperature> result = deviceService.getTemperature(id, pageable);
        return ResponseEntity.status(HttpStatus.OK).body(result.map(MeasurementMapper::toMeasurementResponse));
    }

    @Override
    public ResponseEntity<Page<MeasurementResponse>> getInsolation(Long id, Integer size, Integer page, String sortBy, String sortDir) {
        Sort sort = Sort.by(sortBy, sortDir);
        Pageable pageable = PageRequest.of(page, size, sort);
        Page<Insolation> result = deviceService.getInsolation(id, pageable);
        return ResponseEntity.status(HttpStatus.OK).body(result.map(MeasurementMapper::toMeasurementResponse));
    }

}
