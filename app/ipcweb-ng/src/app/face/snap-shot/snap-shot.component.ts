import { Component, OnInit, AfterViewInit, Renderer2, ElementRef, HostListener, ViewChild, Input, OnDestroy } from '@angular/core';
import { FormBuilder, Validators, FormControl } from '@angular/forms';

import { SnapSearch, SnapListInterface } from './SnapListInterface';
import { ConfigService } from 'src/app/config.service';
import { timeOrderValidator } from 'src/app/shared/validators/time-compare.directive';
import { Json2excelService } from 'src/app/face/json2excel.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { TipsService } from 'src/app/tips/tips.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { BoolEmployee, EmployeeItem } from 'src/app/shared/func-service/employee.service';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { Downloader } from '../shared/downloader';
import { LayoutService } from 'src/app/shared/func-service/layout.service';
import { isStandardTime } from 'src/app/shared/validators/is-standard-time.directive';

import Logger from 'src/app/logger';

@Component({
  selector: 'app-snap-shot',
  templateUrl: './snap-shot.component.html',
  styleUrls: ['../shared/face-page.scss']
})
export class SnapShotComponent implements OnInit, AfterViewInit, OnDestroy {

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

  @ViewChild('add', {static: true}) addChild: any;
  @ViewChild('add', {read: ElementRef}) addView: ElementRef;
  private logger: Logger = new Logger('snap-shot');

  isChrome: boolean = false;
  private employee = new BoolEmployee();
  private ctOb: any;
  private downloader: any;

  isDeleteChecked: boolean = false;
  selectedItem: any = {};
  innerNumber: string = '';
  ttlNumber: number = 0;
  private pageNum: number = 0;
  private ttlPage: number = 0;
  innerPage: string = '0 / 0';
  private isViewInit: boolean = false;
  snapList: SnapListInterface[] = [];
  private losOb: any;
  modifyOp: boolean = false;
  modeBtnName: string = 'deleteMode';

  epBank: EmployeeItem[] = [
    {
      name: 'download',
      task: [],
    }
  ];

  set4Excel = {
    oneTimeNumber: 1000,
    isFirstSearch: true,
    fileName: 'snapshot',
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
        title: 'sNote',
        key: 'sNote',
        type: 'text',
      },
    ]
  };

  SnapForm = this.fb.group({
    beginTime: ['', isStandardTime],
    endTime: ['', isStandardTime],
    beginPosition: [''],
    endPosition: [''],
  }, {validators: timeOrderValidator('beginTime', 'endTime')});

  usingSnapForm = this.fb.group({
    beginTime: [''],
    endTime: [''],
    beginPosition: [''],
    endPosition: [''],
  });

  get beginTime(): FormControl {
    return this.SnapForm.get('beginTime') as FormControl;
  }

  get endTime(): FormControl {
    return this.SnapForm.get('endTime') as FormControl;
  }

  ngOnInit(): void {
    this.timeInit();
    this.onSearch(1);
    // for excel download
    this.j2e.initService();
    this.j2e.set4Column(this.set4Excel.fileName, this.set4Excel.column);
    const addtionList = ['open', 'close', 'Processed', 'Snapshot'];
    this.j2e.addArrayVal(addtionList);

    const secondKey = (this.options && this.options['secondKey']) ? this.options['secondKey'] : 'face';
    const thirdKey = (this.options && this.options['thirdKey']) ? this.options['thirdKey'] : 'SnapShot';
    const fourthKey = (this.options && this.options['fourthKey']) ? this.options['fourthKey'] : 'SnapShot';
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

  ngAfterViewInit(): void {
    this.resize();
    const width = document.getElementById('table-part').clientWidth;
    this.Re2.setStyle(this.el.nativeElement.querySelector('.page-foot'), 'width', width + 'px');
    this.addChild.isListen = false;
    this.addChild.isPutAddress = true;
    this.Re2.setAttribute(this.addView.nativeElement.querySelector('.put-img'), 'hidden', 'true');
    this.isViewInit = true;
  }

  ngOnDestroy() {
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
    // this.SnapForm.get('beginTime').setValue(year + '-' + month + '-' + day + 'T00:00:01');
    this.SnapForm.get('beginTime').setValue('1970-01-01T00:00:00');
    this.SnapForm.get('endTime').setValue(year + '-' + month + '-' + day + 'T23:59:59');
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

  setInnerNumber() {
    let nowNumber = 0;
    if (this.snapList) {
      nowNumber = this.snapList.length;
    }
    if (nowNumber <= 0) {
      this.tips.setRbTip('noResult');
    }
    this.innerNumber = '总共 ' + this.ttlNumber + ' 条 当前 ' + nowNumber + '条';
    this.ttlPage = Math.ceil(this.ttlNumber / 20);
    this.innerPage = this.pageNum + '/' + this.ttlPage;
  }

  onSearch(page: number) {
    this.pfs.formatInt(this.SnapForm.value);
    this.usingSnapForm.patchValue(this.SnapForm.value);
    this.searchFunc(page);
    this.ttlNumber = 0;
    this.pageNum = 1;
    this.ttlPage = 0;
  }

  searchFunc(page: number) {
    this.pageNum = page;
    this.usingSnapForm.value.beginPosition = (page - 1) * 20;
    if (page !== 1) {
      this.usingSnapForm.value.endPosition = Math.min((page) * 20 - 1, this.ttlNumber);
    } else {
      this.usingSnapForm.value.endPosition = (page) * 20 - 1;
    }
    this.checkTimeFormat();
    this.cfgService.putSnapSearch(this.usingSnapForm.value).subscribe(
      res => {
        this.resError.analyseRes(res);
        this.snapList = res.matchList;
        // for excel download
        if (res.numOfMatches > 0 && this.set4Excel.isFirstSearch) {
          this.set4Excel.isFirstSearch = false;
          this.j2e.preAddTranslate(this.snapList[0]);
        }
        this.ttlNumber = res.numOfMatches;
        if (this.ttlNumber === 0) {
          this.pageNum = 0;
        }
        this.setInnerNumber();
      },
      err => {
        this.logger.error(err, 'searchFunc:putSnapSearch:');
        this.pageNum -= 1;
      }
    );
  }

  onDblClick(id: number, order: number) {
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

  checkTimeFormat() {
    const pat = /^.*?-.*?-.*?T.*?:.*?:.*?$/;
    this.usingSnapForm.value.beginTime += (!pat.exec(this.usingSnapForm.value.beginTime) ? ':00' : '');
    this.usingSnapForm.value.endTime += (!pat.exec(this.usingSnapForm.value.endTime) ? ':00' : '');
  }

  downloadJson() {
    this.showWaitTip('prepareForDownloa');
    const oneTimeNumber = this.set4Excel.oneTimeNumber;
    // const oneTimeNumber = 2;
    this.downloader = new Downloader(oneTimeNumber, this.ttlNumber);
    this.epBank[0].task = [];
    for (let i = 1; i <= this.downloader.searchTime; i++) {
      this.epBank[0].task.push(i.toString());
    }
    this.employee.hire(this.epBank[0]);
    let jsonArray = [];
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
    this.usingSnapForm.value.beginPosition = this.downloader.beginPosition;
    this.usingSnapForm.value.endPosition = this.downloader.endPosition;

    this.checkTimeFormat();
    this.cfgService.putSnapSearch(this.usingSnapForm.value).subscribe(
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
        this.logger.error(err, 'downloadLoop:putSnapSearch:');
        this.downloader.searchFail();
        this.downloadLoop();
      }
    );
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

  isItemSelected(id: number) {
    if (this.selectedItem[id]) {
      return true;
    } else {
      return false;
    }
  }

  onSelectAll() {
    this.selectedItem = {};
    for (const item of this.snapList) {
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
            type: 'snapshot',
            name: []
          };
          for (const item of this.snapList) {
            if (this.selectedItem[item.id]) {
              deleteList.push(item.id.toString());
              deleteObj.name.push(this.pfs.getNameFromPath(item.sSnapshotName.toString()));
            }
          }
          if (deleteList.length > 0) {
            this.cfgService.deleteSnapList(deleteList).subscribe(
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

  onDeleteTTL() {
    this.tips.showCTip('deleteTTLTip');
    this.ctOb = this.tips.ctAction.subscribe(
      change => {
        if (change === 'onYes') {
          this.showWaitTip('waitForDelete');
          this.cfgService.resetFaceSnapshot().subscribe(
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
              this.logger.error(err, 'onDeleteTTL:resetFaceSnapshot:');
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

  onModify(order: number) {
    this.addChild.title = 'addInfo';
    this.addChild.afterSaveFunc = () => {
      this.searchFunc(this.pageNum);
    };
    this.addChild.onShow(true, '', {sPicturePath: this.snapList[order].sPicturePath});
  }

  onDelete(id: number) {
    this.tips.showCTip('Are you sure that delete info selected?');
    this.ctOb = this.tips.ctAction.subscribe(
      change => {
        if (change === 'onYes') {
          this.showWaitTip('waitForDelete');
          const deleteList = [];
          const deleteObj = {
            type: 'snapshot',
            name: []
          };
          for (const item of this.snapList) {
            if (item.id === id) {
              deleteList.push(item.id.toString());
              deleteObj.name.push(this.pfs.getNameFromPath(item.sSnapshotName.toString()));
              break;
            }
          }
          if (deleteList.length > 0) {
            this.cfgService.deleteSnapList(deleteList).subscribe(
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
