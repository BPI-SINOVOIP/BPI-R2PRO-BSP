import { Component, OnInit, AfterViewInit, OnDestroy, Renderer2, ElementRef,
  HostListener, ViewChild, Input, ChangeDetectorRef } from '@angular/core';
import { FormBuilder, Validators } from '@angular/forms';

import { Subject } from 'rxjs';
import { ConfigService } from 'src/app/config.service';
import { environment } from 'src/environments/environment';
import { MemberListInterface } from '../list-management/ListMangementInterface';
import { EmployeeService } from 'src/app/shared/func-service/employee.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { ImgWorkshopService } from 'src/app/shared/func-service/img-workshop.service';
import { TipsService } from 'src/app/tips/tips.service';

import Logger from 'src/app/logger';

@Component({
  selector: 'app-batch-input',
  templateUrl: './batch-input.component.html',
  styleUrls: ['./batch-input.component.scss', '../face-page.scss']
})
export class BatchInputComponent implements OnInit, AfterViewInit, OnDestroy {

  @Input() options: any;

  constructor(
    private cfgService: ConfigService,
    private Re2: Renderer2,
    private el: ElementRef,
    private fb: FormBuilder,
    private ES: EmployeeService,
    private resError: ResponseErrorService,
    private pfs: PublicFuncService,
    private ieCss: IeCssService,
    private tips: TipsService,
    private iws: ImgWorkshopService,
    private cdr: ChangeDetectorRef,
  ) { }

  @ViewChild('add', {static: true}) addChild: any;
  @ViewChild('click', {static: true}) clickChild: any;
  private logger: Logger = new Logger('batch-input');

  private ctOb: any;
  serverUrl = environment.serverUrl;
  innerNumber: string = '';
  private ttlNumber: number = 0;
  private pageNum: number = 0;
  private ttlPage: number = 0;
  innerPage: string = '0 / 0';
  private isViewInit: boolean = false;
  private fileList = [];
  conditionType: Array<string> = [];
  listType: Array<string> = [];
  memberList: MemberListInterface[] = [];
  ttlList: MemberListInterface[] = [];
  private selectID: number;
  private perPageNum: number = 20;
  private timer: any;
  isIe: boolean = true;
  isFaceReg: boolean = true;
  private lastId: number = NaN;
  private failRegistration = {
    failNum: 0,
    failMsg: [],
  };
  private lawNum: number = 0;
  private imageOb: any;
  private imageSub: any;

  failReason = {
    0: 'interruptedUpload',
    '-1': 'featureFail',
    2: 'repeat'
  };

  imageReader = {
    name: 'batchImageChecker',
    isShowWaitTip: false,
    waitTip: 'waitForVerifing',
    height: 3000,
    width: 2000,
    size: 1024 * 1024,
    types: ['jpg', 'JPG', ],
    failList: [],
    qualifiedFile: 0,
    failedFile: 0,
    ttlFile: 0,
    reason: {
      height: 'overResolutionHeight',
      width: 'overResolutionWidth',
      size: 'oversizeLimit1M',
      type: 'typeErrorImg',
      error: 'fileError',
      abort: 'fileAbort'
    },
    reset: () => {
      this.imageReader.failList = null;
      this.imageReader.failList = [];
      this.imageReader.qualifiedFile = 0;
      this.imageReader.failedFile = 0;
      this.imageReader.ttlFile = 0;
    },
  };

  AddForm = this.fb.group({
    iAccessCardNumber: [0, Validators.required],
    sTelephoneNumber: [''],
    sAddress: [''],
    sBirthday: ['1970-01-01'],
    sCertificateNumber: ['', Validators.required],
    sCertificateType: ['identityCard'],
    sGender: ['male'],
    sHometown: [''],
    sListType: ['permanent'],
    sName: ['', Validators.required],
    sNation: ['汉族'],
    sNote: [''],
    sType: ['whiteList'],
  });

  fakeData = {
    iAccessCardNumber: 0,
    sTelephoneNumber: '',
    sAddress: '',
    sBirthday: '1970-01-01',
    sCertificateNumber: '',
    sCertificateType: 'identityCard',
    sGender: 'male',
    sHometown: '',
    sListType: 'permanent',
    sName: '',
    sNation: '汉族',
    sNote: 'undone',
    sType: 'whiteList',
  };

  number2feature = {
    1: 'success',
    '-1': 'fail',
    0: 'wait',
  };

  featureTip = {
    1: 'uploadSuccess',
    '-1': 'getFeatureFail',
  };

  workerBee = {
    /*
    set item when < todo start work,
    when > stop end work,
    */
    waitBuffer: {
      defaultTarget: 10,
      ttlWork: 0,
      ttlDone: 0,
      target: 10,
      maxTarget: 9007199254740991,
      work: 0,
      gap: 3000,
      employee: 'batchinput-bufferwaiter',
    },
    initTarget: (workerName: string) => {
      this.workerBee[workerName]['target'] = this.workerBee[workerName]['defaultTarget'];
      this.workerBee[workerName]['work'] = 0;
      this.workerBee[workerName]['ttlDone'] = 0;
    },
    reset: (workerName: string) => {
      this.workerBee[workerName]['work'] = 0;
    },
    startWait: (workerName: string) => {
      this.workerBee[workerName]['work'] = NaN;
    },
    workStatus: (workerName: string) => {
      const bee = this.workerBee[workerName];
      if (isNaN(bee['work'])) {
        return 'wait';
      } else if (bee['ttlDone'] >= bee['ttlWork']) {
        return 'stop';
      } else if (bee['work'] >= bee['target']) {
        return 'pause';
      } else {
        return 'todo';
      }
    },
    wait: (workerName: string, status: string, waitGap: number = 100) => new Promise(
      (resolve, reject) => {
        const waitWork = () => {
          if (this.workerBee.workStatus(workerName) === status) {
            resolve();
          } else {
            setTimeout(waitWork, waitGap);
          }
        };
        waitWork();
      }
    ),
  };

  waitComplete = {
    startTime: 0,
    // set little time of device get value
    timeGap: 200,
    inquireGap: 3000,
    fileNum: 0,
    lastID: 0,
    targetTime: 0,
    iLoadCompleted: 0,
    // improve get signal
    waitCompleteGap: 10000,
    setWaitTime: (fNum: number) => {
      this.waitComplete.fileNum = fNum;
      const waitTime = this.waitComplete.timeGap * this.waitComplete.fileNum;
      this.waitComplete.targetTime = this.waitComplete.startTime + waitTime;
    },
    atStartTime: (tNum: number) => {
      return tNum < this.waitComplete.targetTime ? this.waitComplete.targetTime - tNum : 0;
    },
    initPara: () => {
      this.waitComplete.iLoadCompleted = 0;
      this.waitComplete.lastID = NaN;
      this.waitComplete.fileNum = NaN;
    },
    checkWaitTime: (func: Function, attempFunc: Function) => {
      return (f = func, af = attempFunc) => {
        const nowTime = new Date();
        const deltaTime = this.waitComplete.targetTime - nowTime.getTime();
        if (this.waitComplete.iLoadCompleted !== 0) {
          return;
        } else if (deltaTime <= 0) {
          f();
        } else {
          af();
          setTimeout(this.waitComplete.checkWaitTime(f, af), Math.min(this.waitComplete.waitCompleteGap, deltaTime));
        }
      };
    },
  };


  ngOnInit(): void {
    this.isIe = this.ieCss.getIEBool();

    this.cfgService.getOverlaySnap().subscribe(
      res => {
        this.resError.analyseRes(res);
        res['iFaceRecognitionEnabled'] ? this.isFaceReg = true : this.isFaceReg = false;
      }
    );

    this.getServerUrl();
    this.setInnerNumber();
    this.cfgService.getFacePara().subscribe(
      res => {
        this.resError.analyseRes(res);
        this.conditionType = res['conditionType'];
        this.listType = res['listType'];
      }
    );
  }

  ngAfterViewInit() {
    this.resize();
    const width = document.getElementById('table-part').clientWidth;
    this.Re2.setStyle(this.el.nativeElement.querySelector('.page-foot'), 'width', width + 'px');
    this.isViewInit = true;
  }

  ngOnDestroy() {
    if (this.timer) {
      clearInterval(this.timer);
    }
    if (this.ctOb) {
      this.ctOb.unsubscribe();
    }
    if (this.el.nativeElement.querySelector('.batch-input-files')) {
      this.el.nativeElement.querySelector('.batch-input-files').value = '';
    }
    if (this.imageOb) {
      this.imageOb.unsubscribe();
    }
    this.imageReader.reset();
  }

  @HostListener('window:resize', ['$event'])
  onResize(event) {
    if (this.isViewInit) {
      this.resize();
    }
  }

  resize() {
    const height = document.getElementById('left-bar').clientHeight;
    const width = document.getElementById('table-part').clientWidth;
    this.Re2.setStyle(this.el.nativeElement.querySelector('.col-md-2.col-xl-2.sidebar.right'), 'height', height + 'px');
    this.Re2.setStyle(this.el.nativeElement.querySelector('.member-list'), 'height', height + 'px');
    this.Re2.setStyle(this.el.nativeElement.querySelector('.page-foot'), 'width', width + 'px');
  }

  getServerUrl() {
    const pat1 = /http:\/\/(.*?)\//;
    const pat2 = /https:\/\/(.*?)\//;
    let ip = pat1.exec(this.serverUrl);
    if (ip) {
      this.serverUrl = 'http://' + ip[1];
    } else {
      ip = pat2.exec(this.serverUrl);
      if (ip) {
        this.serverUrl = 'https://' + ip[1];
      } else {
        this.serverUrl = '';
      }
    }
  }

  setInnerNumber() {
    let nowNumber = 0;
    this.ttlNumber = this.ttlList.length;
    this.ttlPage = Math.ceil(this.ttlList.length / this.perPageNum);
    if (this.ttlList.length > 0) {
      const releaseNum = this.ttlNumber - this.perPageNum * (this.pageNum - 1);
      nowNumber = (releaseNum > this.perPageNum) ? this.perPageNum : releaseNum;
      this.memberList = this.ttlList.slice(this.perPageNum * (this.pageNum - 1), this.perPageNum * (this.pageNum - 1) + nowNumber);
    }
    if (this.memberList.length > 0) {
      for (const item of this.memberList) {
        item.sPicturePath += '?=' + Math.random();
      }
    }
    this.innerNumber = '总共 ' + this.ttlNumber + ' 条 当前 ' + nowNumber + '条';
    this.ttlPage = Math.ceil(this.ttlNumber / this.perPageNum);
    this.innerPage = this.pageNum + '/' + this.ttlPage;
  }

  onNo() {
    this.Re2.setStyle(this.el.nativeElement.querySelector('.modal'), 'display', 'none');
  }

  onShow() {
    this.imageReader.reset();
    this.Re2.setStyle(this.el.nativeElement.querySelector('.modal'), 'display', 'block');
    this.el.nativeElement.querySelector('.batch-input-files').value = '';
  }

  onPut() {
    this.logger.debug('in onPut');
  }

  onBlur() {
    this.logger.debug('in onBlur');
  }

  onFileChange(event: any) {
    this.imageReader.isShowWaitTip = true;
    this.imageReader.reset();
    this.fileList = [];
    this.lawNum = 0;
    this.imageReader.ttlFile = event.target.files.length;
    if (this.imageSub) {
      this.imageSub.complete();
      this.imageSub = null;
    }
    this.imageSub = new Subject<number>();
    this.imageOb = this.imageSub.asObservable().subscribe(
      (change: number) => {
        if (change < this.imageReader.ttlFile) {
          let item = event.target.files[change];
          if (!item['type'] || !item.type.match('image')) {
            this.imageReader.failList.push({
              name: item['name'],
              reason: 'typeErrorImg',
            });
            this.imageSub.next(change + 1);
          } else {
            this.iws.shapeJpeg(item, change, this.imageSub, this.fileList, this.imageReader.failList);
          }
        } else {
          this.imageReader.qualifiedFile = this.fileList.length;
          this.imageReader.failedFile = this.imageReader.failList.length;
          this.workerBee.waitBuffer.ttlWork = this.fileList.length;
          this.workerBee.waitBuffer.ttlDone = 0;
          this.imageReader.isShowWaitTip = false;
          this.logger.debug(this.imageReader.isShowWaitTip, 'end:');
          this.cdr.markForCheck();
          this.cdr.detectChanges();
          if (this.imageOb) {
            this.imageOb.unsubscribe();
            this.imageOb = null;
          }
        }
      }
    );
    this.imageSub.next(0);
  }

  onShowDetail() {
    this.tips.setCTTable(this.imageReader.failList);
  }

  onConfirm() {
    if (this.fileList.length === 0) {
      this.tips.setRbTip('selectFilesFirst');
      return;
    }
    // create employee for buffer wait
    this.ES.hireOne(this.workerBee.waitBuffer.employee);
    const nowTime = new Date();
    this.waitComplete.initPara();
    this.waitComplete.startTime = nowTime.getTime();
    this.waitComplete.setWaitTime(this.fileList.length);
    this.showWaitTip('waitForUploading');
    this.tips.setCTProgress(this.workerBee.waitBuffer.ttlWork);
    this.workerBee.initTarget('waitBuffer');
    this.pageNum = 1;
    this.fakeData.sType = this.AddForm.value.sType;
    this.fakeData.sListType = this.AddForm.value.sListType;
    const that = this;
    this.cfgService.getLastFaceId().subscribe(
      dat => {
        that.resError.analyseRes(dat);
        if (dat['id']) {
          that.lastId = Number(dat['id']);
        }
        that.cfgService.getBatchInputBuffer().subscribe(
          res => {
            that.resError.analyseRes(res);
            that.workerBee.waitBuffer.target += res['numOfWaiting'];
            that.workerBee.waitBuffer.work = res['numOfWaiting'];
            that.onConfirmFunc();
          }
        );
      }
    );
    this.onNo();
  }

  onConfirmFunc = () => {
    if (this.fileList.length > 0) {
      this.fakeData.sName = this.getFileName(this.fileList[this.fileList.length - 1].name);
      this.cfgService.putAddMemberInterface(this.fakeData, '').subscribe(
        res => {
          if (res['id'] || res['id'] === 0) {
            this.uploadImg(res.sPicturePath, res.id);
          } else {
            this.whenUploadFail();
            this.logger.error('no id', 'onConfirmFunc:putAddMemberInterface:');
          }
        },
        err => {
          this.whenUploadFail();
          this.logger.error(err, 'onConfirmFunc:putAddMemberInterface:');
        }
      );
    }
  }

  uploadImg(path: string, id: number) {
    path = encodeURI(path);
    const file = this.fileList.pop();
    this.cfgService.putAddMemberImage(file, path).subscribe(
      dat => {
        const datMsg = this.resError.isErrorCode(dat);
        if (datMsg) {
          this.logger.error(datMsg, 'uploadImg:putAddMemberImage:dat:');
          this.whenUploadFail();
        } else {
          this.AddForm.value.sName = this.getFileName(file.name);
          this.cfgService.putAddMemberInterface(this.AddForm.value, id.toString()).subscribe(
            res => {
              res.sPicturePath = this.serverUrl + res.sPicturePath;
              this.ttlList.push(res);
              this.workerBee.waitBuffer.work += 1;
              this.workerBee.waitBuffer.ttlDone += 1;
              this.tips.updateCTProgress(this.workerBee.waitBuffer.ttlDone);
              if (this.fileList.length === 0 && this.workerBee.workStatus('waitBuffer') === 'stop') {
                this.waitComplete.lastID = id;
                this.whenUploadSuccess();
              } else if (this.workerBee.workStatus('waitBuffer') !== 'pause') {
                this.onConfirmFunc();
              } else {
                this.workerBee.startWait('waitBuffer');
                this.inquireBuffer();
                this.workerBee.wait('waitBuffer', 'todo')
                  .then(
                    () => {this.onConfirmFunc(); }
                  )
                  .catch(
                    () => {this.onConfirmFunc(); }
                  );
              }
            },
            err => {
              this.logger.error(err, 'uploadImg:putAddMemberImage:putAddMemberInterface:');
              this.whenUploadFail();
            }
          );
        }
      },
      err => {
        this.whenUploadFail();
        this.logger.error(err, 'uploadImg:putAddMemberImage:');
      }
    );
  }

  inquireBuffer() {
    this.cfgService.getBatchInputBuffer().subscribe(
      res => {
        this.workerBee.waitBuffer.work = res['numOfWaiting'];
        if (this.workerBee.workStatus('waitBuffer') !== 'todo') {
          this.ES.addTaskItem(this.workerBee.waitBuffer.employee);
          const that = this;
          setTimeout(() => {
            that.ES.taskDone(this.workerBee.waitBuffer.employee);
          }, this.workerBee.waitBuffer.gap);
          this.ES.observeTask(this.workerBee.waitBuffer.employee)
            .then(
              () => {
                this.inquireBuffer();
              }
            );
        }
      },
      err => {
        if (this.workerBee.workStatus('waitBuffer') !== 'todo') {
          this.inquireBuffer();
        }
      }
    );
  }

  waitNextInquire() {
    setTimeout(this.inquireBuffer, 3000);
  }

  whenUploadSuccess() {
    this.tips.setCTip('uploadSuccessWaitForGetFeature');
    this.tips.setCTPara('closeProcess');
    const nowTime = new Date();
    const that = this;
    this.waitComplete.checkWaitTime(that.waitGetFeature, this.waitGetFeatureOneTime)();
    // setTimeout(this.waitGetFeature, this.waitComplete.atStartTime(nowTime.getTime()));
  }

  waitGetFeature = () => {
    this.cfgService.getAddMemberInterface(this.waitComplete.lastID).subscribe(
      res => {
        res['iLoadCompleted'] ? this.waitComplete.iLoadCompleted = res.iLoadCompleted : null;
        if (this.waitComplete.iLoadCompleted === 0) {
          setTimeout(this.waitGetFeature, this.waitComplete.inquireGap);
        } else {
          this.checkData();
        }
      },
      err => {
        this.logger.error(err, 'waitGetFeature:getAddMemberInterface:');
        setTimeout(this.waitGetFeature, this.waitComplete.inquireGap);
      }
    );
  }

  waitGetFeatureOneTime = () => {
    this.cfgService.getAddMemberInterface(this.waitComplete.lastID).subscribe(
      res => {
        res['iLoadCompleted'] ? this.waitComplete.iLoadCompleted = res.iLoadCompleted : null;
        if (this.waitComplete.iLoadCompleted !== 0) {
          this.checkData();
        }
      }
    );
  }

  checkData() {
    this.tips.setCTip('uploadSuccessAndGetFeatureSuccess');
    this.lastId = isNaN(this.lastId) ? 0 : this.lastId;
    this.cfgService.checkFaceFeature(this.lastId).subscribe(
      res => {
        this.resError.analyseRes(res, 'registerFail');
        const failList = [];
        this.failRegistration.failMsg = [];
        this.failRegistration.failNum = 0;
        if (res && res.length > 0) {
          for (const item of res) {
            const rea = this.failReason[item['iLoadCompleted']] ? this.failReason[item['iLoadCompleted']] : 'undefined';
            const pat = {
              name: item['sName'],
              reason: rea
            };
            this.failRegistration.failMsg.push(item);
            failList.push(pat);
          }
          this.failRegistration.failNum = res.length;
          const successNum = this.workerBee.waitBuffer.ttlDone - this.failRegistration.failNum;
          const tipList = ['registerComplete', 'successNum', successNum.toString(),
            'failNum', this.failRegistration.failNum.toString(), 'rk-endPoint'];
          this.tips.setCTip('');
          this.tips.setCTMoreTip(tipList);
          this.tips.setCTClickTable(failList);
          this.clearFailData(0);
          this.setInnerNumber();
        } else {
          this.endWaitTip('registerSuccess');
        }
      }
    );
  }

  whenUploadFail() {
    this.endWaitTip('uploadFail');
  }

  showWaitTip(cTipContent: string) {
    this.tips.showCTip(cTipContent);
    this.tips.setCTPara('hideAll');
    this.tips.setCTPara('quit');
  }

  endWaitTip(tip: string) {
    this.tips.setCTip(tip);
    this.tips.setCTPara('showNo');
    this.setInnerNumber();
  }

  onDblClick(id: number, event: any, order: number) {
    return;
    this.selectID = id;
    const that = this;
    this.clickChild.deleteFunc = () => {
      that.onDeleteComfirm();
    };
    this.clickChild.modifyFunc = () => {
      that.addChild.title = 'modifyFile';
      that.addChild.isRefeed = true;
      for (const i in that.ttlList) {
        if (that.ttlList[i].id === that.selectID) {
          that.addChild.refeedObj = that.ttlList[i];
          break;
        }
      }
      that.addChild.afterSaveFunc = () => {
        that.addChild.isRefeed = false;
        that.setInnerNumber();
        for (const i in that.ttlList) {
          if (that.ttlList[i].id === that.selectID) {
            that.ttlList[i].sPicturePath = that.serverUrl + that.ttlList[i].sPicturePath;
            break;
          }
        }
      };
      that.addChild.onShow(true, id, that.memberList[order]);
      that.clickChild.onNo();
    };
    this.clickChild.setXY(event.clientX, event.clientY);
    this.clickChild.onShow();
  }

  onDeleteComfirm() {
    this.tips.showCTip('Are you sure that delete info selected?');
    this.ctOb = this.tips.ctAction.subscribe(
      change => {
        if (change === 'onYes') {
          this.clickChild.onNo();
          this.cfgService.deleteMemberFace('/' + this.selectID).subscribe(
            res => {
              this.resError.analyseRes(res);
            }
          );
          for (const i in this.ttlList) {
            if (this.ttlList[i].id === this.selectID) {
              this.ttlList.splice(Number(i), 1);
              this.setInnerNumber();
            }
          }
          this.tips.setRbTip('deleteSuccess');
          this.ctOb.unsubscribe();
          this.ctOb = null;
          this.tips.setCTPara('close');
        }
      }
    );
  }

  onFileClick(id: string) {
    const event = new MouseEvent('click');
    const fileInputBtn = document.getElementById(id);
    fileInputBtn.dispatchEvent(event);
  }

  getFileName(rawName: string) {
    for (let i = rawName.length - 1; i >= 0; i--) {
      if (rawName[i] === '.') {
        return rawName.slice(0, i);
      }
    }
  }

  pageDoubleUp() {
    if (this.pageNum !== 1 && this.pageNum !== 0) {
      this.pageNum = 1;
      this.setInnerNumber();
    }
  }

  pageUp() {
    if (this.pageNum !== 1 && this.pageNum !== 0) {
      this.pageNum -= 1;
      this.setInnerNumber();
    }
  }

  pageDown() {
    if (this.pageNum < this.ttlPage && this.pageNum !== 0) {
      this.pageNum += 1;
      this.setInnerNumber();
    }
  }

  pageDoubleDown() {
    if (this.ttlPage !== 0 && this.pageNum !== this.ttlPage) {
      this.pageNum = this.ttlPage;
      this.setInnerNumber();
    }
  }

  clearFailData(start: number = 0) {
    if (this.failRegistration.failNum <= 0 || start >= this.ttlList.length) {
      return;
    }
    for (let i = start; i < this.ttlList.length; i++) {
      if (this.isFailRegistration(this.ttlList[i])) {
        this.ttlList.splice(i, 1);
        this.clearFailData(i);
        return;
      }
    }
  }

  isFailRegistration(item: MemberListInterface) {
    for (const ck of this.failRegistration.failMsg) {
      if ((item.sName === ck.sName) && (item.sPicturePath.match(ck.sPicturePath))) {
        return true;
      }
    }
    return false;
  }

  onModify(id: number, order: number) {
    const that = this;
    this.addChild.title = 'modifyFile';
    this.addChild.isRefeed = true;
    for (const i in this.ttlList) {
      if (this.ttlList[i].id === id) {
        this.addChild.refeedObj = this.ttlList[i];
        break;
      }
    }
    this.addChild.afterSaveFunc = () => {
      that.addChild.isRefeed = false;
      that.setInnerNumber();
      for (const i in that.ttlList) {
        if (that.ttlList[i].id === that.selectID) {
          that.ttlList[i].sPicturePath = that.serverUrl + that.ttlList[i].sPicturePath;
          break;
        }
      }
    };
    this.addChild.onShow(true, id, this.memberList[order]);
  }

  onDelete(id: number, order: number) {
    this.selectID = id;
    this.onDeleteComfirm();
  }

}
