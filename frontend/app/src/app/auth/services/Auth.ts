import { Injectable } from '@angular/core';
import { BehaviorSubject, Observable } from 'rxjs';
import { User } from '../../model/User';

@Injectable({ providedIn: 'root' })
export class AuthService {
  private _authenticatedUserSubject = new BehaviorSubject<User | undefined>(undefined);
  public authenticatedUser$: Observable<User | undefined> = this._authenticatedUserSubject.asObservable();

  setAuthenticatedUser(user: User | undefined) {
    this._authenticatedUserSubject.next(user);
  }
}