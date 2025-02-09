package iot.pot.database.repositories;

import iot.pot.database.model.Device;
import iot.pot.database.model.Insolation;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.repository.JpaRepository;

public interface InsolationRepository extends JpaRepository<Insolation, Long> {
    Page<Insolation> findByDevice(Device device, Pageable pageable);
}
