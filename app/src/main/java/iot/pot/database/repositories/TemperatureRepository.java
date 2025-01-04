package iot.pot.database.repositories;

import iot.pot.database.model.Device;
import iot.pot.database.model.Temperature;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

@Repository
public interface TemperatureRepository extends JpaRepository<Temperature, Long> {
    Page<Temperature> findByDevice(Device device, Pageable pageable);
}
