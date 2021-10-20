import { Component, OnInit, AfterViewInit, OnDestroy, ElementRef, Renderer2 } from '@angular/core';
import { AuthService } from '../auth.service';
import { FormGroup, FormBuilder, Validators, FormControl } from '@angular/forms';
import { Observable } from 'rxjs';
import { User } from '../user';
import { ConfigService } from 'src/app/config.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { TipsService } from 'src/app/tips/tips.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { LayoutService } from 'src/app/shared/func-service/layout.service';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-login',
  templateUrl: './login.component.html',
  styleUrls: ['./login.component.scss']
})
export class LoginComponent implements OnInit, AfterViewInit, OnDestroy {

  constructor(
    private fb: FormBuilder,
    private authService: AuthService,
    private cfg: ConfigService,
    private resError: ResponseErrorService,
    private tips: TipsService,
    private el: ElementRef,
    private Re2: Renderer2,
    private pfs: PublicFuncService,
    private los: LayoutService,
  ) { }

  private logger: Logger = new Logger('login');
  objectKey = this.pfs.objectKeys;
  private formSubmitAttempt: boolean;
  private subObserver: any;
  loginObserver$: Observable<number>;
  loginStatus: number = 0;
  private logining: boolean = false;
  form = this.fb.group({
    userName: [ '', Validators.required],
    password: [ '', Validators.required],
    expire: ['']
  });

  get userName(): FormControl {
    return this.form.get('userName') as FormControl;
  }

  get password(): FormControl {
    return this.form.get('password') as FormControl;
  }

  expireDict = {
    temporaryLogin: '',
    autoLoginDay: '?expire=day',
    autoLoginWeek: '?expire=week',
    autoLoginMonth: '?expire=month',
  };

  ngOnInit() {
    if (localStorage.getItem('defaultExpire')) {
      const exp = this.expireDict[localStorage.getItem('defaultExpire')];
      this.form.get('expire').setValue(exp);
    }
    if (localStorage.getItem('username')) {
      this.form.get('userName').setValue(localStorage.getItem('username'));
    }
  }

  ngAfterViewInit() {
    this.adjustHeight();
  }

  ngOnDestroy() {
  }

  adjustHeight() {
    let headHeight = 0;
    if (document.getElementById('head-nav')) {
      headHeight = document.getElementById('head-nav').clientHeight;
    }
    let footHeight = 0;
    if (document.getElementById('footer-component')) {
      footHeight = document.getElementById('footer-component').offsetHeight;
    }
    const cardHeight = document.getElementById('login-card').offsetHeight;
    const bodyHeight = window.innerHeight;
    const setHeight = bodyHeight - headHeight - footHeight - 50;
    const minHeight = Math.ceil(bodyHeight * 0.3 + cardHeight);
    const height = setHeight > minHeight ? setHeight : minHeight;
    this.Re2.setStyle(this.el.nativeElement.querySelector('.container'), 'height', height + 'px');
  }

  onSubmit() {
    let exp = '';
    for (const key of this.objectKey(this.expireDict)) {
      if (this.expireDict[key] === this.form.value.expire) {
        exp = key;
        break;
      }
    }
    // this.authService.login(this.form.value, exp);
    this.login(this.form.value, exp);
    this.formSubmitAttempt = true;
  }

  login(user: User, expire: string = '') {
    if (this.logining) {
      return;
    }
    this.logining = true;
    const loginInfo = {
      sUserName: user.userName,
      sPassword: btoa(user.password),
    };
    this.cfg.putLoginInfo(loginInfo, user.expire).subscribe(
      res => {
        this.loginStatus = res.status;
        if (res.status >= 0) {
          localStorage.setItem('username', user.userName);
          localStorage.setItem('defaultExpire', expire);
          localStorage.setItem('token', res.token);
          // this.cookie.set('token', res.token);
          // this.los.getBrowserSize();
          this.authService.redirect2OldUrl();
        }
        this.logining = false;
      },
      err => {
        this.logger.error(err);
        this.loginStatus = -3;
        this.logining = false;
      }
    );
  }

}
