import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { Observable } from 'rxjs';
import { MeasurementEnum } from '../../model/enums/MeasurementEnum';
import { Device } from '../../model/Device';
import { Threshold } from '../../model/Threshold';

@Injectable({
  providedIn: 'root'
})
export class DeviceService {
  private apiUrl = 'http://localhost:8080/devices'; 

  constructor(private http: HttpClient) {}

  getDevices(): Observable<Device[]> {
    return this.http.get<Device[]>(this.apiUrl);
  }

  getDeviceById(id: number): Observable<Device> {
    return this.http.get<Device>(`${this.apiUrl}/${id}`);
  }

  getDeviceThreshold(id: number, measurement: keyof typeof MeasurementEnum): Observable<any> {
    return this.http.get(`${this.apiUrl}/${id}/thresholds?measurement=${measurement}`, {responseType: 'text'});
  }

  setDeviceThreshold(id: number, measurement: MeasurementEnum, threshold: Threshold): Observable<void> {
    return this.http.patch<void>(`${this.apiUrl}/${id}/thresholds`, threshold, {
      params: { measurement },
    });
  }

  deleteDevice(id: string) {
    return this.http.delete(`${this.apiUrl}/${id}`);
  }

  startWatering(deviceId: number, time: number): Observable<void> {
    return this.http.post<void>(`${this.apiUrl}/${deviceId}/watering?time=${time}`, {});
  }

  getDeviceMeasurements(
    deviceId: number,
    measurement: string,
    page: number = 0,
    size: number = 10,
    sortBy: string = 'date',
    sortDir: string = 'DESC'
  ): Observable<{ content: { date: string; value: number }[] }> {
    return this.http.get<{ content: { date: string; value: number }[] }>(
      `${this.apiUrl}/${deviceId}/measurements?measurement=${measurement}&page=${page}&size=${size}&sortBy=${sortBy}&sortDir=${sortDir}`
    );
  }
  
}
