package iot.pot.database.model;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import jakarta.persistence.Entity;
import jakarta.persistence.Table;
import lombok.*;

@Entity(name = "temperature")
@Table(name = "temperature")
@Data
@Builder
@AllArgsConstructor
@EqualsAndHashCode(callSuper = true)
public class Temperature extends Measurement { }
