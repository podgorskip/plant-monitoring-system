package iot.pot.database.model;

import jakarta.persistence.Entity;
import jakarta.persistence.Table;
import lombok.*;

@Entity(name = "insolation")
@Table(name = "insolation")
@Data
@Builder
@AllArgsConstructor
@EqualsAndHashCode(callSuper = true)
public class Insolation extends Measurement { }
