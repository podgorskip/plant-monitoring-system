package iot.pot.services;

import iot.pot.database.model.Device;
import iot.pot.database.model.Notification;
import iot.pot.database.repositories.NotificationRepository;
import iot.pot.model.enums.MeasurementEnum;
import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Service;

@Service
@RequiredArgsConstructor
public class NotificationService {
    private NotificationRepository notificationRepository;

    public void createNotification(Device device, MeasurementEnum measurement, Boolean min) {
        Notification notification = Notification.builder()
                .type(measurement)
                .device(device)
                .min(min)
                .build();

        notificationRepository.save(notification);
    }
}
