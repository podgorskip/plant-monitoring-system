package iot.pot.validation;

import iot.pot.validation.annotations.Phone;
import jakarta.validation.ConstraintValidator;
import jakarta.validation.ConstraintValidatorContext;

public class PhoneValidator implements ConstraintValidator<Phone, String> {
    @Override
    public boolean isValid(String phone, ConstraintValidatorContext context) {
        if (phone == null) {
            return true;
        }

        String phoneRegex = "^\\+?[0-9]{9,15}$";
        return phone.matches(phoneRegex);
    }
}
