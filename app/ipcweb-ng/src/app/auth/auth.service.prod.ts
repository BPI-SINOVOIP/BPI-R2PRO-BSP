import { Injectable } from '@angular/core';

import { Observable, of, BehaviorSubject } from 'rxjs';
import { tap, delay } from 'rxjs/operators';
import { Router } from '@angular/router';
import { CookieService } from 'ngx-cookie-service';
import { JwtHelperService } from '@auth0/angular-jwt';


@Injectable({
  providedIn: 'root'
})

export class AuthService {

  constructor(
    private router: Router,
    private cookie: CookieService,
    private jwt: JwtHelperService,
    ) { }

  /** TODO: Remove default loggedin that is only for debug */
  private loggedIn: BehaviorSubject<boolean> = new BehaviorSubject<boolean>(false);
  private authLevel: BehaviorSubject<number> = new BehaviorSubject<number>(NaN);

  redirectUrl: string;
  userInfo = {
    userName: '',
    auth: 2,
    isToken: true,
  };

  get AuthLevel() {
    return this.authLevel.asObservable();
  }

  get isLoggedIn() {
    const token = this.cookie.get('token');
    const ckName = localStorage.getItem('username');
    if (this.userInfo.isToken) {
      if (token) {
        const info = this.jwt.decodeToken(token);
        this.userInfo.auth = info['auth'];
        this.userInfo.userName = info['username'];
        this.authLevel.next(Number(this.userInfo.auth));
        this.loggedIn.next(true);
      } else {
        this.authLevel.next(NaN);
        this.loggedIn.next(false);
      }
    } else {
      if (ckName) {
        this.userInfo.userName = ckName;
        this.loggedIn.next(true);
      } else {
        this.loggedIn.next(false);
      }
    }
    return this.loggedIn.asObservable();
  }

  get loginStatus() {
    return this.loggedIn.asObservable();
  }

  logout(): void {
    this.loggedIn.next(false);
    const token = this.cookie.get('token');
    if (token) {
      this.cookie.delete('token');
    }
    this.router.navigate(['/login']);
  }

  redirect2OldUrl() {
    this.loggedIn.next(true);
    this.router.navigate([this.redirectUrl ? this.redirectUrl : '/']);
  }

  getUserName() {
    return this.userInfo.userName;
  }

}
