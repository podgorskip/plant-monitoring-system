import { Routes } from '@angular/router';
import { DevicesComponent } from './components/devices/devices.component';
import { LoginComponent } from './components/login/login.component';

export const routes: Routes = [
    { path: 'devices', component: DevicesComponent },
    { path: 'login', component: LoginComponent }
];
