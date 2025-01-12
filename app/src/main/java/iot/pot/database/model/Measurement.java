package iot.pot.database.model;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import iot.pot.utils.UnixTimestampDeserializer;
import jakarta.persistence.*;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;
import java.time.LocalDateTime;

@Entity
@Inheritance(strategy = InheritanceType.TABLE_PER_CLASS)
@Data
@AllArgsConstructor
@JsonIgnoreProperties(ignoreUnknown = true)
@NoArgsConstructor
public abstract class Measurement {
    @Id
    @GeneratedValue(strategy = GenerationType.SEQUENCE, generator = "measurement_id_seq")
    @SequenceGenerator(name = "measurement_id_seq", sequenceName = "measurement_id_seq")
    private Long id;

    @Column(name = "value", nullable = false)
    private Double value;

    @ManyToOne
    @JoinColumn(name = "device_id")
    private Device device;

    private String unit;

    @Column(name = "date", nullable = false)
    @JsonDeserialize(using = UnixTimestampDeserializer.class)
    private LocalDateTime date = LocalDateTime.now();
}