package iot.pot.mqtt;

import iot.pot.model.enums.Measurement;
import lombok.AllArgsConstructor;
import lombok.Getter;

@Getter
@AllArgsConstructor
public class SubscribeParam {
    private Measurement measurement;
    private MqttDataHandler dataHandler;
}
