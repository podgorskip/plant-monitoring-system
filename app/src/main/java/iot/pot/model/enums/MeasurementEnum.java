package iot.pot.model.enums;

import lombok.AllArgsConstructor;
import lombok.Getter;

@Getter
@AllArgsConstructor
public enum MeasurementEnum {
    AIR_HUMIDITY(new MeasurementDetails(
            "air_humidity",
            "Oops! Air humidity has dropped below threshold [%d allowed, %s current]. Time to grab a humidifier!",
            "Whoa! Air humidity has raised above threshold [%d allowed, %s current]. Feels like a rainforest in here!"
    )),
    SOIL_HUMIDITY(new MeasurementDetails(
            "soil_humidity",
            "Alert! Soil humidity has dropped below threshold [%d allowed, %s current]. Your plants might be thirsty!",
            "Heads up! Soil humidity has raised above threshold [%d allowed, %s current]. Hope your plants don't drown!"
    )),
    TEMPERATURE(new MeasurementDetails(
            "temperature",
            "Brr! Temperature has dropped below threshold [%d allowed, %s current]. Maybe turn up the heat?",
            "Yikes! Temperature has raised above threshold [%d allowed, %s current]. Someone turn on the AC!"
    )),
    INSOLATION(new MeasurementDetails(
            "insolation",
            "Uh-oh! Insolation has dropped below threshold [%d allowed, %s current]. Is it cloudy out there?",
            "Wow! Insolation has raised above threshold [%d allowed, %s current]. Don't forget your sunscreen!"
    ));

    private final MeasurementDetails measurementDetails;

    @Getter
    @AllArgsConstructor
    public static class MeasurementDetails {
        private String topic;
        private String lowerThresholdMessageTemplate;
        private String upperThresholdMessageTemplate;
    }
}
