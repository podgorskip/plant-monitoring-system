package iot.pot.model.request;

import iot.pot.validation.annotations.EmailUnique;
import iot.pot.validation.annotations.Phone;
import jakarta.validation.constraints.Email;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.NotNull;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

@Data
@Builder
@AllArgsConstructor
@NoArgsConstructor
public class UserRequest {
    @NotNull @NotBlank
    private String firstName;

    @NotNull @NotBlank
    private String lastName;

    @NotNull @NotBlank @Email @EmailUnique
    private String email;

    @NotNull @NotBlank @Phone
    private String phoneNumber;

    @NotNull @NotBlank
    private String password;
}
