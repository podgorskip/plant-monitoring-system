package iot.pot.database.model;

import jakarta.persistence.Entity;
import jakarta.persistence.Table;
import lombok.*;

@Entity(name = "soil_humidity")
@Table(name = "soil_humidity")
@Data
@Builder
@AllArgsConstructor
@EqualsAndHashCode(callSuper = true)
public class SoilHumidity extends Measurement { }
