import { Component, Input } from '@angular/core';
import { RouterLink } from '@angular/router';
import { AuthenticationService } from '../../auth/services/auth/authentication.service';
import { User } from '../../model/User';
import { NgIf } from '@angular/common';

@Component({
  selector: 'app-navbar',
  standalone: true,
  imports: [RouterLink, NgIf],
  templateUrl: './navbar.component.html',
  styleUrl: './navbar.component.css'
})
export class NavbarComponent {
  @Input() authenticatedUser?: User;
}
