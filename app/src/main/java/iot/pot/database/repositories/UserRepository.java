package iot.pot.database.repositories;

import iot.pot.database.model.User;
import iot.pot.database.model.enums.Role;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;
import java.util.List;
import java.util.Optional;

@Repository
public interface UserRepository extends JpaRepository<User, Long> {
    List<User> findByRole(Role role);

    Optional<User> findByEmail(String username);

    Optional<User> findByPhoneNumber(String phoneNumber);

    Optional<User> findByMac(String mac);
}
