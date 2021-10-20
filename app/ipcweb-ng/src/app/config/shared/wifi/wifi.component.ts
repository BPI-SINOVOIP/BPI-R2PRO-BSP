import { Component, OnInit, OnDestroy } from '@angular/core';
import { FormBuilder, FormGroup, Validators, FormControl } from '@angular/forms';
import { Subject } from 'rxjs';

import { ConfigService } from 'src/app/config.service';
import { WifiInfo, WifiItemInterface } from './WifiInfo';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { TipsService } from 'src/app/tips/tips.service';
import { LockService } from 'src/app/shared/func-service/lock-service.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-wifi',
  templateUrl: './wifi.component.html',
  styleUrls: ['./wifi.component.scss']
})
export class WifiComponent implements OnInit, OnDestroy {

  constructor(
    private fb: FormBuilder,
    private cfg: ConfigService,
    private ieCss: IeCssService,
    private resError: ResponseErrorService,
    private tips: TipsService,
    private pfs: PublicFuncService,
  ) { }

  private logger: Logger = new Logger('wifi');

  private scanSub: Subject<number> = new Subject<number>();
  lock = new LockService(this.tips);
  private ctOb: any;
  private wifiScanOb: any;

  isChrome: boolean = false;
  securityModes: string[] = [
    'None',
    'WEP',
    'WPA-Personal',
    'WPA-Enterprise',
    'WPA2-Personal',
    'WPA2-Enterprise'
  ];
  securityMode: string;

  securityTypes: string[] = [
    'AES',
    'TKIP'
  ];

  wifiList: WifiItemInterface[] = [];
  isWpsEnabled: boolean = false;
  wifiCNT = {
    waitTime: 1000,
    failTime: 10,
    attemptDict: {},
    rst: {},
  };

  EnabledForm = this.fb.group({
    iPower: 0,
    id: [''],
    sType: [''],
  });

  WifiSettingForm = this.fb.group({
    sName: ['', Validators.required],
    sService: ['', Validators.required],
    sPassword: [''],
    iFavorite: [''],
    iAutoconnect: [''],
    sState: [''],
  });

  get sName(): FormControl {
    return this.WifiSettingForm.get('sName') as FormControl;
  }

  get sService(): FormControl {
    return this.WifiSettingForm.get('sName') as FormControl;
  }

  get sPassword(): FormControl {
    return this.WifiSettingForm.get('sName') as FormControl;
  }

  ngOnInit() {
    this.isChrome = this.ieCss.getChromeBool();
    this.cfg.getWifiEnabledInterface().subscribe(res => {
      this.resError.analyseRes(res);
      this.EnabledForm.patchValue(res);
    });

    this.wifiScanOb = this.scanSub.asObservable().subscribe(
      change => {
        this.lock.unlock('onScan');
        this.lock.unlock('waitScan');
        if (this.pfs.checkNacActive('Wi-FiTab') && (this.EnabledForm.value.iPower || this.EnabledForm.value.iPower === undefined)) {
          switch (change) {
            case 0:
              this.waitScan(1000);
              this.tips.setRbTip('wifiTurnOn', 0);
              break;
            case 1:
              if (this.EnabledForm.value.iPower === undefined) {
                this.EnabledForm.get('iPower').enable();
              }
              if (this.wifiList.length <= 0) {
                this.tips.setRbTip('wifiScaning', 0);
                this.waitScan(1000);
              } else {
                this.tips.setRbTip('', -1);
                this.waitWifiStatus();
              }
              break;
            case -1:
              this.EnabledForm.get('iPower').enable();
              this.tips.setRbTip('getNoWifiInfoClickScan2Fresh');
              break;
          }
        } else {
          this.tips.setRbTip('', -1);
        }
      }
    );
  }

  ngOnDestroy() {
    if (this.ctOb) {
      this.ctOb.unsubscribe();
    }
    if (this.wifiScanOb) {
      this.wifiScanOb.unsubscribe();
    }
  }

  waitScan(waitTime: number) {
    this.lock.lock('waitScan');
    const that = this;
    setTimeout(
      () => {
        that.onScan();
      }
    , waitTime);
  }

  waitGetWifiList(waitTime: number) {
    const that = this;
    setTimeout(
      () => {
        that.getWifiList();
      }
    , waitTime);
  }

  turnWifiOn() {
    this.cfg.turnOnWifi().subscribe(
      res => {
        this.resError.analyseRes(res);
        if (!res['iPower']) {
          this.EnabledForm.patchValue(res);
        }
      },
      err => {
        this.logger.error(err, 'turnWifiOn:');
      }
    );
  }

  turnWifiOff() {
    this.cfg.turnOffWifi().subscribe(
      res => {
        this.resError.analyseRes(res);
        if (res['iPower']) {
          this.EnabledForm.patchValue(res);
        }
      },
      err => {
        this.logger.error(err, 'turnWifiOff:');
      }
    );
  }

  onScan() {
    this.lock.lock('onScan');
    this.cfg.scanWifiList().subscribe(
      res => {
        this.resError.analyseRes(res, 'scanFail');
        this.getWifiList();
      },
      err => {
        this.lock.unlock('waitScan');
        this.lock.unlock('onScan');
        this.tips.setRbTip('scanFail');
        this.logger.error(err, 'onScan');
      });
  }

  getWifiList() {
    this.cfg.getWifiItemInterface().subscribe(
      (res: WifiItemInterface[]) => {
        this.resError.analyseRes(res);
        if (res.length > 0) {
          this.sortWifiList(res);
          this.scanSub.next(1);
        } else {
          this.wifiList = [];
          this.scanSub.next(0);
        }
      },
      err => {
        this.wifiList = [];
        this.scanSub.next(-1);
      }
    );
  }

  sortWifiList(rawList: WifiItemInterface[] = []) {
    this.wifiList = [];
    let favMark = [];
    const listLen = rawList.length;
    for (let i = listLen - 1; i >= 0; i--) {
      if (rawList[i].Favorite === 1) {
        this.wifiList.push(rawList[i]);
        favMark.push(i);
      }
    }
    this.rankListByStrength(this.wifiList);
    for (const num of favMark) {
      rawList.splice(num, 1);
    }
    favMark = null;
    this.rankListByStrength(rawList);
    while (true) {
      const ls = rawList.length;
      if ( ls > 0 && rawList[ls - 1].Strength === 0) {
        rawList.pop();
      } else {
        break;
      }
    }
    for (const item of rawList) {
      this.wifiList.push(item);
    }
  }

  rankListByStrength(rawList: WifiItemInterface[] = []) {
    const listLen = rawList.length;
    let noChange = 0;
    for (let tune = 0; tune < listLen - 1;) {
      noChange = 0;
      for (let i = listLen - 1; i > tune; i--) {
        if (rawList[i].Strength > rawList[i - 1].Strength) {
          noChange = 0;
          const storage = rawList[i - 1];
          rawList[i - 1] = rawList[i];
          rawList[i] = storage;
        } else {
          noChange += 1;
        }
      }
      tune += (noChange + 1);
    }
  }

  onSelectedWifi(info: any) {
    if (info.Favorite === 1 && info.sState !== 'failure') {
      this.WifiSettingForm.patchValue({
        sName: info.sName,
        sService: info.sService,
        sPassword: '········',
        iFavorite: info.Favorite,
        iAutoconnect: 0,
        sState: info.sState,
      });
      this.WifiSettingForm.value.sPassword = '';
      this.WifiSettingForm.get('sPassword').disable();
    } else {
      this.WifiSettingForm.patchValue({
        sName: info.sName,
        sService: info.sService,
        sPassword: '',
        iFavorite: info.Favorite,
        iAutoconnect: 0,
        sState: info.sState,
      });
      this.WifiSettingForm.get('sPassword').enable();
    }
  }

  onSubmit() {
    this.wifiCNT.attemptDict[this.WifiSettingForm.value.sService] = 0;
    this.wifiCNT.rst[this.WifiSettingForm.value.sService] = 0;
    this.pfs.formatInt(this.WifiSettingForm.value);
    this.cfg.setPutWifiInterface(this.WifiSettingForm.value).subscribe(
      res => {
        this.resError.analyseRes(res, 'wifiFail');
        this.waitScan(this.wifiCNT.waitTime);
      },
      err => {
        this.tips.setRbTip('wifiFail');
      }
    );
  }

  waitWifiStatus() {
    if (this.lock.checkLock('waitWifiStatus')) {
      return;
    }
    this.lock.lock('waitWifiStatus');
    const rmList = [];
    const rstRmList = [];
    for (const key of this.pfs.objectKeys(this.wifiCNT.rst)) {
      if (this.wifiCNT.rst[key] === -1) {
        rstRmList.push(key);
        continue;
      }
      for (const item of this.wifiList) {
        if (key === item.sService) {
          if (item.Favorite === 1 || item.sState === 'ready' || item.sState === 'online') {
            this.wifiCNT.rst[key] = 1;
            rmList.push(key);
          } else {
            this.wifiCNT.attemptDict[key] = this.wifiCNT.attemptDict[key] + 1;
            if (this.wifiCNT.attemptDict[key] > this.wifiCNT.failTime) {
              this.wifiCNT.rst[key] = -1;
              rmList.push(key);
              // fake state forTest
              item.sState = 'fail';
              this.cfg.delteOneWifi(key).subscribe(
                res => {
                  const msg = this.resError.isErrorCode(res);
                  if (msg) {
                    this.logger.error(msg, 'waitWifiStatus:delteOneWifi:isErrorCode:');
                    this.tips.setRbTip('deleteFail');
                  } else {
                    this.attemptReconnect();
                  }
                },
                err => {
                  this.logger.error(err, 'waitWifiStatus:delteOneWifi:');
                }
              );
            } else {
              // fake state forTest
              item.sState = 'attempting';
            }
          }
          break;
        }
      }
    }
    for (const key of rmList) {
      delete this.wifiCNT.attemptDict[key];
    }
    for (const key of rstRmList) {
      delete this.wifiCNT.rst[key];
    }
    this.lock.unlock('waitWifiStatus');
    if (this.pfs.objectKeys(this.wifiCNT.attemptDict).length > 0) {
      this.waitScan(this.wifiCNT.waitTime);
    }
  }

  attemptReconnect = () => {
    this.cfg.getAutoConnectWifi().subscribe(
      res => {
        const msg = this.resError.isErrorCode(res);
        if (msg) {
          this.logger.error(msg, 'attemptReconnect:isErrorCode:');
        } else {
          if (res[0] && res[0]['sService']) {
            const info = {
              Favorite: 1,
              sName: 'reconnect',
              sState: '',
              sService: res[0]['sService']
            };
            this.onSelectedWifi(info);
            this.WifiSettingForm.get('iAutoconnect').setValue(1);
            this.tips.setRbTip('recntWifi');
            this.onSubmit();
          } else {
            this.tips.setRbTip('noWifi2Recnt');
          }
        }
      },
      err => {
        this.logger.error(err, 'attemptReconnect:');
      }
    );
  }

  isWifiSelected(info: any) {
    if (this.WifiSettingForm.value.sName === info.sName) {
      return true;
    }
    return false;
  }

  onShowCenterTip(judge: boolean) {
    if (judge) {
      this.tips.showCTip('WIFI.deleteOne');
      this.ctOb = this.tips.ctAction.subscribe(
        change => {
          if (change === 'onYes') {
            this.WifiSettingForm.get('sName').setValue('test');
            this.tips.setCTPara('close');
            this.cfg.delteOneWifi(this.WifiSettingForm.value.sService).subscribe(
              res => {
                this.resError.analyseRes(res);
                delete this.wifiCNT.rst[this.WifiSettingForm.value.sService];
                this.WifiSettingForm.get('sService').setValue('');
                this.WifiSettingForm.get('sName').setValue('');
                this.WifiSettingForm.get('sPassword').setValue('');
                this.WifiSettingForm.get('sState').setValue('');
                this.tips.setRbTip('deleteSuccess');
              },
              err => {
                this.tips.setRbTip('deleteFail');
              }
            );
          } else if (change === 'onNo') {
            this.ctOb.unsubscribe();
            this.ctOb = null;
          }
        }
      );
    } else {
      this.tips.showCTip('WIFI.deleteOne');
      const that = this;
      // when yse
      that.WifiSettingForm.get('sName').setValue('testall');
    }
  }

  onPowerSwitch(change: boolean) {
    if (change) {
      this.EnabledForm.get('iPower').disable();
      this.onScan();
    } else {
      this.wifiList = [];
    }
  }

  onPowerClick() {
    if (this.EnabledForm.value.iPower === undefined) {
      return;
    }
    if (this.EnabledForm.value.iPower) {
      this.turnWifiOff();
    } else {
      this.turnWifiOn();
    }
  }

  getCntStatus(info: any, index: number | string) {
    let cl = '';
    const cnt = this.wifiCNT.rst[info['sService']];
    if (cnt !== undefined) {
      switch (cnt) {
        case 0:
          cl = 'wait-cnt';
          break;
        case 1:
          cl = 'success-cnt';
          break;
        case -1:
          cl = 'fail-cnt';
          break;
      }
    }
    if (this.isWifiSelected(info)) {
      if (cl === '') {
        cl = 'table-primary';
      } else {
        cl += '-deep';
      }
    }
    if (cl === '') {
      if (Number(index) % 2 === 0) {
        cl = 'table-gray';
      } else {
        cl = 'white-bg';
      }
    }
    const rst = {};
    rst[cl] = true;
    return rst;
  }

}
