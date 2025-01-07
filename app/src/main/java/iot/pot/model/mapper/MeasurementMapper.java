package iot.pot.model.mapper;

import iot.pot.database.model.Measurement;
import iot.pot.model.response.MeasurementResponse;

public class MeasurementMapper {
    public static MeasurementResponse toMeasurementResponse(Measurement measurement) {
        return MeasurementResponse.builder()
                .id(measurement.getId())
                .value(measurement.getValue())
                .date(measurement.getDate())
                .build();
    }
}
