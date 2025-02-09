package iot.pot.database.model.enums;

import lombok.AllArgsConstructor;
import java.util.List;
import static iot.pot.database.model.enums.Privilege.*;

@AllArgsConstructor
public enum Role {
    ADMIN(List.of(REGISTER_DEVICE, CHECK_DATA)),
    CUSTOMER(List.of(CHECK_DATA));

    private final List<Privilege> privileges;
}
