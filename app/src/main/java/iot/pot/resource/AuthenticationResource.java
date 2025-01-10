package iot.pot.resource;

import iot.pot.authentication.AuthenticationService;
import iot.pot.database.model.User;
import iot.pot.endpoints.AuthenticationEndpoints;
import iot.pot.model.mapper.UserMapper;
import iot.pot.model.request.AuthenticationRequest;
import iot.pot.model.response.UserResponse;
import lombok.RequiredArgsConstructor;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/auth")
@RequiredArgsConstructor
public class AuthenticationResource implements AuthenticationEndpoints {
    private final AuthenticationService authenticationService;

    @Override
    public ResponseEntity<UserResponse> authenticate(AuthenticationRequest request) {
        User user = authenticationService.authenticate(request.getEmail(), request.getPassword());
        return ResponseEntity
                .status(HttpStatus.OK)
                .body(UserMapper.toUserResponse(user));
    }
}
