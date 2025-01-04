package iot.pot.endpoints;

import iot.pot.model.request.DeviceRequest;
import iot.pot.model.response.DeviceResponse;
import iot.pot.model.response.MeasurementResponse;
import jakarta.validation.constraints.NotNull;
import org.springframework.data.domain.Page;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/devices")
public interface DeviceEndpoints {
    @GetMapping("/{id}/air-humidity")
    ResponseEntity<Page<MeasurementResponse>> getAirHumidity(
            @NotNull @PathVariable Long id,
            @RequestParam(value = "size", required = false, defaultValue = "10") Integer size,
            @RequestParam(value = "page", required = false, defaultValue = "0") Integer page,
            @RequestParam(value = "sortBy", required = false, defaultValue = "date") String sortBy,
            @RequestParam(value = "sortDir", required = false, defaultValue = "DESC") String sortDir
    );

    @GetMapping("/{id}/soil-humidity")
    ResponseEntity<Page<MeasurementResponse>> getSoilHumidity(
            @NotNull @PathVariable Long id,
            @RequestParam(value = "size", required = false, defaultValue = "10") Integer size,
            @RequestParam(value = "page", required = false, defaultValue = "0") Integer page,
            @RequestParam(value = "sortBy", required = false, defaultValue = "date") String sortBy,
            @RequestParam(value = "sortDir", required = false, defaultValue = "DESC") String sortDir
    );

    @GetMapping("/{id}/temperature")
    ResponseEntity<Page<MeasurementResponse>> getTemperature(
            @NotNull @PathVariable Long id,
            @RequestParam(value = "size", required = false, defaultValue = "10") Integer size,
            @RequestParam(value = "page", required = false, defaultValue = "0") Integer page,
            @RequestParam(value = "sortBy", required = false, defaultValue = "date") String sortBy,
            @RequestParam(value = "sortDir", required = false, defaultValue = "DESC") String sortDir
    );

    @GetMapping("/{id}/insolation")
    ResponseEntity<Page<MeasurementResponse>> getInsolation(
            @NotNull @PathVariable Long id,
            @RequestParam(value = "size", required = false, defaultValue = "10") Integer size,
            @RequestParam(value = "page", required = false, defaultValue = "0") Integer page,
            @RequestParam(value = "sortBy", required = false, defaultValue = "date") String sortBy,
            @RequestParam(value = "sortDir", required = false, defaultValue = "DESC") String sortDir
    );
}
