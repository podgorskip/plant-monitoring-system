import { Injectable, OnInit } from '@angular/core';
import { BehaviorSubject, catchError, map, Observable, of, tap, throwError } from 'rxjs';
import { User } from '../../../model/User';
import { HttpClient } from '@angular/common/http';
import { AuthException } from '../../../exceptions/AuthException';
import { AuthService } from '../Auth';

@Injectable({
  providedIn: 'root'
})
export class AuthenticationService {
  private apiUrl = 'http://localhost:8080/auth'; 

  constructor(private http: HttpClient, private authService: AuthService) { }

  authenticate(email: string, password: string): void {
    const body = { email, password };
    this.http.post<User>(`${this.apiUrl}/authenticate`, body).subscribe({
      next: (user) => {
        console.log('Authenticated: ', user);
        this.authService.setAuthenticatedUser(user); 
      }
    })
  }
}
