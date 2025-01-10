import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { Observable } from 'rxjs';
import { Device } from '../../model/Device';

@Injectable({
  providedIn: 'root'
})
export class UserService {
  private apiUrl = 'http://localhost:8080/users'; 

  constructor(private http: HttpClient) {}

  getUserDevices(id: number): Observable<Device[]> {
    return this.http.get<Device[]>(`${this.apiUrl}/${id}/devices`);
  }
}
