import { Component, Input, OnInit } from '@angular/core';
import { DeviceService } from '../../services/device/device.service';
import { Device } from '../../model/Device';
import { Threshold } from '../../model/Threshold';
import { MeasurementEnum } from './../../model/enums/MeasurementEnum';
import { NgFor, NgIf } from '@angular/common';
import { FormsModule } from '@angular/forms';

@Component({
  selector: 'app-device',
  standalone: true,
  imports: [NgFor, FormsModule, NgIf],
  providers: [DeviceService],
  templateUrl: './device.component.html',
  styleUrls: ['./device.component.css'],
})
export class DeviceComponent implements OnInit {
  @Input() device!: Device;
  thresholds: { measurement: MeasurementEnum; threshold: Threshold }[] = [];
  measurementEnumKeys = Object.keys(MeasurementEnum);
  selectedMeasurement?: MeasurementEnum;
  thresholdMin?: number;
  thresholdMax?: number;
  wateringTime!: number;
  selectedMeasurementTab!: string;
  paginatedMeasurements: { date: string; value: number }[] = [];
  currentPage = 0;

  constructor(private deviceService: DeviceService) {}

  ngOnInit(): void {
    this.loadThresholds();
  }

  private loadThresholds(): void {
    this.thresholds = [];
    this.measurementEnumKeys.forEach((key) => {
      const measurementKey = key as keyof typeof MeasurementEnum;
      this.deviceService.getDeviceThreshold(this.device.id, measurementKey).subscribe({
        next: (threshold_value) => {
          this.thresholds.push({
            measurement: MeasurementEnum[measurementKey],
            threshold: threshold_value,
          });
        },
        error: (err) =>
          console.error(`Failed to fetch threshold for ${key}: ${err}`),
      });
    });
  }
  
  addThreshold(): void {
    if (this.selectedMeasurement && this.thresholdMin != null && this.thresholdMax != null) {
      const request: Threshold = { lower: this.thresholdMin, upper: this.thresholdMax };
      this.deviceService
        .setDeviceThreshold(this.device.id, this.selectedMeasurement, request)
        .subscribe({
          next: () => {
            console.log('Threshold updated successfully.');
            this.resetForm();
            this.loadThresholds();
          },
          error: (err) =>
            console.error(
              `Failed to set threshold for ${this.selectedMeasurement}: ${err}`
            ),
        });
    }
  }

  sendWaterRequest(): void {
    if (this.wateringTime <= 0) {
      console.error('Watering time must be greater than 0.');
      return;
    }
    this.deviceService.startWatering(this.device.id, this.wateringTime).subscribe({
      next: () => {
        this.wateringTime = 0;
        alert('Watering started successfully.')
      },
      error: (err) => console.error('Failed to start watering:', err),
    });
  }

  getEnumValue(key: string): string | undefined {
    if (key in MeasurementEnum) {
      return MeasurementEnum[key as keyof typeof MeasurementEnum];
    }
    return undefined; 
  }
  

  fetchMeasurements(measurement: string, page: number = 0): void {
    this.selectedMeasurementTab = measurement;
    this.currentPage = page;
    this.deviceService.getDeviceMeasurements(this.device.id, measurement, page).subscribe({
      next: (measurements) => {
        this.paginatedMeasurements = measurements.content; 
        console.log('Measurements fetched successfully:', measurements);
      },
      error: (err) =>
        console.error(`Failed to fetch measurements for ${measurement}:`, err),
    });
  }

  private resetForm(): void {
    this.selectedMeasurement = undefined;
    this.thresholdMin = undefined;
    this.thresholdMax = undefined;
  }
}
