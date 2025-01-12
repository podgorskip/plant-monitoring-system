package iot.pot.validation;

import iot.pot.database.model.Device;
import iot.pot.messaging.MeasurementNotificationHandler;
import iot.pot.model.enums.MeasurementEnum;
import iot.pot.services.NotificationService;
import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Service;
import java.util.Objects;

@Service
@RequiredArgsConstructor
public class ThresholdVerifier {
  private final MeasurementNotificationHandler notificationHandler;
  private final NotificationService notificationService;

  public void verifyThreshold(MeasurementEnum measurement, Double lowerThreshold, Double upperThreshold, Double value, Device device) {
      System.out.println("Verifying thresholds...");

      if (Objects.nonNull(lowerThreshold) && (lowerThreshold > value)) {
          notificationService.createNotification(device, measurement, true);
          notificationHandler.sendMessage(
                  device.getUser(), String.format(
                          measurement.getMeasurementDetails().getLowerThresholdMessageTemplate(),
                          lowerThreshold,
                          value
                  )
          );
      }

      if (Objects.nonNull(upperThreshold) && (upperThreshold < value)) {
          notificationService.createNotification(device, measurement, false);
          notificationHandler.sendMessage(
                  device.getUser(), String.format(
                          measurement.getMeasurementDetails().getUpperThresholdMessageTemplate(),
                          upperThreshold,
                          value
                  )
          );
      }
  }
}
