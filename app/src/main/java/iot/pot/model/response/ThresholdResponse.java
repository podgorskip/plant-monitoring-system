package iot.pot.model.response;

import lombok.AllArgsConstructor;
import lombok.Getter;

@Getter
@AllArgsConstructor
public class ThresholdResponse {
    private Double lower;
    private Double upper;
}
