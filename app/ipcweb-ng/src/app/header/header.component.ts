import { Component, OnInit, OnDestroy, Renderer2, ElementRef } from '@angular/core';
import { AuthService } from '../auth/auth.service';
import { Observable } from 'rxjs';
import { TranslateService } from '@ngx-translate/core';
import { TipsService } from 'src/app/tips/tips.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { LayoutService } from 'src/app/shared/func-service/layout.service';

@Component({
  selector: 'app-header',
  templateUrl: './header.component.html',
  styleUrls: ['./header.component.scss']
})
export class HeaderComponent implements OnInit, OnDestroy {

  isLoggedIn$: Observable<boolean>;

  constructor(
    private authService: AuthService,
    public translate: TranslateService,
    private tips: TipsService,
    private pfs: PublicFuncService,
    private los: LayoutService,
    private Re2: Renderer2,
    private el: ElementRef,
  ) { }

  private nowHead: string = null;
  private loginStatus: any;
  private heightChange: any;
  private losOb: any;
  private height: number = NaN;
  userName: string = '用户名';
  currentLang: string = 'zh-CN';
  switchTip = {
    'zh-CN': 'toEnglish',
    'en-US': 'toChinese',
  };
  langBank = [
    'zh-CN',
    'en-US'
  ];

  headerNav = [
    'preview',
    'download',
    'face',
    'face-para',
    'face-manage',
    'config',
    'about'
  ];

  headerNavDefault = [
    'preview',
    'download',
    'face',
    'face-para',
    'face-manage',
    'config',
    'about'
  ];

  ngOnInit() {
    this.currentLang = localStorage.getItem('currentLanguage');
    if (!this.pfs.isInArrayString(this.langBank, this.currentLang)) {
      this.currentLang = 'zh-CN';
    }
    this.isLoggedIn$ = this.authService.isLoggedIn;
    this.loginStatus = this.isLoggedIn$.subscribe(change => {
      if (change) {
        this.userName = this.authService.getUserName();
      }
    });
    this.heightChange = this.los.headHeight.subscribe((change: number) => {
      if (!isNaN(change) && change !== 0) {
        this.height = change;
        this.setHeadSize(change);
      }
    });
    this.losOb = this.los.HeaderLayout.subscribe(
      (change: string) => {
        if (change !== '') {
          this.headerNav = JSON.parse(change);
          if (!isNaN(this.height)) {
            this.waitCNT(5000, this.headerNav.length, this.el)
              .then(
                () => {
                  this.setHeadSize(this.height);
                }
              )
              .catch();
          }
        }
      }
    );
  }

  ngOnDestroy() {
    if (this.loginStatus) {
      this.loginStatus.unsubscribe();
    }
    if (this.heightChange) {
      this.heightChange.unsubscribe();
    }
    if (this.losOb) {
      this.losOb.unsubscribe();
    }
  }

  onLogout() {
    this.authService.logout();
  }

  onSwitchLanguage() {
    if (this.currentLang === 'zh-CN') {
      this.currentLang = 'en-US';
    } else {
      this.currentLang = 'zh-CN';
    }
    this.translate.use(this.currentLang);
    localStorage.setItem('currentLanguage', this.currentLang);
  }

  onHeadChange(change: string) {
    if (change !== this.nowHead) {
      this.nowHead = change;
      this.tips.setRbTip('', -1);
    }
  }

  setHeadSize(height: number) {
    this.waitCNTExist(5000, this.el)
      .then(
        () => {
          this.setHeadSizeFunc(height);
        }
      )
      .catch();
  }

  setHeadSizeFunc(height: number) {
    let linkHeight = Math.round(height * 0.33);
    let bandHeight = Math.round(height * 0.5);
    const navList = this.el.nativeElement.querySelectorAll('.nav-link');
    const len = ((navList.length + 2) * 3 + 2) * linkHeight;
    const width = this.los.headWidth;
    let margin = 0;
    if (len > width * 0.65) {
      linkHeight = Math.round(width * 0.7 / ((navList.length + 2) * 3 + 2));
      bandHeight = Math.round(1.5 * linkHeight);
    } else {
      margin = Math.max(Math.round((width - len) * 0.5 / (navList.length + 3)), margin);
    }
    for (const item of navList) {
      this.Re2.setStyle(item, 'font-size', linkHeight + 'px');
      this.Re2.setStyle(item, 'margin-left', margin + 'px');
    }
    this.Re2.setStyle(this.el.nativeElement.querySelector('.navbar-brand'), 'font-size', bandHeight + 'px');
    this.Re2.setStyle(this.el.nativeElement.querySelector('.user-icon'), 'width', linkHeight + 'px');
    this.Re2.setStyle(this.el.nativeElement.querySelector('.user-icon'), 'height', linkHeight + 'px');
    this.Re2.setStyle(this.el.nativeElement.querySelector('.user-icon'), 'background-size', linkHeight + 'px');
    this.Re2.setStyle(this.el.nativeElement.querySelector('.exit-icon'), 'width', linkHeight + 'px');
    this.Re2.setStyle(this.el.nativeElement.querySelector('.exit-icon'), 'height', linkHeight + 'px');
    this.Re2.setStyle(this.el.nativeElement.querySelector('.exit-icon'), 'background-size', linkHeight + 'px');
  }

  waitCNT = (timeout: number, waitNum: number, el: ElementRef) => new Promise(
    (resolve, reject) => {
      const waitFunc = () => {
        timeout -= 100;
        const cnt = el.nativeElement.querySelectorAll('.cnt');
        if (cnt.length >= waitNum) {
          resolve();
        } else if (timeout < 0) {
          reject();
        } else {
          setTimeout(waitFunc, 100);
        }
      };
      waitFunc();
    }
  )

  waitCNTExist = (timeout: number, el: ElementRef) => new Promise(
    (resolve, reject) => {
      const waitFunc = () => {
        timeout -= 100;
        const cnt = el.nativeElement.querySelectorAll('.cnt');
        if (cnt.length >= 1) {
          resolve();
        } else if (timeout < 0) {
          reject();
        } else {
          setTimeout(waitFunc, 100);
        }
      };
      waitFunc();
    }
  )
}
