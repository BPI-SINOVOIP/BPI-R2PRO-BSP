import { Component, OnInit, AfterViewInit, Renderer2, ElementRef, HostListener, Input } from '@angular/core';
import { FormBuilder, FormControl, Validators } from '@angular/forms';

import { ConfigService } from 'src/app/config.service';
import { ControlResultInterface } from './ControlResultInterface';
import { timeOrderValidator } from 'src/app/shared/validators/time-compare.directive';
import { ageOrderValidator } from 'src/app/shared/validators/age-compare.directive';
import { Json2excelService } from 'src/app/face/json2excel.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { TipsService } from 'src/app/tips/tips.service';
import { LockService } from 'src/app/shared/func-service/lock-service.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { BoolEmployee, EmployeeItem } from 'src/app/shared/func-service/employee.service';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { Downloader } from '../shared/downloader';
import { LayoutService } from 'src/app/shared/func-service/layout.service';
import { isNumberJudge } from 'src/app/shared/validators/benumber.directive';
import { isStandardTime } from 'src/app/shared/validators/is-standard-time.directive';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-control',
  templateUrl: './control.component.html',
  styleUrls: ['../shared/face-page.scss']
})
export class ControlComponent implements OnInit, AfterViewInit {

  @Input() options: any;

  constructor(
    private cfgService: ConfigService,
    private Re2: Renderer2,
    private el: ElementRef,
    private fb: FormBuilder,
    private j2e: Json2excelService,
    private resError: ResponseErrorService,
    private tips: TipsService,
    private pfs: PublicFuncService,
    private ieCss: IeCssService,
    private los: LayoutService,
  ) { }

  private logger: Logger = new Logger('control');

  isChrome: boolean = false;
  private employee = new BoolEmployee();
  private ctOb: any;
  private downloader: any;
  private losOb: any;
  modifyOp: boolean = false;
  modeBtnName: string = 'deleteMode';

  searchOption: Array<string> = [
    'MNG.searchByCondition',
    'MNG.searchByName',
  ];

  genderType: Array<string> = [
    'male',
    'female',
    'all',
  ];

  conditionStatus: Array<string> = [
    'open',
    'close',
    'all',
  ];

  epBank: EmployeeItem[] = [
    {
      name: 'download',
      task: [],
    }
  ];

  set4Excel = {
    oneTimeNumber: 1000,
    isFirstSearch: true,
    fileName: 'control',
    column: [
      {
        title: 'id',
        key: 'id',
        type: 'text',
      },
      {
        title: 'sTime',
        key: 'sTime',
        type: 'text',
      },
      {
        title: 'sStatus',
        key: 'sStatus',
        type: 'text',
      },
      {
        title: 'sSimilarity',
        key: 'sSimilarity',
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
        title: 'sRegistrationTime',
        key: 'sRegistrationTime',
        type: 'text',
      },
      {
        title: 'sGender',
        key: 'sGender',
        type: 'text',
      },
      {
        title: 'sBirthday',
        key: 'sBirthday',
        type: 'text',
      },
      {
        title: 'sTelephoneNumber',
        key: 'sTelephoneNumber',
        type: 'text',
      },
      {
        title: 'sType',
        key: 'sType',
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
        title: 'sNote',
        key: 'sNote',
        type: 'text',
      },
    ]
  };

  isDeleteChecked: boolean = false;
  selectedItem: any = {};
  innerNumber: string = '';
  ttlNumber: number = 0;
  private pageNum: number = 0;
  private ttlPage: number = 0;
  innerPage: string = '0 / 0';
  private isViewInit: boolean = false;
  memberList: ControlResultInterface[] = [];
  selectedOption = 'MNG.searchByCondition';

  conditionForm = this.fb.group({
    beginTime: ['', isStandardTime],
    endTime: ['', isStandardTime],
    status: ['all'],
    gender: ['all'],
    minAge: [0, Validators.required],
    maxAge: [100, Validators.required],
    accessCardNumber: [0, [Validators.required, isNumberJudge]],
    beginPosition: [''],
    endPosition: [''],
  }, { validators: [timeOrderValidator('beginTime', 'endTime'), ageOrderValidator('minAge', 'maxAge')]});

  usingconditionForm = this.fb.group({
    beginTime: [''],
    endTime: [''],
    status: ['all'],
    gender: ['all'],
    minAge: [0],
    maxAge: [100],
    accessCardNumber: [0],
    beginPosition: [''],
    endPosition: [''],
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
    this.j2e.addArrayVal(this.genderType);
    this.j2e.addArrayVal(this.conditionStatus);
    // for excel add for word don't be translated
    const addtionList = ['identityCard', 'permanent'];
    this.j2e.addArrayVal(addtionList);

    const secondKey = (this.options && this.options['secondKey']) ? this.options['secondKey'] : 'face';
    const thirdKey = (this.options && this.options['thirdKey']) ? this.options['thirdKey'] : 'Control';
    const fourthKey = (this.options && this.options['fourthKey']) ? this.options['fourthKey'] : 'Control';
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
    this.isViewInit = true;
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

  onSearch(page: number) {
    switch (this.selectedOption) {
      case 'MNG.searchByCondition':
        this.pfs.formatInt(this.conditionForm.value);
        this.usingconditionForm.patchValue(this.conditionForm.value);
        break;
      case 'MNG.searchByName':
        this.pfs.formatInt(this.NameForm.value);
        this.usingNameForm.patchValue(this.NameForm.value);
        break;
    }
    this.searchFunc(page);
    // this.ttlNumber = 0;
    // this.pageNum = 1;
    // this.ttlPage = 0;
  }

  searchFunc(page: number) {
    this.pageNum = page;
    if (this.selectedOption === 'MNG.searchByCondition') {
      this.usingconditionForm.value.beginPosition = (page - 1) * 20;
      if (page !== 1) {
        this.usingconditionForm.value.endPosition = Math.min((page) * 20 - 1, this.ttlNumber);
      } else {
        this.usingconditionForm.value.endPosition = (page) * 20 - 1;
      }
      this.checkTimeFormat();
      this.cfgService.getControlSearchResult(this.usingconditionForm.value).subscribe(
        res => {
          this.resError.analyseRes(res);
          this.memberList = res.matchList;
          // for excel download
          if (res.numOfMatches > 0 && this.set4Excel.isFirstSearch) {
            this.set4Excel.isFirstSearch = false;
            this.j2e.preAddTranslate(this.memberList[0]);
          }
          this.ttlNumber = (res.numOfMatches || 0);
          if (this.ttlNumber === 0) {
            this.pageNum = 0;
          }
          this.setInnerNumber();
        },
        err => {
          this.logger.error(err, 'searchFunc:getControlSearchResult:');
          this.pageNum -= 1;
        }
      );
    } else if (this.selectedOption === 'MNG.searchByName') {
      this.usingNameForm.value.beginPosition = (page - 1) * 20;
      if (page !== 1) {
        this.usingNameForm.value.endPosition = Math.min((page) * 20 - 1, this.ttlNumber);
      } else {
        this.usingNameForm.value.endPosition = (page) * 20 - 1;
      }
      this.cfgService.getControlNameSearchResult(this.usingNameForm.value).subscribe(
        res => {
          this.resError.analyseRes(res);
          this.memberList = res.matchList;
          // for excel download
          if (res.numOfMatches > 0 && this.set4Excel.isFirstSearch) {
            this.set4Excel.isFirstSearch = false;
            this.j2e.preAddTranslate(this.memberList[0]);
          }
          this.ttlNumber = res.numOfMatches;
          if (this.ttlNumber === 0) {
            this.pageNum = 0;
          }
          this.setInnerNumber();
        },
        err => {
          this.logger.error(err, 'searchFunc:getControlNameSearchResult:');
          this.pageNum -= 1;
        }
      );
    }
  }

  checkTimeFormat() {
    const pat = /^.*?-.*?-.*?T.*?:.*?:.*?$/;
    this.usingconditionForm.value.beginTime += (!pat.exec(this.usingconditionForm.value.beginTime) ? ':00' : '');
    this.usingconditionForm.value.endTime += (!pat.exec(this.usingconditionForm.value.endTime) ? ':00' : '');
  }

  downloadJson() {
    this.showWaitTip('prepareForDownloa');
    const oneTimeNumber = this.set4Excel.oneTimeNumber;
    this.downloader = new Downloader(oneTimeNumber, this.ttlNumber);
    this.epBank[0].task = [];
    for (let i = 1; i <= this.downloader.searchTime; i++) {
      this.epBank[0].task.push(i.toString());
    }
    this.employee.hire(this.epBank[0]);
    let jsonArray = [];
    switch (this.selectedOption) {
      case 'MNG.searchByCondition':
        this.usingconditionForm.patchValue(this.conditionForm.value);
        break;
      case 'MNG.searchByName':
        this.usingNameForm.patchValue(this.NameForm.value);
        break;
    }
    this.downloadLoop();
    this.employee.observeTask(this.epBank[0].name, 86400000)
      .then(
        () => {
          if (!this.employee.getWorkResult(this.epBank[0].name)) {
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
          this.employee.dismissOne(this.epBank[0].name);
        }
      )
      .catch(
        () => {
          this.endWaitTip('downloadFail');
          const pageRange: string = this.downloader.beginPosition.toString() + '~' + this.downloader.endPosition.toString();
          const tipsList: Array<string> = ['failPageRange', pageRange];
          this.tips.setCTMoreTip(tipsList);
          this.employee.dismissOne(this.epBank[0].name);
        }
      );
  }

  downloadLoop() {
    if (this.downloader.isSearchEnd()) {
      return;
    }
    if (this.downloader.isSearchError()) {
      for (let i = this.downloader.page; i <= this.downloader.searchTime; i++) {
        this.employee.doTask(this.epBank[0].name, i.toString(), false, null);
      }
      return;
    }
    if (this.selectedOption === 'MNG.searchByCondition') {
      this.usingconditionForm.value.beginPosition = this.downloader.beginPosition;
      this.usingconditionForm.value.endPosition = this.downloader.endPosition;
      this.checkTimeFormat();
      this.cfgService.getControlSearchResult(this.usingconditionForm.value).subscribe(
        res => {
          if (res['matchList']) {
            this.downloader.jsonDict[this.downloader.page.toString()] = res.matchList;
            this.employee.doTask(this.epBank[0].name, this.downloader.page.toString(), true, null);
            this.downloader.doneOneSearch();
            this.downloadLoop();
            if (this.downloader.isFirstInFunc) {
              this.downloader.isFirstInFunc = false;
              this.resError.analyseRes(res);
              this.j2e.preAddTranslate(res.matchList[0]);
            }
          } else {
            this.downloader.searchFail();
            this.downloadLoop();
          }
        },
        err => {
          this.logger.error(err, 'downloadLoop:getControlSearchResult:');
          this.downloader.searchFail();
          this.downloadLoop();
        }
      );
    } else if (this.selectedOption === 'MNG.searchByName') {
      this.usingNameForm.value.beginPosition = this.downloader.beginPosition;
      this.usingNameForm.value.endPosition = this.downloader.endPosition;
      this.cfgService.getControlNameSearchResult(this.usingNameForm.value).subscribe(
        res => {
          if (res['matchList']) {
            this.downloader.jsonDict[this.downloader.page.toString()] = res.matchList;
            this.employee.doTask(this.epBank[0].name, this.downloader.page.toString(), true, null);
            this.downloader.doneOneSearch();
            this.downloadLoop();
            if (this.downloader.isFirstInFunc) {
              this.downloader.isFirstInFunc = false;
              this.resError.analyseRes(res);
              this.j2e.preAddTranslate(res.matchList[0]);
            }
          } else {
            this.downloader.searchFail();
            this.downloadLoop();
          }
        },
        err => {
          this.logger.error(err, 'downloadLoop:getControlNameSearchResult:');
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

  showWaitTip(cTipContent: string) {
    this.tips.showCTip(cTipContent);
    this.tips.setCTPara('hideAll');
    this.tips.setCTPara('quit');
  }

  endWaitTip(tip: string) {
    this.tips.setCTip(tip);
    this.tips.setCTPara('showNo');
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

  onDeleteTTL() {
    this.tips.showCTip('deleteTTLTip');
    this.ctOb = this.tips.ctAction.subscribe(
      change => {
        if (change === 'onYes') {
          this.showWaitTip('waitForDelete');
          this.cfgService.resetFaceControl().subscribe(
            res => {
              this.resError.analyseRes(res, 'deleteFail');
              this.endWaitTip('deleteSuccess');
              if (this.pageNum <= this.ttlPage) {
                this.onSearch(this.pageNum);
              } else {
                this.onSearch(1);
              }
            },
            err => {
              this.logger.error(err, 'onDeleteTTL:resetFaceControl:');
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

  onDblClick(id: number) {
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
  }

  isItemSelected(id: number) {
    if (this.selectedItem[id]) {
      return true;
    } else {
      return false;
    }
  }

  onSelectAll() {
    this.selectedItem = {};
    for (const item of this.memberList) {
      this.selectedItem[item.id] = true;
    }
  }

  onUnselectAll() {
    this.selectedItem = {};
  }

  onDeleteComfirm() {
    this.tips.showCTip('Are you sure that delete info selected?');
    this.ctOb = this.tips.ctAction.subscribe(
      change => {
        if (change === 'onYes') {
          this.showWaitTip('waitForDelete');
          const deleteList = [];
          const deleteObj = {
            type: 'control',
            name: []
          };
          for (const item of this.memberList) {
            if (this.selectedItem[item.id]) {
              deleteList.push(item.id.toString());
              deleteObj.name.push(this.pfs.getNameFromPath(item.sSnapshotName.toString()));
            }
          }
          if (deleteList.length > 0) {
            this.cfgService.deleteControlList(deleteList).subscribe(
              res => {
                this.resError.analyseRes(res);
                this.cfgService.deleteReview(deleteObj).subscribe(
                  dat => {
                    this.selectedItem = {};
                    this.endWaitTip('deleteSuccess');
                    this.onSearch(this.pageNum);
                  },
                  err => {
                    this.selectedItem = {};
                    this.endWaitTip('deleteFail');
                  }
                );
              },
              err => {
                this.selectedItem = {};
                this.endWaitTip('deleteFail');
              }
            );
          } else {
            this.endWaitTip('nothing2delete');
          }
        } else if (change === 'onNo') {
          this.ctOb.unsubscribe();
          this.ctOb = null;
        }
      }
    );
  }
}
