package iot.pot.model.response;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;
import java.time.LocalDateTime;

@Data
@Builder
@AllArgsConstructor
@NoArgsConstructor
public class MeasurementResponse {
    private Long id;
    private Double value;
    private LocalDateTime date;
    private String unit;
}
