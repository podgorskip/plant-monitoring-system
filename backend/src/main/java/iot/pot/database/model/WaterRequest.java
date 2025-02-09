package iot.pot.database.model;

import jakarta.persistence.*;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;
import java.time.LocalDateTime;

@Entity(name = "water_request")
@Table(name = "water_request")
@Data
@Builder
@AllArgsConstructor
@NoArgsConstructor
public class WaterRequest {
    @Id
    @SequenceGenerator(name = "water_request_id_seq", sequenceName = "water_request_id_seq")
    @GeneratedValue(strategy = GenerationType.SEQUENCE, generator = "water_request_id_seq")
    private Long id;

    @ManyToOne
    @JoinColumn(name = "device_id", nullable = false)
    private Device device;

    @Column(name = "time", nullable = false)
    private Integer time;

    @Column(name = "date", nullable = false)
    @Builder.Default
    private LocalDateTime date = LocalDateTime.now();
}
