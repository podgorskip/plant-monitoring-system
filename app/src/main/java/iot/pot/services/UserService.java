package iot.pot.services;

import iot.pot.database.model.Device;
import iot.pot.database.model.User;
import iot.pot.database.model.enums.Role;
import iot.pot.database.repositories.UserRepository;
import iot.pot.exceptions.UserException;
import iot.pot.model.request.UserRequest;
import lombok.RequiredArgsConstructor;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Isolation;
import org.springframework.transaction.annotation.Transactional;

import java.util.Optional;

@Service
@RequiredArgsConstructor
public class UserService {
    private final UserRepository userRepository;
    private final DeviceService deviceService;
    private final PasswordEncoder passwordEncoder;

    @Transactional
    public User createUser(String userMac, String deviceMac, UserRequest request) {
        Optional<User> user = userRepository.findByMac(userMac);

        if (user.isPresent()) {
            throw new UserException(UserException.ExceptionType.CONFLICT, userMac);
        }

        User newUser = User.builder()
                .mac(userMac)
                .firstName(request.getFirstName())
                .lastName(request.getLastName())
                .email(request.getEmail())
                .phoneNumber(request.getPhoneNumber())
                .password(passwordEncoder.encode(request.getPassword()))
                .verified(false)
                .role(Role.CUSTOMER)
                .build();

        userRepository.saveAndFlush(newUser);
        deviceService.assignDevice(newUser, deviceMac);
        return newUser;
    }

    public Optional<User> findByEmail(String email) {
        return userRepository.findByEmail(email);
    }

    public Optional<User> findById(Long id) {
        return userRepository.findById(id);
    }
}
