package iot.pot.database.model;

import jakarta.persistence.Entity;
import jakarta.persistence.Table;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.EqualsAndHashCode;

@Entity(name = "insolation_digital")
@Table(name = "insolation_digital")
@Data
@Builder
@AllArgsConstructor
@EqualsAndHashCode(callSuper = true)
public class InsolationDigital extends Measurement {
}
