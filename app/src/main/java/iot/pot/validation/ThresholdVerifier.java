package iot.pot.validation;

import iot.pot.database.model.User;
import iot.pot.messaging.MeasurementNotificationHandler;
import iot.pot.model.enums.Measurement;
import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Service;

@Service
@RequiredArgsConstructor
public class ThresholdVerifier {
  private final MeasurementNotificationHandler notificationHandler;

  public void verifyThreshold(Measurement measurement, Double lowerThreshold, Double upperThreshold, Double value, User user) {
      if (lowerThreshold > value) {
          notificationHandler.sendMessage(
                  user, String.format(
                          measurement.getMeasurementDetails().getLowerThresholdMessageTemplate(),
                          lowerThreshold,
                          value
                  )
          );
      }

      if (upperThreshold < value) {
          notificationHandler.sendMessage(
                  user, String.format(
                          measurement.getMeasurementDetails().getUpperThresholdMessageTemplate(),
                          upperThreshold,
                          value
                  )
          );
      }
  }
}
