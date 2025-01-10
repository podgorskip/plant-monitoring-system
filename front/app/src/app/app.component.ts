import { Component, OnInit } from '@angular/core';
import { RouterOutlet } from '@angular/router';
import { NavbarComponent } from './components/navbar/navbar.component';
import { HttpClientModule } from '@angular/common/http';
import { AuthenticationService } from './auth/services/auth/authentication.service';
import { User } from './model/User';
import { AuthService } from './auth/services/Auth';

@Component({
  selector: 'app-root',
  standalone: true,
  imports: [RouterOutlet, NavbarComponent, HttpClientModule],
  providers: [AuthenticationService],
  templateUrl: './app.component.html',
  styleUrl: './app.component.css'
})
export class AppComponent implements OnInit {
  authenticatedUser?: User;

  constructor(private authenticationService: AuthService) { }
  
  ngOnInit(): void {
      this.authenticationService.authenticatedUser$.subscribe({
        next: (user) => {
          console.log(user)
          this.authenticatedUser = user;
        }
      })
  }
}
