import { NgIf } from '@angular/common';
import { Component, OnInit } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { UserRequest } from '../../model/dto/UserRequest';
import { UserService } from '../../services/user/user.service';
import { ActivatedRoute, Router } from '@angular/router';

@Component({
  selector: 'app-registration',
  standalone: true,
  imports: [FormsModule, NgIf],
  providers: [UserService],
  templateUrl: './registration.component.html',
  styleUrls: ['./registration.component.css'],
})
export class RegistrationComponent implements OnInit {
  userMac!: string;
  deviceMac!: string;
  user: UserRequest = {
    firstName: '',
    lastName: '',
    email: '',
    phoneNumber: '',
    password: '',
  };

  constructor(private userService: UserService, private route: ActivatedRoute, private router: Router) { }

  ngOnInit(): void {
    const user: string | null = this.route.snapshot.paramMap.get('userMac');
    const device: string | null = this.route.snapshot.paramMap.get('deviceMac');

    if (user && device) {
      this.userMac = user;
      this.deviceMac = device;
    }

    new Error();
  }

  onSubmit(): void {
    this.userService.createUser(this.user, this.userMac, this.deviceMac)
      .then(() => {
        this.router.navigate(['/login']);
      })
  }
}
