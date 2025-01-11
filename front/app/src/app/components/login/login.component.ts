import { CommonModule } from '@angular/common';
import { Component } from '@angular/core';
import { FormsModule, NgForm } from '@angular/forms';
import { AuthenticationService } from '../../auth/services/auth/authentication.service';
import { Router } from '@angular/router';

@Component({
  selector: 'app-login',
  standalone: true,
  imports: [FormsModule, CommonModule],
  providers: [AuthenticationService],
  templateUrl: './login.component.html',
  styleUrls: ['./login.component.css']
})
export class LoginComponent {
  email?: string = 'podorskip@interi.eu';
  password?: string = 'pass';
  errorMessage?: string;

  constructor(
    private authenticationService: AuthenticationService,
    private router: Router
  ) {}

  onSubmit(form: NgForm): void {
    if (form.valid && this.email && this.password) {
      this.authenticationService.authenticate(this.email, this.password).subscribe({
        next: (authenticated) => {
          console.log(authenticated);
          if (authenticated) this.router.navigate(['/devices']);
        }
      })
    }
  }
}
