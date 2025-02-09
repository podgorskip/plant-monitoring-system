package iot.pot.model.response;

import lombok.Builder;
import lombok.Data;
import java.time.LocalDateTime;

@Data
@Builder
public class DeviceResponse {
    private Long id;
    private String deviceNumber;
    private String name;
    private String info;
    private LocalDateTime creationDate;
}
