import { Component, OnInit, OnDestroy, ViewChild } from '@angular/core';
import { FormBuilder, Validators } from '@angular/forms';
import { Subject } from 'rxjs';
import { ConfigService } from 'src/app/config.service';
import { HardDiskManagementInterface } from './HardDiskManagementInterface';
import { QuotaInterface } from './QuotaInterface';
import { isNumberJudge } from 'src/app/shared/validators/benumber.directive';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { EmployeeService, BoolEmployee, EmployeeItem } from 'src/app/shared/func-service/employee.service';
import { LockService } from 'src/app/shared/func-service/lock-service.service';
import { TipsService } from 'src/app/tips/tips.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-hard-disk-management',
  templateUrl: './hard-disk-management.component.html',
  styleUrls: ['./hard-disk-management.component.scss']
})
export class HardDiskManagementComponent implements OnInit, OnDestroy {

  hddList: HardDiskManagementInterface[] = [];

  constructor(
    private cfg: ConfigService,
    private fb: FormBuilder,
    private ieCss: IeCssService,
    private resError: ResponseErrorService,
    private ES: EmployeeService,
    private tips: TipsService,
    private pfs: PublicFuncService,
  ) { }

  private logger: Logger = new Logger('hard-disk-management');

  isChrome: boolean = false;
  isFormat: boolean = false;
  lock = new LockService(this.tips);
  private videoStatusSub = new Subject<number>();
  private videoStatusOb = null;
  employee = new BoolEmployee();
  epBank: EmployeeItem[] = [
    {
      name: 'init',
      task: ['list', 'path']
    }
  ];

  private recordStatus = {
    eventSnap: 0,
    timeSnap: 0,
    video: 0,
  }

  private recordCtl = {
    subject: null,
    ob: null,
    eventSnap: 0,
    timeSnap: 0,
    video: 0,
    reset: () => {
      this.recordCtl.eventSnap = 0;
      this.recordCtl.timeSnap = 0;
      this.recordCtl.video = 0;
    },
    isComplete: () => {
      if (!this.recordCtl.eventSnap || !this.recordCtl.timeSnap || !this.recordCtl.video) {
        return 0;
      }
      const rst = this.recordCtl.eventSnap + this.recordCtl.timeSnap + this.recordCtl.video;
      if (rst >= 3) {
        return 1;
      } else {
        return -1;
      }
    },
    doOne: (key: string) => {
      this.recordCtl[key] = 1;
      this.recordCtl.subject.next(key);
    },
    failOne: (key: string) => {
      this.recordCtl[key] = -1;
    },
    init: () => {
      this.recordCtl.subject = new Subject<string>();
    }
  }

  selectPath: string = '';
  formatEmployeeName: string = 'hard-disk-format';
  avalidDisk: Array<number> = [];

  HardDiskManagementInterfaceForm = this.fb.group({
    selected: [false],
    id: [''],
    iFormatStatus: [''],
    iFormatProg: [''],
    iFreeSize: [''],
    iMediaSize: [''],
    sStatus: [''],
    iTotalSize: [''],
    sAttributes: [''],
    sDev: [''],
    sMountPath: [''],
    sName: [''],
    sType: ['']
  });

  QuotaInterfaceForm = this.fb.group({
    id: [''],
    iFreePictureQuota: [''],
    iFreeVideoQuota: [''],
    iTotalPictureVolume: [''],
    iTotalVideoVolume: [''],
    iPictureQuotaRatio: ['', [Validators.required, isNumberJudge]],
    iVideoQuotaRatio: ['', [Validators.required, isNumberJudge]]
  });

  selectForm = this.fb.group({
    selectAll: [false],
  });

  ngOnInit(): void {
    this.recordCtl.init();
    this.isChrome = this.ieCss.getChromeBool();
    this.updateHddList();
    this.employee.hire(this.epBank[0]);
    this.cfg.getStoragePath().subscribe(
      res => {
        this.resError.analyseRes(res);
        this.selectPath = res['sMountPath'];
        this.employee.doTask(this.epBank[0].name, this.epBank[0].task[0], true);
      },
      err => {
        this.logger.error(err, 'ngOnInit');
        this.employee.doTask(this.epBank[0].name, this.epBank[0].task[0], false);
      }
    );
    this.employee.observeTask(this.epBank[0].name, 10000)
      .then(() => {
        if (this.employee.getWorkResult(this.epBank[0].name)) {
          let idNum = 3;
          for (const item of this.hddList) {
            if (item.sMountPath === this.selectPath) {
              idNum = item.id;
              break;
            }
          }
          this.updateQuota(idNum);
          this.QuotaInterfaceForm.get('iFreePictureQuota').disable();
          this.QuotaInterfaceForm.get('iFreeVideoQuota').disable();
          this.QuotaInterfaceForm.get('iTotalPictureVolume').disable();
          this.QuotaInterfaceForm.get('iTotalVideoVolume').disable();
          this.employee.dismissOne(this.epBank[0].name);
        } else {
          this.tips.showInitFail();
          this.logger.error('ngOnInit:observeTask:no-name');
          this.employee.dismissOne(this.epBank[0].name);
        }
      })
      .catch((error) => {
        this.logger.error(error, 'ngOnInit:observeTask:catch');
        this.tips.showInitFail();
        this.employee.dismissOne(this.epBank[0].name);
      });
  }

  ngOnDestroy() {
    this.ES.dismissOne(this.formatEmployeeName);
  }

  sizeCompare(size1: number, size2: number) {
    if (size1 === size2) {
      return true;
    }
    const bigSize = Math.max(size1, size2);
    const smallSize = Math.min(size1, size2);
    const deltaSize = bigSize - smallSize;
    return deltaSize < 0.001;
  }

  updateHddList(): void {
    if (this.lock.checkLock('updateHddList')) {
      return;
    }
    this.lock.lock('updateHddList');
    this.cfg.getHardDiskManagementInterface().subscribe(
      (res: HardDiskManagementInterface[]) => {
        this.resError.analyseRes(res);
        const Len = res.length;
        this.hddList = [];
        for (let i = 0; i < Len; i++) {
          this.hddList.push(null);
        }
        for (let i = 0; i < Len; i++) {
          res[i].iTotalSize = Number(res[i].iTotalSize.toFixed(3));
          res[i].iFreeSize = Number(res[i].iFreeSize.toFixed(3));
          res[i].selected = false;
          this.hddList[res[i]['id'] - 1] = res[i];
        }
        // add for options
        this.updateAvalidDisk(res);
        this.HardDiskManagementInterfaceForm.patchValue(res);
        this.employee.doTask(this.epBank[0].name, this.epBank[0].task[1], true, null);
        this.lock.unlock('updateHddList');
      },
      err => {
        this.logger.error(err, 'updateHddList');
        this.employee.doTask(this.epBank[0].name, this.epBank[0].task[1], false, null);
        this.lock.unlock('updateHddList');
      }
    );
  }

  updateAvalidDisk(rawRes: HardDiskManagementInterface[]) {
    this.avalidDisk = [];
    for (const item of rawRes) {
      if (item.sStatus === 'mounted') {
        this.avalidDisk.push(item.id);
      }
    }
    for (const item of this.avalidDisk) {
      if (!this.QuotaInterfaceForm.value.id || this.QuotaInterfaceForm.value.id === item) {
        return;
      }
    }
    if (this.avalidDisk.length > 0) {
      this.QuotaInterfaceForm.get('id').setValue(this.avalidDisk[0]);
    } else {
      this.tips.setRbTip('noAvalidDisk');
      this.logger.error('updateAvalidDisk:noAvalidDisk');
    }
  }

  updateQuota(quotaId: number): void {
    if (this.lock.checkLock('updateQuota')) {
      return;
    }
    this.lock.lock('updateQuota');
    this.cfg.getQuotaInterface(quotaId).subscribe(
      (res: QuotaInterface) => {
        this.resError.analyseRes(res);
        res.iFreePictureQuota = Number(res.iFreePictureQuota.toFixed(3));
        res.iFreeVideoQuota = Number(res.iFreeVideoQuota.toFixed(3));
        res.iTotalPictureVolume = Number(res.iTotalPictureVolume.toFixed(3));
        res.iTotalVideoVolume = Number(res.iTotalVideoVolume.toFixed(3));
        this.QuotaInterfaceForm.patchValue(res);
        this.lock.unlock('updateQuota');
      },
      err => {
        this.tips.setRbTip('inquiryQuotaFail');
        this.logger.error('updateQuota:inquiryQuotaFail');
        this.lock.unlock('updateQuota');
      }
    );
  }

  onSubmit() {
    this.pfs.formatInt(this.QuotaInterfaceForm.value);
    this.cfg.setQuotaInterface(this.QuotaInterfaceForm.value.id, this.QuotaInterfaceForm.value)
      .subscribe(res => {
      this.resError.analyseRes(res);
      this.tips.showSaveSuccess();
    },
    err => {
      this.logger.error(err, 'onSubmit:setQuotaInterface:');
      this.tips.showSaveFail();
    });
  }

  ctTipQuit(tipStr: string) {
    this.tips.setCTip(tipStr);
    this.tips.setCTPara('quit');
    this.tips.setCTPara('showNo');
  }

  ctTipQuitMoreTip(tipStr: Array<string>) {
    this.tips.setCTMoreTip(tipStr);
    this.tips.setCTPara('showNo');
  }

  format(stopMedia: boolean = false) {
    if (!this.ES.hireOne(this.formatEmployeeName)) {
      this.tips.setRbTip('formatingNotClick');
      this.logger.error('format:formatingNotClick:');
      return;
    }
    const Len = this.hddList.length;
    let formatCnt = 0;
    for (let i = 0; i < Len; i++) {
      if (this.hddList[i].selected === true) {
        formatCnt += 1;
        this.ES.pushTaskList(this.formatEmployeeName, this.hddList[i].id.toString());
      }
    }
    if (formatCnt <= 0) {
      this.tips.setRbTip('noDiskSelect');
      this.logger.error('format:noDiskSelect:');
      this.ES.dismissOne(this.formatEmployeeName);
      return;
    }
    this.logger.debug('format number: ' + formatCnt);
    if (!stopMedia) {
      this.tips.showCTip('formatting');
      this.tips.setCTPara('hideAll');
      this.formatFunc();
      return;
    }
    this.tips.showCTip('stopMediaRecord');
    this.tips.setCTPara('hideAll');
    this.recordStatusGet();
    this.recordCtl.ob = this.recordCtl.subject.asObservable().subscribe(
      change => {
        const rst = this.recordCtl.isComplete();
        if (rst) {
          this.recordCtl.ob.unsubscribe();
          if (this.recordCtl.ob) {
            this.recordCtl.ob = null;
          }
          if (rst > 0) {
            this.format1();
          } else {
            this.ctTipQuit('getMediaRecordFail');
          }
        }
      }
    )
  }

  format1() {
    this.recordStop();
    this.recordCtl.ob = this.recordCtl.subject.asObservable().subscribe(
      change => {
        const rst = this.recordCtl.isComplete();
        if (rst) {
          this.recordCtl.ob.unsubscribe();
          if (this.recordCtl.ob) {
            this.recordCtl.ob = null;
          }
          if (rst > 0) {
            this.format2();
          } else {
            this.ctTipQuit('stopMediaRecordFail');
          }
        }
      }
    )
  }

  requireVideoStatus = () => {
    this.cfg.getRecordStatus().subscribe(
      res => {
        const rst = Number(res);
        if (isNaN(rst)) {
          this.videoStatusSub.next(-1);
        } else if (rst < 0) {
          this.videoStatusSub.next(-1);
        } else {
          this.videoStatusSub.next(rst);
        }
      },
      err => {
        this.videoStatusSub.next(-1);
      }
    );
  }

  format2() {
    let failCnt = 0;
    let searchTime = 0;
    if (this.videoStatusOb) {
      this.videoStatusOb.unsubscribe();
      this.videoStatusOb = null;
    }
    this.videoStatusOb = this.videoStatusSub.asObservable().subscribe(
      change => {
        searchTime += 1;
        if (change == 0) {
          this.tips.setCTip('formatting');
          this.formatFunc();
          this.videoStatusOb.unsubscribe();
          if (this.videoStatusOb) {
            this.videoStatusOb = null;
          }
        } else if (change > 0){
          setTimeout(this.requireVideoStatus, 500);
        } else {
          failCnt += 1
          if (failCnt > 10) {
            this.ctTipQuit('stopMediaRecordFail');
            this.videoStatusOb.unsubscribe();
            if (this.videoStatusOb) {
              this.videoStatusOb = null;
            }
          } else {
            setTimeout(this.requireVideoStatus, 500);
          }
        }
        if (searchTime >= 10) {
          this.logger.info('format2:stop record self');
          this.cfg.sendRecordSignal(false).subscribe();
          searchTime = 0;
        }
      }
    )
    this.requireVideoStatus();
  }
  
  formatFunc() {
    for (const item of this.ES.bank[this.formatEmployeeName].taskList) {
      this.cfg.formatHardDisk(Number(item)).subscribe(res => {
      },
      err => {
        this.logger.error(err, 'formatFunc:formatHardDisk:');
      });
    }
    this.inquireFormatStatus();
    this.ES.observeTask(this.formatEmployeeName)
      .then(
        () => {
          this.format3('completeFormating');
          // this.tips.setRbTip('completeFormating');
          this.ES.dismissOne(this.formatEmployeeName);
        }
      )
      .catch(
        (error) => {
          this.format3('getFormatStatusFailFresh');
          // this.tips.setRbTip('getFormatStatusFailFresh');
          this.logger.error(error, 'format:observeTask:catch:');
          this.ES.dismissOne(this.formatEmployeeName);
        }
      );
  }

  format3(tipStr: string, restorageRecord: boolean = false) {
    if (!restorageRecord) {
      this.ctTipQuit(tipStr);
      return;
    }
    this.tips.setCTip(tipStr);
    const tipList = [];
    tipList.push('restartMediaRecord');
    this.tips.setCTMoreTip(tipList);
    tipList.pop();
    this.recordStatusSet();
    this.recordCtl.ob = this.recordCtl.subject.asObservable().subscribe(
      change => {
        const rst = this.recordCtl.isComplete();
        if (rst) {
          this.recordCtl.ob.unsubscribe();
          if (this.recordCtl.ob) {
            this.recordCtl.ob = null;
          }
          if (rst > 0) {
            tipList.push('restartMediaRecordSuccess');
            this.ctTipQuitMoreTip(tipList);
          } else {
            tipList.push('restartMediaRecordFail');
            this.ctTipQuitMoreTip(tipList);
          }
        }
      }
    )
  }

  inquireFormatStatus() {
    const that = this;
    setTimeout(() => {
      if (!that.ES.isTaskDone(that.formatEmployeeName)) {
        that.cfg.getHardDiskManagementInterface().subscribe(
          (res: HardDiskManagementInterface[]) => {
            const msg = that.resError.isErrorCode(res);
            if (msg) {
              that.logger.error(msg, 'inquireFormatStatus:getHardDiskManagementInterface:resError:');
            } else {
              const taskList = that.ES.getTaskList(that.formatEmployeeName);
              this.logger.debug(taskList, 'taskList:');
              for (const item of res) {
                if (this.pfs.isInArrayString(taskList, item.id.toString())) {
                  if (item.sName === 'Emmc' && item.sStatus === 'mounted') {
                    that.hddList[item.id - 1].iFormatProg = 100;
                    that.ES.doTask(that.formatEmployeeName, item.id.toString());
                    that.updateHddList();
                  } else if (item.sName === 'Emmc' && item.sStatus !== 'mounted') {
                    if (that.hddList[item.id - 1].iFormatProg < 90) {
                      that.hddList[item.id - 1].iFormatProg += Math.ceil(Math.random() * 10);
                    }
                  } else {
                    that.hddList[item.id - 1].iFormatStatus = item.iFormatStatus;
                    that.hddList[item.id - 1].iFormatProg = item.iFormatProg;
                    that.hddList[item.id - 1].sStatus = item.sStatus;
                    that.hddList[item.id - 1].iFreeSize = item.iFreeSize;
                    if (that.sizeCompare(that.hddList[item.id - 1].iTotalSize, that.hddList[item.id - 1].iFreeSize)) {
                      that.ES.doTask(that.formatEmployeeName, item.id - 1);
                      that.updateHddList();
                    }
                  }
                }
              }
            }
            that.inquireFormatStatus();
          },
          err => {
            that.inquireFormatStatus();
          }
        );
      }
    }, 1000);
  }

  checkAll(change: any) {
    this.lock.lock('checkAll');
    const Len = this.hddList.length;
    let changeCount = 0;
    for (let i = 0; i < Len; i++) {
      // if (this.hddList[i].sStatus !== 'unmounted' && this.hddList[i].sName !== 'Emmc') {
      if (this.hddList[i].sStatus !== 'unmounted') {
        changeCount += 1;
        this.hddList[i].selected = this.selectForm.value.selectAll;
      }
    }
    if (changeCount === 0) {
      this.selectForm.get('selectAll').setValue(!this.selectForm.value.selectAll);
      this.tips.setRbTip('noDiskCanBeSelected');
    }
    this.lock.unlock('checkAll');
  }

  onSelectedHardDisk(info: any) {
    if (info.sStatus !== 'unmounted') {
      // if (info.sName !== 'Emmc') {
      this.hddList[info.id - 1].selected = !this.hddList[info.id - 1].selected;
      // }
      this.selectPath = info.sMountPath;
      this.updateQuota(info.id);
    } else {
      this.tips.setRbTip('diskIsUnmounted');
    }
  }

  onSwitchDisk(id: number | string) {
    if (this.lock.checkLock('onSwitchDisk')) {
      return;
    }
    this.lock.lock('onSwitchDisk');
    id = Number(id);
    if (this.isHardDiskSelected(this.hddList[id - 1])) {
      this.lock.unlock('onSwitchDisk');
      return;
    }
    this.selectPath = this.hddList[id - 1].sMountPath;
    this.updateQuota(id);
    this.lock.unlock('onSwitchDisk');
  }

  isHardDiskSelected(info: any) {
    if (info['sMountPath'] === this.selectPath) {
      return true;
    } else {
      return false;
    }
  }

  recordStatusGet() {
    this.recordCtl.reset();
    this.cfg.getScreenshotTimingInfo().subscribe(
      res => {
        if (res['iEnabled'] == 0 || res['iEnabled'] == 1) {
          this.recordStatus['eventSnap'] = res['iEnabled'];
          this.recordCtl.doOne('eventSnap');
        } else {
          this.recordCtl.failOne('eventSnap');
          this.logger.debug(res['iEnabled'], 'recordStatusGet:getScreenshotTimingInfo:');
        }
      },
      err => {
        this.logger.debug(err, 'recordStatusGet:getScreenshotTimingInfo:');
        this.recordCtl.failOne('eventSnap');
      }
    )
    this.cfg.getScreenshotEventInfo().subscribe(
      res => {
        if (res['iEnabled'] == 0 || res['iEnabled'] == 1) {
          this.recordStatus['timeSnap'] = res['iEnabled'];
          this.recordCtl.doOne('timeSnap');
        } else {
          this.recordCtl.failOne('timeSnap');
          this.logger.debug(res['iEnabled'], 'recordStatusGet:getScreenshotEventInfo:');
        }
      },
      err => {
        this.logger.debug(err, 'recordStatusGet:getScreenshotEventInfo:');
        this.recordCtl.failOne('timeSnap');
      }
    )
    this.cfg.getPlanAdvancePara(0).subscribe(
      res => {
        if (res['iEnabled'] == 0 || res['iEnabled'] == 1) {
          this.recordStatus['video'] = res['iEnabled'];
          this.recordCtl.doOne('video');
        } else {
          this.recordCtl.failOne('video');
          this.logger.debug(res['iEnabled'], 'recordStatusGet:getPlanAdvancePara:');
        }
      },
      err => {
        this.logger.debug(err, 'recordStatusGet:getPlanAdvancePara:');
        this.recordCtl.failOne('video');
      }
    )
  }

  recordStatusSet() {
    this.recordCtl.reset();
    const data = {
      iEnabled: 0
    }
    data.iEnabled = this.recordStatus.eventSnap;
    this.cfg.putScreenshotEventInfo(data).subscribe(
      res => {
        if (res['iEnabled'] == 0 || res['iEnabled'] == 1) {
          this.recordCtl.doOne('eventSnap');
        } else {
          this.recordCtl.failOne('eventSnap');
          this.logger.debug(res['iEnabled'], 'recordStatusSet:getScreenshotTimingInfo:');
        }
      },
      err => {
        this.logger.debug(err, 'recordStatusSet:getScreenshotTimingInfo:');
        this.recordCtl.failOne('eventSnap');
      }
    )
    data.iEnabled = this.recordStatus.timeSnap;
    this.cfg.putScreenshotTimingInfo(data).subscribe(
      res => {
        if (res['iEnabled'] == 0 || res['iEnabled'] == 1) {
          this.recordCtl.doOne('timeSnap');
        } else {
          this.recordCtl.failOne('timeSnap');
          this.logger.debug(res['iEnabled'], 'recordStatusSet:putScreenshotTimingInfo:');
        }
      },
      err => {
        this.logger.debug(err, 'recordStatusSet:putScreenshotTimingInfo:');
        this.recordCtl.failOne('timeSnap');
      }
    )
    data.iEnabled = this.recordStatus.video;
    this.cfg.setPlanAdvancePara(0, data).subscribe(
      res => {
        if (res['iEnabled'] == 0 || res['iEnabled'] == 1) {
          this.recordCtl.doOne('video');
        } else {
          this.recordCtl.failOne('video');
          this.logger.debug(res['iEnabled'], 'recordStatusSet:setPlanAdvancePara:');
        }
      },
      err => {
        this.logger.debug(err, 'recordStatusSet:setPlanAdvancePara:');
        this.recordCtl.failOne('video');
      }
    )
  }

  recordStop() {
    this.recordCtl.reset();
    const data = {
      iEnabled: 0
    }
    this.cfg.putScreenshotEventInfo(data).subscribe(
      res => {
        if (res['iEnabled'] == 0 || res['iEnabled'] == 1) {
          this.recordCtl.doOne('eventSnap');
        } else {
          this.recordCtl.failOne('eventSnap');
          this.logger.debug(res['iEnabled'], 'recordStop:getScreenshotTimingInfo:');
        }
      },
      err => {
        this.logger.debug(err, 'recordStop:getScreenshotTimingInfo:');
        this.recordCtl.failOne('eventSnap');
      }
    )
    this.cfg.putScreenshotTimingInfo(data).subscribe(
      res => {
        if (res['iEnabled'] == 0 || res['iEnabled'] == 1) {
          this.recordCtl.doOne('timeSnap');
        } else {
          this.recordCtl.failOne('timeSnap');
          this.logger.debug(res['iEnabled'], 'recordStop:putScreenshotTimingInfo:');
        }
      },
      err => {
        this.logger.debug(err, 'recordStop:putScreenshotTimingInfo:');
        this.recordCtl.failOne('timeSnap');
      }
    )
    this.cfg.setPlanAdvancePara(0, data).subscribe(
      res => {
        if (res['iEnabled'] == 0 || res['iEnabled'] == 1) {
          this.recordCtl.doOne('video');
        } else {
          this.recordCtl.failOne('video');
          this.logger.debug(res['iEnabled'], 'recordStop:setPlanAdvancePara:');
        }
      },
      err => {
        this.logger.debug(err, 'recordStop:setPlanAdvancePara:');
        this.recordCtl.failOne('video');
      }
    )
  }
}
