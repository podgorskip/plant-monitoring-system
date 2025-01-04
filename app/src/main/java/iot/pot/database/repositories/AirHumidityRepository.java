package iot.pot.database.repositories;

import iot.pot.database.model.AirHumidity;
import iot.pot.database.model.Device;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.repository.JpaRepository;

public interface AirHumidityRepository extends JpaRepository<AirHumidity, Long> {
    Page<AirHumidity> findByDevice(Device device, Pageable pageable);
}
