package iot.pot.endpoints;

import iot.pot.model.request.AuthenticationRequest;
import iot.pot.model.response.UserResponse;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/auth")
public interface AuthenticationEndpoints {
    @PostMapping("/authenticate")
    ResponseEntity<UserResponse> authenticate(@RequestBody AuthenticationRequest request);
}
