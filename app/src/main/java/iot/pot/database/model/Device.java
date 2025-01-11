package iot.pot.database.model;

import jakarta.persistence.*;
import lombok.*;
import java.time.LocalDateTime;
import java.util.Set;

@Entity(name = "device")
@Table(name = "device")
@Data
@Builder
@AllArgsConstructor
public class Device {
    @Id
    @SequenceGenerator(name = "device_id_seq", sequenceName = "device_id_seq")
    @GeneratedValue(strategy = GenerationType.SEQUENCE, generator = "device_id_seq")
    private Long id;

    @Column(name = "mac", unique = true)
    private String mac;

    @Column(name = "name")
    @Builder.Default
    private String name = "Smart pot";

    @Column(name = "info")
    @Builder.Default
    private String info = "ESP32";

    @ManyToOne
    @JoinColumn(name = "user_id")
    private User user;

    @Column(name = "air_humidity_lower_threshold")
    private Double airHumidityLowerThreshold;

    @Column(name = "air_humidity_upper_threshold")
    private Double airHumidityUpperThreshold;

    @Column(name = "soil_humidity_lower_threshold")
    private Double soilHumidityLowerThreshold;

    @Column(name = "soil_humidity_upper_threshold")
    private Double soilHumidityUpperThreshold;

    @Column(name = "temperature_lower_threshold")
    private Double temperatureLowerThreshold;

    @Column(name = "temperature_upper_threshold")
    private Double temperatureUpperThreshold;

    @Column(name = "insolation_lower_threshold")
    private Double insolationLowerThreshold;

    @Column(name = "insolation_upper_threshold")
    private Double insolationUpperThreshold;

    @Column(name = "creation_date", nullable = false)
    @Builder.Default
    private LocalDateTime creationDate = LocalDateTime.now();

    @OneToMany(mappedBy = "device")
    @EqualsAndHashCode.Exclude
    @ToString.Exclude
    private Set<AirHumidity> airHumiditySet;

    @OneToMany(mappedBy = "device")
    @EqualsAndHashCode.Exclude
    @ToString.Exclude
    private Set<SoilHumidity> soilHumiditySet;

    @OneToMany(mappedBy = "device")
    @EqualsAndHashCode.Exclude
    @ToString.Exclude
    private Set<Temperature> temperatureSet;

    @OneToMany(mappedBy = "device")
    @EqualsAndHashCode.Exclude
    @ToString.Exclude
    private Set<Insolation> insolationSet;

    public Device() {

    }
}
