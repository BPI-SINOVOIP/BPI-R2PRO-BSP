import { Component, OnInit, HostListener, AfterViewInit, ElementRef } from '@angular/core';
import { TranslateService } from '@ngx-translate/core';
import { AuthService } from 'src/app/auth/auth.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { LayoutService } from 'src/app/shared/func-service/layout.service';

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.scss']
})
export class AppComponent implements OnInit, AfterViewInit {

  constructor(
    public translate: TranslateService,
    private auth: AuthService,
    private pfs: PublicFuncService,
    private los: LayoutService,
    private el: ElementRef
  ) {}

  title = 'ipcweb-ng';
  langBank = [
    'zh-CN',
    'en-US'
  ];

  public async ngOnInit() {
    this.los.getLayoutConfig();
    this.translate.setDefaultLang('zh-CN');
    let currentLanguage = await localStorage.getItem('currentLanguage') || this.translate.getBrowserCultureLang();
    if (!this.pfs.isInArrayString(this.langBank, currentLanguage)) {
      currentLanguage = 'zh-CN';
    }
    this.translate.use(currentLanguage);
    localStorage.setItem('currentLanguage', currentLanguage);
  }

  ngAfterViewInit() {
    this.los.getBrowserSize();
  }

  public selectLanguage(lang) {
    if (this.pfs.isInArrayString(this.langBank, lang)) {
      this.translate.use(lang);
      localStorage.setItem('currentLanguage', lang);
    }
  }

  @HostListener('window:resize', ['$event'])
  onResize(event) {
    this.los.getBrowserSize();
  }
}
