package iot.pot.model.mapper;

import iot.pot.database.model.Device;
import iot.pot.model.response.DeviceResponse;

public class DeviceMapper {

    public static DeviceResponse toDeviceResponse(Device device) {
        return DeviceResponse.builder()
                .id(device.getId())
                .deviceNumber(device.getMac())
                .creationDate(device.getCreationDate())
                .user(UserMapper.toUserResponse(device.getUser()))
                .build();
    }
}
