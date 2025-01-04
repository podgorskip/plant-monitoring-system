package iot.pot.database;

import iot.pot.database.repositories.UserRepository;
import iot.pot.database.model.User;
import iot.pot.database.model.enums.Role;
import jakarta.annotation.PostConstruct;
import jakarta.transaction.Transactional;
import lombok.RequiredArgsConstructor;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.stereotype.Service;
import java.util.List;

@Service
@RequiredArgsConstructor
public class InitializationService {
    private final UserRepository userRepository;
    private final PasswordEncoder passwordEncoder;

    @PostConstruct
    @Transactional
    public void init() {
        List<User> user = userRepository.findByRole(Role.ADMIN);

        if (user.isEmpty()) {
            User admin = User.builder()
                    .mac("12:34:56:78:90:AB")
                    .firstName("Patryk")
                    .lastName("Podgórski")
                    .email("podgorski.p@interia.eu")
                    .phoneNumber("571799037")
                    .password(passwordEncoder.encode("123456"))
                    .verified(true)
                    .role(Role.ADMIN)
                    .build();

            userRepository.save(admin);
        }
    }
}
