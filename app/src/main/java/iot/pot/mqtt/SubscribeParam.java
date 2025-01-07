package iot.pot.mqtt;

import iot.pot.model.MeasurementInterface;
import iot.pot.model.enums.MeasurementEnum;
import lombok.AllArgsConstructor;
import lombok.Getter;

@Getter
@AllArgsConstructor
public class SubscribeParam {
    private MeasurementEnum measurementEnum;
    private MeasurementInterface dataHandler;
}
