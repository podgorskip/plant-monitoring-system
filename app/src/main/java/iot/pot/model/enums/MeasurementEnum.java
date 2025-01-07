package iot.pot.model.enums;

import lombok.AllArgsConstructor;
import lombok.Getter;

@Getter
@AllArgsConstructor
public enum MeasurementEnum {
    AIR_HUMIDITY(new MeasurementDetails(
            "humidity",
            "Oops! Air humidity has dropped below threshold [%d allowed, %d current]. Time to grab a humidifier!",
            "Whoa! Air humidity has raised above threshold [%d allowed, %d current]. Feels like a rainforest in here!"
    )),
    SOIL_HUMIDITY(new MeasurementDetails(
            "soil_humidity",
            "Alert! Soil humidity has dropped below threshold [%d allowed, %d current]. Your plants might be thirsty!",
            "Heads up! Soil humidity has raised above threshold [%d allowed, %d current]. Hope your plants don't drown!"
    )),
    TEMPERATURE(new MeasurementDetails(
            "temperature",
            "Brr! Temperature has dropped below threshold [%d allowed, %d current]. Maybe turn up the heat?",
            "Yikes! Temperature has raised above threshold [%d allowed, %d current]. Someone turn on the AC!"
    )),
    INSOLATION(new MeasurementDetails(
            "insolation",
            "Uh-oh! Insolation has dropped below threshold [%d allowed, %d current]. Is it cloudy out there?",
            "Wow! Insolation has raised above threshold [%d allowed, %d current]. Don't forget your sunscreen!"
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
