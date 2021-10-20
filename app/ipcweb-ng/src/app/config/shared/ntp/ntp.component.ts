import { Component, OnInit, OnDestroy, AfterViewInit, Renderer2, ElementRef, ViewChild } from '@angular/core';
import { ConfigService } from 'src/app/config.service';
import { FormBuilder, Validators, FormControl } from '@angular/forms';

import { TimeZoneInterface } from './NtpInterface';
import { isNumberJudge } from 'src/app/shared/validators/benumber.directive';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { BoolEmployee, EmployeeItem } from 'src/app/shared/func-service/employee.service';
import { TipsService } from 'src/app/tips/tips.service';
import { LockService } from 'src/app/shared/func-service/lock-service.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { isStandardTime } from 'src/app/shared/validators/is-standard-time.directive';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-ntp',
  templateUrl: './ntp.component.html',
  styleUrls: ['./ntp.component.scss'],
})
export class NtpComponent implements OnInit, AfterViewInit, OnDestroy {

  constructor(
    private cfg: ConfigService,
    private fb: FormBuilder,
    private Re2: Renderer2,
    private el: ElementRef,
    private ieCss: IeCssService,
    private resError: ResponseErrorService,
    private tips: TipsService,
    private pfs: PublicFuncService,
  ) { }

  @ViewChild('modal', {read: ElementRef})
  modalChild: ElementRef;

  // isLocalTime: boolean = false;
  testTip: string = '';
  private modeChange: any;
  private localTimeOb: any;
  private startTime: string;
  private rcdTime: any;
  private timer: any;
  private setTimer: any;
  timezoneList: TimeZoneInterface[];
  isChrome: boolean = false;
  private employee = new BoolEmployee();
  private lock = new LockService(this.tips);
  private logger: Logger = new Logger('ntp');

  epBank: EmployeeItem[] = [
    {
      name: 'init',
      task: ['timeZone', 'ntp', 'now'],
    },
    {
      name: 'save',
      task: ['time', 'ntp']
    }
  ];

  LocalForm = this.fb.group({
    isLocalTime: false,
    setTime: ['', isStandardTime],
  });

  NtpForm = this.fb.group({
    iAutoDst: [''],
    iAutoMode: [''],
    iRefreshTime: ['', [Validators.required, isNumberJudge]],
    sNtpServers: ['', Validators.required],
    sTimeZone: 'HawaiianStandardTime10',
    sTimeZoneFile: [''],
    sTimeZoneFileDst: ['']
  });

  get iRefreshTime(): FormControl {
    return this.NtpForm.get('iRefreshTime') as FormControl;
  }

  TimeForm = this.fb.group({
    time: [''],
  });

  SetTimeForm = this.fb.group({
    time: [''],
  });

  ngOnInit() {
    this.isChrome = this.ieCss.getChromeBool();
    this.employee.hire(this.epBank[0]);
    this.cfg.getTimZoneInfo().subscribe(
      (res: TimeZoneInterface[])  => {
        this.resError.analyseRes(res);
        this.timezoneList = res;
        this.employee.numTask(this.epBank, 0, 0, 1, 1);
      },
      err => {
        this.logger.error(err, 'ngOnInit:getTimZoneInfo:');
        this.employee.numTask(this.epBank, 0, 0, 0, 1);
      }
    );

    this.cfg.getNtpInfo().subscribe(
      res => {
        this.resError.analyseRes(res);
        this.NtpForm.patchValue(res);
        this.employee.numTask(this.epBank, 0, 1, 1, 1);
      },
      err => {
        this.logger.error(err, 'ngOnInit:getNtpInfo:');
        this.employee.numTask(this.epBank, 0, 1, 0, 1);
      }
    );

    this.cfg.getNowTime().subscribe(
      res => {
        this.resError.analyseRes(res);
        this.TimeForm.patchValue(res);
        const now = new Date();
        this.startTime = now.getFullYear() + '-' + this.addZero(now.getMonth() + 1) + '-' + this.addZero(now.getDate())
          + 'T' + this.addZero(now.getHours()) + ':' + this.addZero(now.getMinutes()) + ':' + this.addZero(now.getSeconds());
        this.LocalForm.get('setTime').setValue(this.TimeForm.value.time);
        this.rcdTime = new Date(this.LocalForm.value.setTime);
        this.setIntervalFunc();
        this.employee.numTask(this.epBank, 0, 2, 1, 1);
      },
      err => {
        this.logger.error(err, 'ngOnInit:getNowTime:');
        this.employee.numTask(this.epBank, 0, 2, 0, 1);
      }
    );

    this.employee.observeTask(this.epBank[0].name, 5000)
      .then(
        () => {
          if (!this.employee.getWorkResult(this.epBank[0].name)) {
            this.showInitFail();
          }
          this.employee.dismissOne(this.epBank[0].name);
        }
      )
      .catch(
        () => {
          this.showInitFail();
          this.employee.dismissOne(this.epBank[0].name);
        }
      );
  }

  showInitFail() {
    this.pfs.waitNavActive(20000, 'timeTab')
      .then(
        () => {
          this.tips.showInitFail();
        }
      )
      .catch(
        (error) => {
          this.logger.error(error, 'showInitFail:');
        }
      );
  }

  ngAfterViewInit() {
    this.Re2.setAttribute(this.el.nativeElement.querySelector('.device-time'), 'disabled', 'true');
    this.modeChange = this.NtpForm.get('iAutoMode').valueChanges.subscribe(
      change => {
        if (change) {
          this.Re2.removeAttribute(this.el.nativeElement.querySelector('.ntp-services'), 'disabled');
          this.Re2.removeAttribute(this.el.nativeElement.querySelector('.calibration-interval'), 'disabled');
          this.Re2.setAttribute(this.el.nativeElement.querySelector('.nowtime'), 'disabled', 'true');
          this.Re2.setAttribute(this.el.nativeElement.querySelector('.islocaltime'), 'disabled', 'true');
        } else {
          if (!this.LocalForm.value.isLocalTime) {
            this.Re2.removeAttribute(this.el.nativeElement.querySelector('.nowtime'), 'disabled');
          }
          this.Re2.removeAttribute(this.el.nativeElement.querySelector('.islocaltime'), 'disabled');
          this.Re2.setAttribute(this.el.nativeElement.querySelector('.ntp-services'), 'disabled', 'true');
          this.Re2.setAttribute(this.el.nativeElement.querySelector('.calibration-interval'), 'disabled', 'true');
        }
      }
    );

    this.localTimeOb = this.LocalForm.get('isLocalTime').valueChanges.subscribe(
      (change: boolean) => {
        if (change) {
          const now = new Date();
          const setTime = now.getFullYear() + '-' + this.addZero(now.getMonth() + 1) + '-' + this.addZero(now.getDate())
              + 'T' + this.addZero(now.getHours()) + ':' + this.addZero(now.getMinutes()) + ':' + this.addZero(now.getSeconds());
          this.LocalForm.get('setTime').setValue(setTime);
          this.Re2.setAttribute(this.el.nativeElement.querySelector('.nowtime'), 'disabled', 'true');
          this.setLocalIntervalFunc();
        } else {
          if (this.setTimer) {
            this.Re2.removeAttribute(this.el.nativeElement.querySelector('.nowtime'), 'disabled');
            clearInterval(this.setTimer);
          }
        }
      }
    );
  }

  ngOnDestroy() {
    if (this.modeChange) {
      this.modeChange.unsubscribe();
    }
    if (this.localTimeOb) {
      this.localTimeOb.unsubscribe();
    }
    if (this.timer) {
      clearInterval(this.timer);
    }
    if (this.setTimer) {
      clearInterval(this.setTimer);
    }
  }

  setIntervalFunc() {
    if (this.timer) {
      clearInterval(this.timer);
    }
    this.timer = setInterval(() => {
      const now = new Date();
      const recordTime = new Date(this.startTime);
      this.startTime = now.getFullYear() + '-' + this.addZero(now.getMonth() + 1) + '-' + this.addZero(now.getDate())
          + 'T' + this.addZero(now.getHours()) + ':' + this.addZero(now.getMinutes()) + ':' + this.addZero(now.getSeconds());
      const delta = now.getTime() - recordTime.getTime();
      const deltaDays = Math.floor(delta / (24 * 3600 * 1000));
      const leaveHours = delta % (24 * 3600 * 1000);
      const deltaHours = Math.floor(leaveHours / (3600 * 1000));
      const leaveMinutes = leaveHours % (3600 * 1000);
      const deltaMinutes = Math.floor(leaveMinutes / (60 * 1000));
      const leaveSeconds = leaveMinutes % (60 * 1000);
      const deltaSeconds = Math.floor(leaveSeconds / 1000);
      this.rcdTime.setDate(this.rcdTime.getDate() + deltaDays);
      this.rcdTime.setHours(this.rcdTime.getHours() + deltaHours);
      this.rcdTime.setMinutes(this.rcdTime.getMinutes() + deltaMinutes);
      this.rcdTime.setSeconds(this.rcdTime.getSeconds() + deltaSeconds);
      this.TimeForm.get('time').setValue(
        this.rcdTime.getFullYear() + '-' + this.addZero(this.rcdTime.getMonth() + 1) + '-' + this.addZero(this.rcdTime.getDate()) + 'T'
        + this.addZero(this.rcdTime.getHours()) + ':' + this.addZero(this.rcdTime.getMinutes()) + ':'
        + this.addZero(this.rcdTime.getSeconds()));
    }, 1000);
  }

  setLocalIntervalFunc() {
    if (this.setTimer) {
      clearInterval(this.setTimer);
    }
    this.setTimer = setInterval(() => {
      const now = new Date();
      const setTime = now.getFullYear() + '-' + this.addZero(now.getMonth() + 1) + '-' + this.addZero(now.getDate())
          + 'T' + this.addZero(now.getHours()) + ':' + this.addZero(now.getMinutes()) + ':' + this.addZero(now.getSeconds());
      this.LocalForm.get('setTime').setValue(setTime);
    }, 1000);
  }

  addZero(num: number) {
    if (num <= 9) {
      return '0' + num;
    } else {
      return num;
    }
  }

  onSubmit() {
    this.transformTimeZone();
    this.pfs.formatInt(this.NtpForm.value);
    try {
      this.cfg.putNtpInfo(this.NtpForm.value).subscribe(
        res => {
          this.resError.analyseRes(res);
          this.NtpForm.patchValue(res);
          if (!this.NtpForm.value.iAutoMode) {
            this.TimeForm.get('time').setValue(this.LocalForm.value.setTime);
            this.cfg.putNowTime(this.TimeForm.value).subscribe(
              dat => {
                this.resError.analyseRes(dat);
                const now = new Date();
                this.startTime = now.getFullYear() + '-' + this.addZero(now.getMonth() + 1) + '-' + this.addZero(now.getDate())
                  + 'T' + this.addZero(now.getHours()) + ':' + this.addZero(now.getMinutes()) + ':' + this.addZero(now.getSeconds());
                this.LocalForm.get('setTime').setValue(this.TimeForm.value.time);
                this.rcdTime = new Date(this.LocalForm.value.setTime);
                this.TimeForm.patchValue(dat);
                this.tips.showSaveSuccess();
              },
              err => {
                this.logger.error(err, 'onSubmit:putNowTime:');
                this.tips.showSaveFail();
              }
            );
          } else {
            setTimeout(this.freshTime(true), 500);
          }
        },
        err => {
          this.logger.error(err, 'onSubmit:putNtpInfo:');
          this.tips.showSaveFail();
        }
      );
    } catch (error) {
      this.logger.error(error, 'onSubmit:catch:');
      this.tips.showSaveFail();
    }
  }

  freshTime = (showTip: boolean) => {
    return () => {
      this.cfg.getNowTime().subscribe(
        dat => {
          this.resError.analyseRes(dat);
          this.TimeForm.patchValue(dat);
          const now = new Date();
          this.startTime = now.getFullYear() + '-' + this.addZero(now.getMonth() + 1) + '-' + this.addZero(now.getDate())
            + 'T' + this.addZero(now.getHours()) + ':' + this.addZero(now.getMinutes()) + ':' + this.addZero(now.getSeconds());
          this.LocalForm.get('setTime').setValue(this.TimeForm.value.time);
          this.rcdTime = new Date(this.LocalForm.value.setTime);
          this.setIntervalFunc();
          if (showTip) {
            this.tips.showSaveSuccess();
          }
        },
        err => {
          this.logger.error(err, 'onSubmit:putNtpInfo:getNowTime:');
          if (showTip) {
            this.tips.showSaveFail();
          }
        }
      );
    };
  }

  transformTimeZone() {
    for (const item of this.timezoneList) {
      if (item.sTimeZone === this.NtpForm.value.sTimeZone) {
        this.NtpForm.value.sTimeZoneFile = item.sTimeZoneFile;
        this.NtpForm.value.sTimeZoneFileDst = item.sTimeZoneFileDst;
      }
    }
  }

  onShowTip() {
    this.Re2.setStyle(this.modalChild.nativeElement.querySelector('.modal'), 'display', 'block');
    this.modalChild.nativeElement.querySelector('button.ml-2.quit').hidden = true;
    this.testTip = 'NTP.testing';
    const that = this;
    setTimeout(() => {
      that.testTip = 'NTP.testSuccess';
      that.modalChild.nativeElement.querySelector('button.ml-2.quit').hidden = false;
    }, 2000);
  }

  closeModal() {
    this.Re2.setStyle(this.modalChild.nativeElement.querySelector('.modal'), 'display', 'none');
  }
}
