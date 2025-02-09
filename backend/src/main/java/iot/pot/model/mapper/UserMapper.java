package iot.pot.model.mapper;

import iot.pot.database.model.User;
import iot.pot.model.response.UserResponse;

public class UserMapper {

    public static UserResponse toUserResponse(User user) {
        return UserResponse.builder()
                .id(user.getId())
                .firstName(user.getFirstName())
                .lastName(user.getLastName())
                .email(user.getEmail())
                .phoneNumber(user.getPhoneNumber())
                .verified(user.getVerified())
                .role(user.getRole())
                .build();
    }
}
