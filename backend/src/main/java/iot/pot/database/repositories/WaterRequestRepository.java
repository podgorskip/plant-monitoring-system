package iot.pot.database.repositories;

import iot.pot.database.model.WaterRequest;
import org.springframework.data.jpa.repository.JpaRepository;

public interface WaterRequestRepository extends JpaRepository<WaterRequest, Long> {
}
