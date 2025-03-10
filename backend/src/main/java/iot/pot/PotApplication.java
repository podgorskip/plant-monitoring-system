package iot.pot;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.web.bind.annotation.CrossOrigin;

@SpringBootApplication(scanBasePackages = "iot.pot")
public class PotApplication {
    public static void main(String[] args) {
        SpringApplication.run(PotApplication.class, args);
    }
}
