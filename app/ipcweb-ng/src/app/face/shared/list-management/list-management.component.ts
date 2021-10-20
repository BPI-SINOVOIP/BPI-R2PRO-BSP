import { Component, OnInit, OnDestroy, AfterViewInit, Renderer2, ElementRef, HostListener, ViewChild, Input } from '@angular/core';
import { FormBuilder, Validators, FormControl } from '@angular/forms';

import { MemberListInterface, SearchCondition } from './ListMangementInterface';
import { ConfigService } from 'src/app/config.service';
import { timeOrderValidator } from 'src/app/shared/validators/time-compare.directive';
import { ageOrderValidator } from 'src/app/shared/validators/age-compare.directive';
import { Json2excelService } from 'src/app/face/json2excel.service';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { TipsService } from 'src/app/tips/tips.service';
import { LockService } from 'src/app/shared/func-service/lock-service.service';
import { BoolEmployee, EmployeeItem } from 'src/app/shared/func-service/employee.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { Downloader } from '../downloader';
import { LayoutService } from 'src/app/shared/func-service/layout.service';
import { isNumberJudge } from 'src/app/shared/validators/benumber.directive';
import { isStandardTime } from 'src/app/shared/validators/is-standard-time.directive';

import Logger from 'src/app/logger';

@Component({
  selector: 'app-list-management',
  templateUrl: './list-management.component.html',
  styleUrls: ['../face-page.scss']
})

export class ListManagementComponent implements OnInit, AfterViewInit, OnDestroy {

  @Input() options: any;

  constructor(
    private cfgService: ConfigService,
    private Re2: Renderer2,
    private el: ElementRef,
    private fb: FormBuilder,
    private j2e: Json2excelService,
    private ieCss: IeCssService,
    private resError: ResponseErrorService,
    private tips: TipsService,
    private pfs: PublicFuncService,
    private los: LayoutService,
  ) { }

  @ViewChild('add', {static: true}) addChild: any;
  @ViewChild('click', {static: true}) clickChild: any;
  private logger: Logger = new Logger('list-management');

  private employee = new BoolEmployee();
  searchOption: Array<string> = [
    'MNG.searchByCondition',
    'MNG.searchByName',
  ];

  conditionType: Array<string> = [
    'whiteList',
    'blackList',
    'all',
  ];

  genderType: Array<string> = [
    'male',
    'female',
    'all',
  ];

  number2feature = {
    1: 'success',
    '-1': 'fail',
    0: 'wait',
    2: 'repeat'
  };

  epBank: EmployeeItem[] = [
    {
      name: 'delete',
      task: [],
    },
    {
      name: 'download',
      task: [],
    }
  ];

  modeBtnName: string = 'deleteMode';
  isFaceReg: boolean = true;
  isChrome: boolean = false;
  private ctOb: any;
  private downloader: any;
  private losOb: any;
  modifyOp: boolean = false;

  innerNumber: string = '';
  ttlNumber: number = 0;
  private pageNum: number = 1;
  private ttlPage: number = 0;
  innerPage: string = '0 / 0';
  private isViewInit: boolean = false;
  isDeleteChecked: boolean = false;
  selectedItem: {} = {};
  memberList: MemberListInterface[] = [];
  selectedOption = 'MNG.searchByCondition';
  deleteCalList = {
    target: 0,
    done: 0,
  };

  set4Excel = {
    oneTimeNumber: 1000,
    isFirstSearch: true,
    fileName: 'list',
    column: [
      {
        title: 'id',
        key: 'id',
        type: 'text',
      },
      {
        title: 'sType',
        key: 'sType',
        type: 'text',
      },
      {
        title: 'sName',
        key: 'sName',
        type: 'text',
      },
      {
        title: 'iAccessCardNumber',
        key: 'iAccessCardNumber',
        type: 'text',
      },
      {
        title: 'sGender',
        key: 'sGender',
        type: 'text',
      },
      {
        title: 'iLoadCompleted',
        key: 'iLoadCompleted',
        type: 'text',
      },
      {
        title: 'sRegistrationTime',
        key: 'sRegistrationTime',
        type: 'text',
      },
      {
        title: 'sBirthday',
        key: 'sBirthday',
        type: 'text',
      },
      {
        title: 'sCertificateType',
        key: 'sCertificateType',
        type: 'text',
      },
      {
        title: 'sCertificateNumber',
        key: 'sCertificateNumber',
        type: 'text',
      },
      {
        title: 'sTelephoneNumber',
        key: 'sTelephoneNumber',
        type: 'text',
      },
      {
        title: 'sNation',
        key: 'sNation',
        type: 'text',
      },
      {
        title: 'sHometown',
        key: 'sHometown',
        type: 'text',
      },
      {
        title: 'sAddress',
        key: 'sAddress',
        type: 'text',
      },
      {
        title: 'sNote',
        key: 'sNote',
        type: 'text',
      },
    ]
  };

  conditionForm = this.fb.group({
    beginTime: ['', isStandardTime],
    endTime: ['', isStandardTime],
    type: 'all',
    gender: 'all',
    minAge: [0, Validators.required],
    maxAge: [100, Validators.required],
    accessCardNumber: [0, [Validators.required, isNumberJudge]],
    beginPosition: [''],
    endPosition: ['']
  }, { validators: [timeOrderValidator('beginTime', 'endTime'), ageOrderValidator('minAge', 'maxAge')]});

  usingConditionForm = this.fb.group({
    beginTime: [''],
    endTime: [''],
    type: 'all',
    gender: 'all',
    minAge: 0,
    maxAge: 100,
    accessCardNumber: 0,
    beginPosition: [''],
    endPosition: ['']
  });

  NameForm = this.fb.group({
    name: ['', Validators.required],
    beginPosition: [''],
    endPosition: [''],
  });

  usingNameForm = this.fb.group({
    name: [''],
    beginPosition: [''],
    endPosition: [''],
  });

  get minAge(): FormControl {
    return this.conditionForm.get('minAge') as FormControl;
  }

  get maxAge(): FormControl {
    return this.conditionForm.get('maxAge') as FormControl;
  }

  get accessCardNumber(): FormControl {
    return this.conditionForm.get('accessCardNumber') as FormControl;
  }

  get beginTime(): FormControl {
    return this.conditionForm.get('beginTime') as FormControl;
  }

  get endTime(): FormControl {
    return this.conditionForm.get('endTime') as FormControl;
  }

  get name(): FormControl {
    return this.NameForm.get('name') as FormControl;
  }

  ngOnInit(): void {
    this.isChrome = this.ieCss.getChromeBool();

    this.timeInit();
    this.onSearch(1);
    // for excel download
    this.j2e.initService();
    this.j2e.set4Column(this.set4Excel.fileName, this.set4Excel.column);
    this.j2e.addObjectVal('iLoadCompleted', this.number2feature);
    this.j2e.addArrayVal(this.genderType);
    this.j2e.addArrayVal(this.conditionType);
    const addtionList = ['identityCard', 'permanent'];
    this.j2e.addArrayVal(addtionList);

    const secondKey = (this.options && this.options['secondKey']) ? this.options['secondKey'] : 'face';
    const thirdKey = (this.options && this.options['thirdKey']) ? this.options['thirdKey'] : 'MemberList';
    const fourthKey = (this.options && this.options['fourthKey']) ? this.options['fourthKey'] : 'ListManagement';
    this.losOb = this.los.getfourthLayout(secondKey, thirdKey, fourthKey).subscribe(
      (rst: string) => {
        const rstJson = JSON.parse(rst);
        if (rstJson.length > 0 && rstJson[0] === 'modify') {
          this.modifyOp = true;
        } else {
          this.modifyOp = false;
        }
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
    if (this.ctOb) {
      this.ctOb.unsubscribe();
    }
    if (this.downloader) {
      this.downloader = null;
    }
    if (this.losOb) {
      this.losOb.unsubscribe();
    }
  }

  timeInit() {
    const today = new Date();
    const year = today.getFullYear().toString();
    let month = '';
    let day = '';
    if (today.getMonth() <= 8) {
      month =  '0' + (today.getMonth() + 1);
    } else {
      month = (today.getMonth() + 1).toString();
    }
    if (today.getDate() <= 9) {
      day = '0' + (today.getDate()).toString();
    } else {
      day = today.getDate().toString();
    }
    // this.conditionForm.get('beginTime').setValue(year + '-' + month + '-' + day + 'T00:00:01');
    this.conditionForm.get('beginTime').setValue('1970-01-01T00:00:00');
    this.conditionForm.get('endTime').setValue(year + '-' + month + '-' + day + 'T23:59:59');
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

  pageDoubleUp() {
    if (this.ttlPage) {
      this.searchFunc(1);
    }
  }

  pageUp() {
    if (this.ttlPage && this.pageNum - 1 >= 1) {
      this.searchFunc(this.pageNum - 1);
    }
  }

  pageDown() {
    if (this.ttlPage && this.pageNum + 1 <= this.ttlPage) {
      this.searchFunc(this.pageNum + 1);
    }
  }

  pageDoubleDown() {
    if (this.ttlPage) {
      this.searchFunc(this.ttlPage);
    }
  }

  isItemSelected(id: number) {
    if (this.selectedItem[id]) {
      return true;
    } else {
      return false;
    }
  }

  onDblClick(id: number, event: any, order: number) {
    if (!this.modifyOp) {
      return;
    }
    if (this.isDeleteChecked) {
      if (this.selectedItem[id]) {
        this.selectedItem[id] = false;
      } else {
        this.selectedItem[id] = true;
      }
    }
    // } else {
    //   this.selectedItem = {};
    //   this.selectedItem[id] = true;
    //   const that = this;
    //   this.clickChild.deleteFunc = () => {
    //     that.onDeleteComfirm();
    //   };
    //   this.clickChild.modifyFunc = () => {
    //     that.addChild.title = 'modifyInfo';
    //     that.addChild.afterSaveFunc = () => {
    //       that.searchFunc(that.pageNum);
    //     };
    //     that.addChild.onShow(true, id, that.memberList[order]);
    //     that.clickChild.onNo();
    //   };
    //   this.clickChild.setXY(event.clientX, event.clientY);
    //   this.clickChild.onShow();
    // }
  }

  onDeleteChecked() {
    this.isDeleteChecked = !this.isDeleteChecked;
    if (this.isDeleteChecked) {
      this.modeBtnName = 'inquireMode';
    } else {
      this.modeBtnName = 'deleteMode';
    }
    if (this.isDeleteChecked) {
      this.selectedItem = {};
    }
  }

  onSelectAll() {
    // this.selectedItem = {};
    for (const item of this.memberList) {
      this.selectedItem[item.id] = true;
    }
  }

  onUnselectAll() {
    this.selectedItem = {};
  }

  onDeleteComfirm() {
    this.employee.hireOne(this.epBank[0].name);
    this.tips.showCTip('deleteInfoWithControlTip');
    this.deleteCalList.target = 0;
    this.deleteCalList.done = 0;
    this.ctOb = this.tips.ctAction.subscribe(
      change => {
        if (change === 'onYes') {
          this.showWaitTip();
          // tslint:disable-next-line: forin
          for (const key in this.selectedItem) {
            this.deleteCalList.target += 1;
            if (this.selectedItem[key]) {
              this.employee.appendTask(this.epBank[0].name, key.toString());
              this.cfgService.deleteMemberFace('/' + key).subscribe(
                res => {
                  this.employee.doTask(this.epBank[0].name, key.toString(), true, null);
                  this.selectedItem[key] = false;
                  this.deleteCalList.done += 1;
                  const msg = this.resError.isErrorCode(res);
                  if (msg) {
                    this.logger.error(msg, 'onDeleteComfirm:ctAction:deleteMemberFace:');
                  }
                },
                err => {
                  this.employee.doTask(this.epBank[0].name, key.toString(), false, null);
                  this.selectedItem[key] = false;
                }
              );
            } else {
              this.deleteCalList.done += 1;
            }
          }
          this.clickChild.onNo();
          this.employee.observeTask(this.epBank[0].name, 100000)
            .then(() => {
              this.endWaitTip('deleteSuccess');
              if (this.pageNum <= this.ttlPage) {
                this.onSearch(this.pageNum);
              } else {
                this.onSearch(1);
              }
              this.employee.dismissOne(this.epBank[0].name);
            })
            .catch(() => {
              this.employee.dismissOne(this.epBank[0].name);
              this.endWaitTip('deleteFail');
            });
        } else if (change === 'onNo') {
          this.ctOb.unsubscribe();
          this.ctOb = null;
          this.selectedItem = {};
          this.employee.dismissOne(this.epBank[0].name, false);
        }
      }
    );
  }

  waitDeleteDone = (timeoutms: number) => new Promise((resolve, reject) => {
    const deleteDone = () => {
      timeoutms -= 100;
      if (this.deleteCalList.target <= this.deleteCalList.done) {
        resolve();
      } else if (timeoutms <= 0) {
        reject();
      } else {
        setTimeout(deleteDone, 100);
      }
    };
    deleteDone();
  })

  showWaitTip(cTipContent: string = 'waitForDelete') {
    this.tips.showCTip(cTipContent);
    this.tips.setCTPara('hideAll');
    this.tips.setCTPara('quit');
  }

  endWaitTip(tip: string) {
    this.tips.setCTip(tip);
    this.tips.setCTPara('showNo');
  }

  onSearch(page: number) {
    switch (this.selectedOption) {
      case 'MNG.searchByCondition':
        this.usingConditionForm.patchValue(this.conditionForm.value);
        break;
      case 'MNG.searchByName':
        this.usingNameForm.patchValue(this.NameForm.value);
        break;
    }
    this.searchFunc(page);
  }

  searchFunc(page: number, limitNum: number = 20) {
    page === 0 ? page = 1 : null;
    this.pageNum = page;
    if (this.selectedOption === 'MNG.searchByCondition') {
      this.usingConditionForm.value.beginPosition = (page - 1) * limitNum;
      if (page !== 1) {
        this.usingConditionForm.value.endPosition = Math.min((page) * limitNum - 1, this.ttlNumber);
      } else {
        this.usingConditionForm.value.endPosition = (page) * limitNum - 1;
      }
      this.pfs.formatInt(this.usingConditionForm.value);
      this.checkTimeFormat();
      this.cfgService.putMemberSearchCondition(this.usingConditionForm.value).subscribe(
        res => {
          const msg = this.resError.isErrorCode(res);
          if (msg) {
            this.logger.error(msg, 'searchFunc:putMemberSearchCondition:');
            this.resError.analyseRes(res);
          }
          this.memberList = res.matchList;
          // for excel download
          if (res.numOfMatches > 0 && this.set4Excel.isFirstSearch) {
            this.set4Excel.isFirstSearch = false;
            this.j2e.preAddTranslate(this.memberList[0]);
          }
          this.ttlNumber = res.numOfMatches;
          this.freshPicUrl();
          if (this.ttlNumber === 0) {
            this.pageNum = 0;
          }
          this.setInnerNumber();
        },
        err => {
          this.logger.error(err, 'searchFunc:putMemberSearchCondition:');
          this.pageNum -= 1;
          this.setInnerNumber();
        }
      );
    } else if (this.selectedOption === 'MNG.searchByName') {
      this.usingNameForm.value.beginPosition = (page - 1) * limitNum;
      if (page !== 1) {
        this.usingNameForm.value.endPosition = Math.min((page) * limitNum - 1, this.ttlNumber);
      } else {
        this.usingNameForm.value.endPosition = (page) * limitNum - 1;
      }
      this.pfs.formatInt(this.usingNameForm.value);
      this.cfgService.putMemberNameSearch(this.usingNameForm.value).subscribe(
        res => {
          const msg = this.resError.isErrorCode(res);
          if (msg) {
            this.logger.error(msg, 'searchFunc:putMemberNameSearch:');
            this.resError.analyseRes(res);
          }
          this.memberList = res.matchList;
          // for excel download
          if (res.numOfMatches > 0 && this.set4Excel.isFirstSearch) {
            this.set4Excel.isFirstSearch = false;
            this.j2e.preAddTranslate(this.memberList[0]);
          }
          this.freshPicUrl();
          this.ttlNumber = res.numOfMatches;
          if (this.ttlNumber === 0) {
            this.pageNum = 0;
          }
          this.setInnerNumber();
        },
        err => {
          this.logger.error(err, 'searchFunc:putMemberNameSearch:');
          this.pageNum -= 1;
          this.setInnerNumber();
        }
      );
    }
  }

  freshPicUrl() {
    if (this.memberList && this.memberList.length > 0) {
      for (const item of this.memberList) {
        item.sPicturePath += '?=' + Math.random();
      }
    }
  }

  setInnerNumber() {
    let nowNumber = 0;
    if (this.memberList) {
      nowNumber = this.memberList.length;
    }
    if (nowNumber <= 0) {
      this.tips.setRbTip('noResult');
    }
    this.innerNumber = '总共 ' + this.ttlNumber + ' 条 当前 ' + nowNumber + '条';
    this.ttlPage = Math.ceil(this.ttlNumber / 20);
    this.innerPage = this.pageNum + '/' + this.ttlPage;
  }

  onAdd() {
    this.addChild.title = 'addInfo';
    this.addChild.onShow();
    const that = this;
    this.addChild.afterSaveFunc = () => {
      that.searchFunc(that.pageNum);
    };
  }

  checkTimeFormat() {
    const pat = /^.*?-.*?-.*?T.*?:.*?:.*?$/;
    this.usingConditionForm.value.beginTime += (!pat.exec(this.usingConditionForm.value.beginTime) ? ':00' : '');
    this.usingConditionForm.value.endTime += (!pat.exec(this.usingConditionForm.value.endTime) ? ':00' : '');
  }

  downloadJson() {
    this.showWaitTip('prepareForDownloa');
    const oneTimeNumber = this.set4Excel.oneTimeNumber;
    // const oneTimeNumber = 2;
    this.downloader = new Downloader(oneTimeNumber, this.ttlNumber);
    this.epBank[1].task = [];
    for (let i = 1; i <= this.downloader.searchTime; i++) {
      this.epBank[1].task.push(i.toString());
    }
    this.employee.hire(this.epBank[1]);
    let jsonArray = [];
    switch (this.selectedOption) {
      case 'MNG.searchByCondition':
        this.usingConditionForm.patchValue(this.conditionForm.value);
        break;
      case 'MNG.searchByName':
        this.usingNameForm.patchValue(this.NameForm.value);
        break;
    }
    this.downloadLoop();
    this.employee.observeTask(this.epBank[1].name, 86400000)
      .then(
        () => {
          if (!this.employee.getWorkResult(this.epBank[1].name)) {
            this.endWaitTip('downloadPartFail');
            const pageRange: string = this.downloader.beginPosition.toString() + '~' + this.downloader.endPosition.toString();
            const tipsList: Array<string> = ['failPageRange', pageRange];
            this.tips.setCTMoreTip(tipsList);
          } else {
            for (let page = 1; page <= this.downloader.searchTime; page++) {
              jsonArray = jsonArray.concat(this.downloader.jsonDict[page.toString()]);
            }
            this.downloadExcel(this.set4Excel.fileName, jsonArray);
          }
          this.employee.dismissOne(this.epBank[1].name);
        }
      )
      .catch(
        () => {
          this.endWaitTip('downloadFail');
          const pageRange: string = this.downloader.beginPosition.toString() + '~' + this.downloader.endPosition.toString();
          const tipsList: Array<string> = ['failPageRange', pageRange];
          this.tips.setCTMoreTip(tipsList);
          this.employee.dismissOne(this.epBank[1].name);
        }
      );
  }

  downloadLoop() {
    if (this.downloader.isSearchEnd()) {
      return;
    }
    if (this.downloader.isSearchError()) {
      for (let i = this.downloader.page; i <= this.downloader.searchTime; i++) {
        this.employee.doTask(this.epBank[1].name, i.toString(), false, null);
      }
      return;
    }
    if (this.selectedOption === 'MNG.searchByCondition') {
      this.usingConditionForm.value.beginPosition = this.downloader.beginPosition;
      this.usingConditionForm.value.endPosition = this.downloader.endPosition;
      this.checkTimeFormat();
      this.cfgService.putMemberSearchCondition(this.usingConditionForm.value).subscribe(
        res => {
          if (res['matchList']) {
            this.downloader.jsonDict[this.downloader.page.toString()] = res.matchList;
            this.employee.doTask(this.epBank[1].name, this.downloader.page.toString(), true, null);
            this.downloader.doneOneSearch();
            this.downloadLoop();
            if (this.downloader.isFirstInFunc) {
              this.downloader.isFirstInFunc = false;
              const msg = this.resError.isErrorCode(res);
              if (msg) {
                this.logger.error(msg, 'downloadLoop:putMemberSearchCondition:');
                this.resError.analyseRes(res);
              }
              this.j2e.preAddTranslate(res.matchList[0]);
            }
          } else {
            this.downloader.searchFail();
            this.downloadLoop();
          }
        },
        err => {
          this.logger.error(err, 'downloadLoop:putMemberSearchCondition:');
          this.downloader.searchFail();
          this.downloadLoop();
        }
      );
    } else if (this.selectedOption === 'MNG.searchByName') {
      this.usingNameForm.value.beginPosition = this.downloader.beginPosition;
      this.usingNameForm.value.endPosition = this.downloader.endPosition;
      this.cfgService.putMemberNameSearch(this.usingNameForm.value).subscribe(
        res => {
          if (res['matchList']) {
            this.downloader.jsonDict[this.downloader.page.toString()] = res.matchList;
            this.employee.doTask(this.epBank[1].name, this.downloader.page.toString(), true, null);
            this.downloader.doneOneSearch();
            this.downloadLoop();
            if (this.downloader.isFirstInFunc) {
              this.downloader.isFirstInFunc = false;
              const msg = this.resError.isErrorCode(res);
              if (msg) {
                this.logger.error(msg, 'downloadLoop:putMemberNameSearch:');
                this.resError.analyseRes(res);
              }
              this.j2e.preAddTranslate(res.matchList[0]);
            }
          } else {
            this.downloader.searchFail();
            this.downloadLoop();
          }
        },
        err => {
          this.logger.error(err, 'downloadLoop:putMemberNameSearch:');
          this.downloader.searchFail();
          this.downloadLoop();
        }
      );
    }
  }

  downloadExcel(colName: string, js: any) {
    this.j2e.push(colName, js);
    this.j2e.getExcel(this.set4Excel.fileName);
    this.endWaitTip('startDownloadWithTip');
  }

  onModify(id: number, order: number) {
    if (!this.modifyOp) {
      return;
    }
    this.addChild.title = 'modifyInfo';
    this.addChild.afterSaveFunc = () => {
      this.searchFunc(this.pageNum);
    };
    this.addChild.onShow(true, id, this.memberList[order]);
    this.clickChild.onNo();
  }

  onDelete(id: number) {
    if (!this.modifyOp) {
      return;
    }
    this.selectedItem = {};
    this.selectedItem[id] = true;
    this.onDeleteComfirm();
  }

  onDeleteTTL() {
    this.tips.showCTip('deleteTTLTip');
    this.ctOb = this.tips.ctAction.subscribe(
      change => {
        if (change === 'onYes') {
          this.showWaitTip();
          this.cfgService.resetFaceMember().subscribe(
            res => {
              const msg = this.resError.isErrorCode(res);
              if (msg) {
                this.logger.error(msg, 'onDeleteTTL:resetFaceMember:');
                this.endWaitTip('deleteFail');
              } else {
                this.endWaitTip('deleteSuccess');
              }
              this.onSearch(1);
            },
            err => {
              this.logger.error(err, 'onDeleteTTL:resetFaceMember:');
              this.endWaitTip('deleteFail');
            }
          );
        } else if (change === 'onNo') {
          this.ctOb.unsubscribe();
          this.ctOb = null;
        }
      }
    );
  }
}
