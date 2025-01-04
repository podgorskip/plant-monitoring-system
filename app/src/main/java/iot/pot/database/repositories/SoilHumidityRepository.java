package iot.pot.database.repositories;

import iot.pot.database.model.Device;
import iot.pot.database.model.SoilHumidity;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

@Repository
public interface SoilHumidityRepository extends JpaRepository<SoilHumidity, Long> {
    Page<SoilHumidity> findByDevice(Device device, Pageable pageable);
}
