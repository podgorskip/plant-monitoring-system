package iot.pot.resource;

import iot.pot.database.model.User;
import iot.pot.endpoints.UserEndpoints;
import iot.pot.exceptions.UserException;
import iot.pot.model.mapper.DeviceMapper;
import iot.pot.model.mapper.UserMapper;
import iot.pot.model.request.UserRequest;
import iot.pot.model.response.DeviceResponse;
import iot.pot.model.response.UserResponse;
import iot.pot.services.UserService;
import lombok.RequiredArgsConstructor;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.List;

@RestController
@RequestMapping("/users")
@RequiredArgsConstructor
public class UserResource implements UserEndpoints {
    private final UserService userService;

    @Override
    public ResponseEntity<String> registerUserWithDevice(String userMac, String deviceMac) {
        String redirectLink = String.format("localhost:8080/users/%s/devices/%s", userMac, deviceMac);
        String responseBody = "<a href='" + redirectLink + "'>Create account here</a>";
        return ResponseEntity.ok().header("Content-Type", "text/html").body(responseBody);
    }

    @Override
    public ResponseEntity<UserResponse> createUser(String userMac, String deviceMac, UserRequest request) {
        User user = userService.createUser(userMac, deviceMac, request);
        return ResponseEntity.status(HttpStatus.CREATED).body(UserMapper.toUserResponse(user));
    }

    @Override
    public ResponseEntity<UserResponse> getUser(Long id) {
        User user = userService.findById(id)
                .orElseThrow(() -> new UserException(UserException.ExceptionType.NOT_FOUND, id));

        return ResponseEntity.status(HttpStatus.OK).body(UserMapper.toUserResponse(user));
    }

    @Override
    public ResponseEntity<List<DeviceResponse>> getUserDevices(Long id) {
        User user = userService.findById(id)
            .orElseThrow(() -> new UserException(UserException.ExceptionType.NOT_FOUND, id));

        return ResponseEntity.status(HttpStatus.OK).body(user.getDevices().stream().map(DeviceMapper::toDeviceResponse).toList());
    }
}
