package iot.pot.authentication;

import iot.pot.database.model.User;
import iot.pot.exceptions.AuthenticationException;
import iot.pot.services.UserService;
import lombok.RequiredArgsConstructor;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.stereotype.Service;

@Service
@RequiredArgsConstructor
public class AuthenticationService {
    private final UserService userService;
//    private final PasswordEncoder passwordEncoder;

    public User authenticate(String email, String password) {
        User user = userService.findByEmail(email)
                .orElseThrow(() -> new AuthenticationException("Authentication failure. Incorrect email."));

//        if (!passwordEncoder.matches(password, user.getPassword())) {
//            throw new AuthenticationException("Authentication failure. Incorrect password.");
//        }

        if (!user.getPassword().equals(password)) {
            throw new AuthenticationException("Authentication failure. Incorrect password.");
        }

        return user;
    }
}
