package iot.pot.database.model;

import jakarta.persistence.*;
import lombok.*;

@Entity(name = "air_humidity")
@Table(name = "air_humidity")
@Data
@Builder
@AllArgsConstructor
@EqualsAndHashCode(callSuper = true)
public class AirHumidity extends Measurement { }
