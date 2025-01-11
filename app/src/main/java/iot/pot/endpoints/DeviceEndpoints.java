package iot.pot.endpoints;

import iot.pot.model.enums.MeasurementEnum;
import iot.pot.model.request.FrequencyRequest;
import iot.pot.model.request.ThresholdRequest;
import iot.pot.model.response.FrequencyResponse;
import iot.pot.model.response.MeasurementResponse;
import iot.pot.model.response.ThresholdResponse;
import jakarta.validation.constraints.NotNull;
import org.springframework.data.domain.Page;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/devices")
public interface DeviceEndpoints {
    @GetMapping("/{id}/measurements")
    ResponseEntity<Page<MeasurementResponse>> getAirHumidity(
            @NotNull @PathVariable Long id,
            @RequestParam("measurement") MeasurementEnum measurement,
            @RequestParam(value = "size", required = false, defaultValue = "10") Integer size,
            @RequestParam(value = "page", required = false, defaultValue = "0") Integer page,
            @RequestParam(value = "sortBy", required = false, defaultValue = "date") String sortBy,
            @RequestParam(value = "sortDir", required = false, defaultValue = "DESC") String sortDir
    );

    @PatchMapping("/{id}/thresholds")
    ResponseEntity<String> setThreshold(
            @NotNull @PathVariable("id") Long id,
            @RequestBody ThresholdRequest request,
            @RequestParam("measurement") MeasurementEnum measurement
    );

    @PostMapping("/{id}/watering")
    ResponseEntity<String> sendWaterRequest(
            @NotNull @PathVariable("id") Long id,
            @RequestParam("time") Integer time
    );

    @GetMapping("/{id}/thresholds")
    ResponseEntity<ThresholdResponse> getThresholdForMeasurement(
            @NotNull @PathVariable Long id,
            @RequestParam("measurement") MeasurementEnum measurement
    );

    @GetMapping("/{id}/frequency")
    ResponseEntity<FrequencyResponse> getFrequencyResponse(
            @NotNull @PathVariable Long id,
            @RequestParam("measurement") MeasurementEnum measurement
    );

    @PatchMapping("/{id}/frequency")
    ResponseEntity<String> setFrequency(
            @NotNull @PathVariable("id") Long id,
            @RequestBody FrequencyRequest request,
            @RequestParam("measurement") MeasurementEnum measurement
    );
}
