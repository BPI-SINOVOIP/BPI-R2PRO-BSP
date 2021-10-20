import { Injectable, OnDestroy } from '@angular/core';
import { BehaviorSubject, Subject } from 'rxjs';

import { AuthService } from 'src/app/auth/auth.service';
import { ConfigService } from 'src/app/config.service';
import { LayoutCell } from './layout-cell';
import { TipsService } from 'src/app/tips/tips.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import Logger from 'src/app/logger';

@Injectable({
  providedIn: 'root'
})
export class LayoutService implements OnDestroy {

  constructor(
    private resError: ResponseErrorService,
    private cfg: ConfigService,
    private auth: AuthService,
    private tips: TipsService,
  ) { }

  private logger: Logger = new Logger('layout-service');

  private headerHeight: Subject<number> = new Subject<number>();
  private sideHeight: BehaviorSubject<number> = new BehaviorSubject<number>(0);
  private finishSignal: BehaviorSubject<boolean> = new BehaviorSubject<boolean>(false);
  private headerLy: BehaviorSubject<string> = new BehaviorSubject<string>('');
  private playerResolution: BehaviorSubject<string> = new BehaviorSubject<string>('');
  private authOb: any;
  private layoutKey: string = 'layout';
  private isLayoutReady: boolean = false;
  private lastAuth: number = NaN;
  private loginOb: any;

  private layoutConfig: LayoutCell = {
    auth: 4,
    item: [
      {
        auth: 4,
        name: 'preview'
      },
      {
        auth: 4,
        name: 'config',
        item: [
          {
            auth: 4,
            name: 'System',
            item: [
              {
                auth: 4,
                name: 'Settings',
                item: [
                  {
                    auth: 4,
                    name: 'basic'
                  }
                ]
              },
            ]
          },
          {
            auth: 4,
            name: 'Network',
            item: [
              {
                auth: 4,
                name: 'Basic',
                item: [
                  {
                    auth: 4,
                    name: 'TCPIP'
                  },
                  {
                    auth: 4,
                    name: 'Port'
                  },
                ]
              },
            ]
          },
        ]
      },
      {
        auth: 4,
        name: 'about'
      },
    ],
    name: 'header',
  };
  private layoutDict: any = {
    header: {
      layout: ['preview', 'config', 'about'],
      about: {},
      config: {
        layout: ['System', 'Network'],
        System: {
          Settings: {
            layout: ['basic', ],
            basic: {}
          },
          layout: ['Settings', ]
        },
        Network: {
          layout: ['Basic', ],
          Basic: {
            layout: ['TCPIP', 'Port'],
            TCPIP: {},
            Port: {}
          }
        }
      },
      preview: {},
    }
  };

  browser: any = {
    height: NaN,
    width: NaN,
  };

  header: any = {
    height: NaN,
    width: NaN,
    setHeight: (setHeight: number) => {
      const oldHeight = document.getElementById('head-nav') ? document.getElementById('head-nav').clientHeight : NaN;
      this.header.height = setHeight;
      this.header.width = this.browser.width;
      if (oldHeight !== this.header.height) {
        this.headerHeight.next(this.header.height);
      }
    },
  };

  footer: any = {
    height: NaN,
    width: NaN,
  };

  sideBar: any = {
    height: NaN,
    width: NaN,
    windowHeight: 0,
    setHeight: () => {
      if (this.browser.height > this.sideBar.windowHeight) {
        this.sideBar.windowHeight = this.browser.height;
      }
      const oldHeight = this.sideBar.height;
      this.sideBar.height = this.sideBar.windowHeight - this.header.height - this.footer.height - 60;
      if (oldHeight !== this.sideBar.height) {
        this.sideHeight.next(this.sideBar.height);
      }
    }
  };

  get sideBarHeight() {
    return this.sideHeight.asObservable();
  }

  get sideBarHeightValue() {
    return this.sideBar.height;
  }

  get headHeight() {
    return this.headerHeight.asObservable();
  }

  get headWidth() {
    return this.header.width;
  }

  get HeaderLayout() {
    return this.headerLy.asObservable();
  }

  get FinishSignal() {
    return this.finishSignal.asObservable();
  }

  get PlayerResolution() {
    return this.playerResolution.asObservable();
  }

  ngOnDestroy() {
    if (this.loginOb) {
      this.loginOb.unsubscribe();
    }
  }

  getBrowserSize() {
    this.browser.height = Math.max(
      Math.min(window.innerHeight, window.outerHeight),
      Math.round(window.outerHeight * 0.8),
      Math.round(window.screen.availHeight * 0.8)
    );
    this.browser.width = Math.max(
      Math.min(window.innerWidth, window.outerWidth),
      Math.round(window.outerWidth * 0.8),
      Math.round(window.screen.availWidth * 0.8)
    );
    this.header.setHeight(Math.round(this.browser.height / 12.5));
    this.footer.height = document.getElementById('footer-component').offsetHeight;
    this.sideBar.setHeight();
  }

  getLayoutConfig() {
    this.cfg.getDefaultPara('webPage').subscribe(
      res => {
        this.resError.analyseRes(res);
        this.layoutConfig = JSON.parse(res.toString());
        this.authObserver();
      },
      err => {
        this.authObserver();
        this.logger.error(err, 'init fail in getLayoutConfig');
        this.tips.showInitFail();
      }
    );

    this.loginOb = this.auth.loginStatus.subscribe(
      (change: boolean) => {
        if (change) {
          this.updateResolution();
        }
      }
    );
  }

  updateResolution = () => {
    this.cfg.getVideoEncoderInterface('subStream').subscribe(
      res => {
        const resolution = res['sResolution'];
        if (resolution) {
          this.playerResolution.next(resolution);
          if (this.loginOb) {
            this.loginOb.unsubscribe();
            this.loginOb = null;
          }
        }
      },
      err => {
        this.logger.error(err, 'loginOb:getVideoEncoderInterface:');
      }
    );
  }

  authObserver() {
    if (this.authOb) {
      this.authOb.unsubscribe();
    }
    this.authOb = null;
    this.authOb = this.auth.AuthLevel.subscribe(
      (change: number) => {
        if (!isNaN(change) && change !== this.lastAuth) {
          this.lastAuth = change;
          this.isLayoutReady = false;
          this.finishSignal.next(false);
          this.calLayout(change);
        }
      }
    );
  }

  calLayout(authLv: number) {
    this.layoutDict = {};
    if (this.layoutConfig.auth < authLv) {
      return;
    }
    this.layoutDict[this.layoutConfig.name] = {};
    this.calLoop(authLv, this.layoutConfig, this.layoutDict[this.layoutConfig.name]);
    const headerStr = JSON.stringify(this.layoutDict[this.layoutConfig.name][this.layoutKey]);
    this.headerLy.next(headerStr);
    this.isLayoutReady = true;
    this.finishSignal.next(true);
  }

  calLoop(authLv: number, cycleObj: LayoutCell, rstObj: any) {
    rstObj[this.layoutKey] = [];
    for (const item of cycleObj.item) {
      if (item.auth >= authLv) {
        rstObj[this.layoutKey].push(item.name);
        rstObj[item.name] = {};
        if (item['item'] && item.item.length > 0) {
          this.calLoop(authLv, item, rstObj[item.name]);
        }
      }
    }
  }

  getSecondLayout(secondKey: string, isSideBar: boolean) {
    const secondLt: Subject<string> = new Subject<string>();
    this.waitLayoutTrue(5000)
      .then(
        () => {
          if (isSideBar) {
            const str = JSON.stringify(this.getSideBarLayout(this.layoutDict[this.layoutConfig.name][secondKey]));
            secondLt.next(str);
          } else {
            const str = JSON.stringify(this.layoutDict[this.layoutConfig.name][secondKey][this.layoutKey]);
            secondLt.next(str);
          }
        }
      )
      .catch();
    return secondLt;
  }

  getThirdLayout(secondKey: string, thirdKey: string) {
    const thirdLt: Subject<string> = new Subject<string>();
    this.waitLayoutTrue(5000)
      .then(
        () => {
          if (this.layoutDict[this.layoutConfig.name][secondKey]) {
            if (this.layoutDict[this.layoutConfig.name][secondKey][thirdKey]) {
              const str = JSON.stringify(this.layoutDict[this.layoutConfig.name][secondKey][thirdKey][this.layoutKey]);
              thirdLt.next(str);
            }
          }
        }
      )
      .catch();
    return thirdLt;
  }

  getfourthLayout(secondKey: string, thirdKey: string, fourthKey: string) {
    const fourthLt: Subject<string> = new Subject<string>();
    this.waitLayoutTrue(5000)
      .then(
        () => {
          const str = JSON.stringify(this.layoutDict[this.layoutConfig.name][secondKey][thirdKey][fourthKey][this.layoutKey]);
          fourthLt.next(str);
        }
      )
      .catch();
    return fourthLt;
  }

  getSecondLt(secondKey: string, isSideBar: boolean = false) {
    if (!this.getSideBarLayout(this.layoutDict[this.layoutConfig.name][secondKey])) {
      return [];
    }
    if (isSideBar) {
      return this.getSideBarLayout(this.layoutDict[this.layoutConfig.name][secondKey]);
    } else {
      return this.layoutDict[this.layoutConfig.name][secondKey][this.layoutKey];
    }
  }

  getThirdLt(secondKey: string, thirdKey: string) {
    if (this.layoutDict[this.layoutConfig.name][secondKey] &&
      this.layoutDict[this.layoutConfig.name][secondKey][thirdKey] &&
      this.layoutDict[this.layoutConfig.name][secondKey][thirdKey][this.layoutKey]) {
        return this.layoutDict[this.layoutConfig.name][secondKey][thirdKey][this.layoutKey];
    }
    return [];
  }

  getfourthLt(secondKey: string, thirdKey: string, fourthKey: string) {
    if (this.layoutDict[this.layoutConfig.name][secondKey] &&
      this.layoutDict[this.layoutConfig.name][secondKey][thirdKey] &&
      this.layoutDict[this.layoutConfig.name][secondKey][thirdKey][fourthKey] &&
      this.layoutDict[this.layoutConfig.name][secondKey][thirdKey][fourthKey][this.layoutKey]) {
        return this.layoutDict[this.layoutConfig.name][secondKey][thirdKey][fourthKey][this.layoutKey];
    }
    return [];
  }

  getSideBarLayout(obj: any) {
    const menu = obj[this.layoutKey];
    if (menu) {
      const list = [];
      for (const name of menu) {
        const one = {};
        one['name'] = name;
        one['items'] = [];
        for (const two of obj[name][this.layoutKey]) {
          one['items'].push(two);
        }
        list.push(one);
      }
      return list;
    }
    return [];
  }

  getFinishSignal() {
    const signal: Subject<boolean> = new Subject<boolean>();
    this.waitLayoutTrue(5000)
      .then(
        () => {
          signal.next(true);
        }
      )
      .catch();
    return signal;
  }

  waitLayoutTrue = (waitTime: number) => new Promise(
    (resolve, reject) => {
      const waitFunc = () => {
        waitTime -= 100;
        if (this.isLayoutReady) {
          resolve();
        } else if (waitTime < 0) {
          reject();
        } else {
          setTimeout(waitFunc, 100);
        }
      };
      waitFunc();
    }
  )
}
