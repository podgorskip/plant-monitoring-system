import { Injectable } from '@angular/core';
import { BehaviorSubject, catchError, map, Observable, tap, throwError } from 'rxjs';
import { User } from '../../../model/User';
import { HttpClient } from '@angular/common/http';
import { AuthException } from '../../../exceptions/AuthException';
import { AuthService } from '../Auth';

@Injectable({
  providedIn: 'root',
})
export class AuthenticationService {
  private apiUrl = 'http://localhost:8080/auth'; 

  constructor(private http: HttpClient, private authService: AuthService) {}

  authenticate(email: string, password: string): Observable<boolean> {
    const body = { email, password };

    return this.http.post<User>(`${this.apiUrl}/authenticate`, body).pipe(
      tap((user) => {
        console.log('Authenticated:', user);
        this.authService.setAuthenticatedUser(user); 
      }),
      map(() => true), 
      catchError((error) => {
        console.error('Authentication failed:', error);
        return throwError(() => new AuthException('Authentication failed'));
      })
    );
  }
}
