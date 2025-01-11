import { Routes } from '@angular/router';
import { DevicesComponent } from './components/devices/devices.component';
import { LoginComponent } from './components/login/login.component';
import { RegistrationComponent } from './components/registration/registration.component';

export const routes: Routes = [
    { path: 'devices', component: DevicesComponent },
    { path: 'login', component: LoginComponent },
    { path: 'users/:userMac/devices/:deviceMac', component: RegistrationComponent }
];
