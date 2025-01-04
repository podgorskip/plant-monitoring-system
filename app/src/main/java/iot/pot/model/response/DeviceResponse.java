package iot.pot.model.response;

import lombok.Builder;
import lombok.Data;
import java.time.LocalDateTime;

@Data
@Builder
public class DeviceResponse {
    private Long id;
    private String deviceNumber;
    private UserResponse user;
    private LocalDateTime creationDate;
}
