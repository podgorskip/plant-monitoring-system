import { Component, Input, OnInit } from '@angular/core';
import { AuthenticationService } from '../../auth/services/auth/authentication.service';
import { DeviceService } from '../../services/device/device.service';
import { User } from '../../model/User';
import { Device } from '../../model/Device';
import { UserService } from '../../services/user/user.service';
import { CommonModule, NgFor, NgIf } from '@angular/common';
import { DeviceComponent } from '../device/device.component';
import { AuthService } from '../../auth/services/Auth';

@Component({
  selector: 'app-devices',
  standalone: true,
  imports: [NgFor, DeviceComponent, NgIf, CommonModule],
  providers: [DeviceService, AuthenticationService, UserService],
  templateUrl: './devices.component.html',
  styleUrl: './devices.component.css'
})
export class DevicesComponent implements OnInit {
  authenticatedUser?: User;
  devices: Device[] = [];
  selectedDevice?: Device;

  constructor(
    private deviceService: DeviceService,
    private userService: UserService,
    private authenticationService: AuthService
  ) {}

  ngOnInit(): void {
    this.authenticationService.authenticatedUser$.subscribe({
      next: (user) => {
        if (user) { 
          this.authenticatedUser = user;
          console.log('Authenticated user in DevicesComponent:', user);
          this.fetchDevices(user.id);
        } else {
          console.log('No authenticated user found.');
        }
      },
      error: (err) => console.error('Error in subscription:', err),
    });
  }
  

  selectDevice(device: Device): void {
    this.selectedDevice = this.selectedDevice ? undefined : device;
  }

  private fetchDevices(id: number): void {
    this.authenticatedUser? this.userService.getUserDevices(id).subscribe({
      next: (devices) => {
        this.devices = devices
      },
      error: (err) => console.log(`Failed to fetch devices for user, error: ${err.message}`)
    }) : null;
  }
}
