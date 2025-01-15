package iot.pot.database.repositories;

import iot.pot.database.model.Device;
import iot.pot.database.model.InsolationDigital;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

@Repository
public interface InsolationDigitalRepository extends JpaRepository<InsolationDigital, Long> {
    Page<InsolationDigital> findByDevice(Device device, Pageable pageable);
}
