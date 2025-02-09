package iot.pot.services;

import iot.pot.database.model.Device;
import iot.pot.database.model.WaterRequest;
import iot.pot.database.repositories.WaterRequestRepository;
import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Service;

@Service
@RequiredArgsConstructor
public class WaterRequestService {
    private final WaterRequestRepository waterRequestRepository;

    public WaterRequest createWaterRequest(Device device, Integer time) {
        WaterRequest waterRequest = WaterRequest.builder()
                .time(time)
                .device(device)
                .build();
        return waterRequestRepository.save(waterRequest);
    }
}
