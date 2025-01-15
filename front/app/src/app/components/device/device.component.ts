import { Component, Input, OnInit } from '@angular/core';
import { DeviceService } from '../../services/device/device.service';
import { Device } from '../../model/Device';
import { Threshold } from '../../model/Threshold';
import { MeasurementEnum } from './../../model/enums/MeasurementEnum';
import { NgFor, NgIf } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { Frequency } from '../../model/Frequency';

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
  frequencies: { measurement: MeasurementEnum; frequency: Frequency }[] = []; 
  measurementEnumKeys = Object.keys(MeasurementEnum);
  measurementEnumKeysFiltered = Object.keys(MeasurementEnum).filter(
    (key) => key !== 'INSOLATION_DIGITAL'
  );
  selectedMeasurement?: MeasurementEnum;
  thresholdMin?: number;
  thresholdMax?: number;
  wateringTime!: number;
  frequencyValue?: number;
  selectedMeasurementTab!: string;
  paginatedMeasurements: { date: string; value: number }[] = [];
  iconPath?: string;
  currentPage = 0;

  constructor(private deviceService: DeviceService) {}

  ngOnInit(): void {
    this.loadThresholds();
    this.deviceService.getDeviceMeasurements(this.device.id, 'INSOLATION_DIGITAL').subscribe({
      next: (data: { content: { date: string; value: number }[] }) => {
        if (data.content && data.content.length > 0) {
          this.iconPath = data.content[0].value === 0 ? 'assets/sun.png' : 'assets/moon.png'; 
        } else {
          console.log('No content available.');
        }
      },
      error: (err) => {
        console.error(`Failed to fetch device measurements: ${err}`);
      }
    });
  }

  loadThresholds(): void {
    this.thresholds = [];
    this.measurementEnumKeysFiltered.forEach((key) => {
      const measurementKey = key as keyof typeof MeasurementEnum;
      this.deviceService.getDeviceThreshold(this.device.id, measurementKey).subscribe({
        next: (threshold_value) => {
          const parsedThreshold = JSON.parse(threshold_value);
            this.thresholds.push({
            measurement: MeasurementEnum[measurementKey],
            threshold: parsedThreshold,
          });
        },
        error: (err) =>
          console.error(`Failed to fetch threshold for ${key}: ${err}`),
      });
    });
  }
  

  private loadFrequencies(): void {
    this.frequencies = [];
    this.measurementEnumKeys.forEach((key) => {
      const measurementKey = key as keyof typeof MeasurementEnum;
      this.deviceService.getDeviceFrequency(this.device.id, measurementKey).subscribe({
        next: (frequency_value) => {
          this.frequencies.push({
            measurement: MeasurementEnum[measurementKey],
            frequency: frequency_value,
          });
        },
        error: (err) =>
          console.error(`Failed to fetch frequency for ${key}: ${err}`),
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
            this.resetForm();
            this.loadThresholds();
            alert('Threshold updated successfully.');
          },
          error: (err) =>
            console.error(
              `Failed to set threshold for ${this.selectedMeasurement}: ${err}`
            ),
        });
    }
  }

  addFrequency(): void {
    if (this.selectedMeasurement && this.frequencyValue != null) {
      const request: Frequency = { frequency: this.frequencyValue };
      this.deviceService
        .setDeviceFrequency(this.device.id, this.selectedMeasurement, request)
        .subscribe({
          next: () => {
            this.resetForm();
            this.loadFrequencies();
            alert('Frequency updated successfully.')
          },
          error: (err) =>
            console.error(
              `Failed to set frequency for ${this.selectedMeasurement}: ${err}`
            ),
        });
    }
  }

  formatPrettyDate(dateInput: string | Date | number[]): string {
    console.log(dateInput);

    if (Array.isArray(dateInput)) {
        const [year, month, day, hour = 0, minute = 0, second = 0] = dateInput;
        dateInput = new Date(year, month - 1, day, hour, minute, second);
    }

    const date = typeof dateInput === 'string' || dateInput instanceof Date
        ? new Date(dateInput)
        : null;

    if (!date || isNaN(date.getTime())) {
        console.error("Invalid input: dateInput is not a valid date or format", dateInput);
        return "Invalid Date";
    }

    return date.toLocaleDateString('en-US', {
        weekday: 'long',
        year: 'numeric',
        month: 'long',
        day: 'numeric',
        hour: 'numeric',
        minute: 'numeric',
    });
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
    this.frequencyValue = undefined;
  }
}
