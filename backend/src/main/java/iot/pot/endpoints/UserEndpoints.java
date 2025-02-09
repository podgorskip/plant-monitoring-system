package iot.pot.endpoints;

import iot.pot.model.request.UserRequest;
import iot.pot.model.response.DeviceResponse;
import iot.pot.model.response.UserResponse;
import jakarta.validation.Valid;
import jakarta.validation.constraints.NotNull;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;
import java.util.List;

@RestController
@RequestMapping("/users")
public interface UserEndpoints {

    @GetMapping("/{userMac}/devices/{deviceMac}")
    ResponseEntity<String> registerUserWithDevice(
            @NotNull @PathVariable("userMac") String userMac,
            @NotNull @PathVariable("deviceMac") String deviceMac
    );

    @PostMapping("/{userMac}/devices/{deviceMac}")
    ResponseEntity<UserResponse> createUser(
            @NotNull @PathVariable("userMac") String userMac,
            @NotNull @PathVariable("deviceMac") String deviceMac,
            @Valid @RequestBody UserRequest request
    );

    @GetMapping("/{id}")
    ResponseEntity<UserResponse> getUser(@NotNull @PathVariable Long id);

    @GetMapping(value = "/{id}/devices", produces = "application/json")
    ResponseEntity<List<DeviceResponse>> getUserDevices(@NotNull @PathVariable Long id);
}
