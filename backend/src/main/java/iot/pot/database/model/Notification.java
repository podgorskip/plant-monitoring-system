package iot.pot.database.model;

import iot.pot.model.enums.MeasurementEnum;
import jakarta.persistence.*;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;
import java.time.LocalDateTime;

@Entity(name = "notification")
@Table(name = "notification")
@Data
@Builder
@AllArgsConstructor
@NoArgsConstructor
public class Notification {
    @Id
    @SequenceGenerator(name = "water_request_id_seq", sequenceName = "water_request_id_seq")
    @GeneratedValue(strategy = GenerationType.SEQUENCE, generator = "water_request_id_seq")
    private Long id;

    @ManyToOne
    @JoinColumn(name = "device_id", nullable = false)
    private Device device;

    @Enumerated(value = EnumType.STRING)
    @Column(name = "type", nullable = false)
    private MeasurementEnum type;

    @Column(name = "min")
    private Boolean min;

    @Column(name = "date", nullable = false)
    @Builder.Default
    private LocalDateTime date = LocalDateTime.now();
}
