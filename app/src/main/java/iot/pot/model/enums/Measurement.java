package iot.pot.model.enums;

import lombok.AllArgsConstructor;
import lombok.Getter;

@Getter
@AllArgsConstructor
public enum Measurement {
    AIR_HUMIDITY(new MeasurementDetails(
            "humidity",
            "Air humidity has dropped below threshold [%d allowed, %d current]",
            "Air humidity has raised above threshold [%d allowed, %d current]"
    )),
    SOIL_HUMIDITY(new MeasurementDetails(
            "soil_humidity",
            "Soil humidity has dropped below threshold [%d allowed, %d current]",
            "Soil humidity has raised above threshold [%d allowed, %d current]"
    )),
    TEMPERATURE(new MeasurementDetails(
            "temperature",
            "Temperature has dropped below threshold [%d allowed, %d current]",
            "Temperature has raised above threshold [%d allowed, %d current]"
    )),
    INSOLATION(new MeasurementDetails(
            "insolation",
            "Insolation has dropped below threshold [%d allowed, %d current]",
            "Insolation has raised above threshold [%d allowed, %d current]"
    ));;

    private final MeasurementDetails measurementDetails;

    @Getter
    @AllArgsConstructor
    public static class MeasurementDetails {
        private String topic;
        private String lowerThresholdMessageTemplate;
        private String upperThresholdMessageTemplate;
    }
}
