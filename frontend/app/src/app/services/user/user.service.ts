import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { firstValueFrom, Observable } from 'rxjs';
import { Device } from '../../model/Device';
import { UserRequest } from '../../model/dto/UserRequest';

@Injectable({
  providedIn: 'root'
})
export class UserService {
  private apiUrl = 'http://localhost:8080/users'; 

  constructor(private http: HttpClient) {}

  getUserDevices(id: number): Observable<Device[]> {
    return this.http.get<Device[]>(`${this.apiUrl}/${id}/devices`);
  }

  createUser(user: UserRequest, userMac: string, deviceMac: string): Promise<any> {
    return firstValueFrom(this.http.post<number>(`${this.apiUrl}/${userMac}/devices/${deviceMac}`, user));
  }
}
