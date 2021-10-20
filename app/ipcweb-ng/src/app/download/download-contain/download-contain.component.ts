import { Component, OnInit, Renderer2, ElementRef, Input, ViewChild, AfterViewInit } from '@angular/core';
import { FormBuilder, Validators, FormControl } from '@angular/forms';

import { ConfigService } from 'src/app/config.service';
import { DownloadListInterface, MatchList } from './DownloadInterface';
import { timeOrderValidator } from 'src/app/shared/validators/time-compare.directive';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { TipsService } from 'src/app/tips/tips.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { DiyHttpService } from 'src/app/shared/func-service/diy-http.service';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { LayoutService } from 'src/app/shared/func-service/layout.service';
import { LockerService } from 'src/app/shared/func-service/lock-service.service';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-download-contain',
  templateUrl: './download-contain.component.html',
  styleUrls: ['./download-contain.component.scss']
})
export class DownloadContainComponent implements OnInit, AfterViewInit {

  constructor(
    private fb: FormBuilder,
    private cfg: ConfigService,
    private Re2: Renderer2,
    private el: ElementRef,
    private resError: ResponseErrorService,
    private tips: TipsService,
    private pfs: PublicFuncService,
    private dhs: DiyHttpService,
    private ics: IeCssService,
    private los: LayoutService,
  ) { }

  @Input() pageType: any;

  private logger: Logger;
  private locker: LockerService;

  deleteType: string;
  isChrome: boolean = false;
  isIE: boolean = false;
  selectAll: boolean = false;
  innerNumber = '共 0 条 当前 0 条';
  innerPage = '0/0';
  previewUrl = '';
  nowPage: number;
  private ttlPage: number;
  private fileList: Array<number> = [];
  nowNavOb: any;

  searchTypeDict: any = {
    videoRecord: ['video0', 'video1'],
    pictureRecord: ['photo0', 'photo1']
  };

  SearchForm = this.fb.group({
    searchType: 'video1',
    startTime: ['', Validators.required],
    endTime: ['', Validators.required],
    maxResults: 20,
    searchResultPosition: 0,
    order: [0, Validators.required],
  }, {validators: [timeOrderValidator('startTime', 'endTime')]});

  downloadList: MatchList[] = [];

  get startTime(): FormControl {
    return this.SearchForm.get('startTime') as FormControl;
  }
  get endTime(): FormControl {
    return this.SearchForm.get('endTime') as FormControl;
  }
  get order(): FormControl {
    return this.SearchForm.get('order') as FormControl;
  }

  ngOnInit(): void {
    if (this.pageType['type']) {
      this.logger = new Logger(this.pageType.type);
    } else {
      this.logger = new Logger('download-contain');
    }
    this.locker = new LockerService(this.tips, this.logger);
    this.isIE = this.ics.getIEBool();
    this.isChrome = this.ics.getChromeBool();
    this.timeInit();
    if (this.pageType) {
      this.SearchForm.get('searchType').setValue(this.searchTypeDict[this.pageType.type][0]);
    }
    this.onInquire(1);
  }

  ngAfterViewInit(): void {
    if (this.pageType.type !== 'videoRecord' || this.isIE) {
      this.Re2.setStyle(this.el.nativeElement.querySelector('.preview'), 'display', 'none');
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
    // this.SearchForm.get('startTime').setValue(year + '-' + month + '-' + day + 'T00:00');
    this.SearchForm.get('startTime').setValue('1970-01-01T00:00:00');
    this.SearchForm.get('endTime').setValue(year + '-' + month + '-' + day + 'T23:59:59');
  }

  onInquire(pageNumber: number) {
    this.inquireBody(pageNumber);
  }

  inquireBody = (pageNumber: number) => {
    this.SearchForm.value.order = Number(this.SearchForm.value.order);
    this.SearchForm.value.searchResultPosition = (pageNumber - 1) * this.SearchForm.value.maxResults;
    this.deleteType = this.SearchForm.value.searchType;
    this.cfg.getDownloadList(this.SearchForm.value).subscribe(
      (res: DownloadListInterface) => {
        this.resError.analyseRes(res);
        this.downloadList = res.matchList;
        if (this.downloadList) {
          const rLen = this.downloadList.length;
          this.selectAll = false;
          for (let i = 0; i < rLen; i++) {
            this.downloadList[i].fileSize = Math.ceil(this.downloadList[i].fileSize * 100) / 100;
            this.downloadList[i].isChecked = false;
          }
          this.ttlPage = Math.ceil(res.numOfMatches / this.SearchForm.value.maxResults);
          this.innerPage = pageNumber + '/' + this.ttlPage;
          this.innerNumber = '总共 ' + res.numOfMatches + '条 当前 ' + this.downloadList.length + ' 条';
          this.nowPage = pageNumber;
        } else {
          this.selectAll = false;
          this.ttlPage = 0;
          this.innerPage = '0/0';
          this.innerNumber = '总共 ' + 0 + '条 当前 ' + 0 + ' 条';
          this.pfs.waitNavActive(20000, this.pageType.type + 'Tab')
            .then(
              () => {
                this.tips.setRbTip('noResult');
              }
            )
            .catch(
              (error) => {
                this.logger.error(error, 'inquireBody:getDownloadList:waitNavActive:Wait Show noResult Fail:');
              }
            );
        }
      },
      err => {
        this.logger.error(err, 'inquireBody:getDownloadList:');
      }
    );
  }

  onSelectAll() {
    this.selectAll = !this.selectAll;
    let len = 0;
    if (this.downloadList) {
      len = this.downloadList.length;
    }
    for (let i = 0; i < len; i++) {
      this.downloadList[i].isChecked = this.selectAll;
    }
  }

  onDownload() {
    if (!this.downloadList) {
      return;
    }
    const len = this.downloadList.length;
    if (this.isIE) {
      if (this.pageType.type === 'pictureRecord') {
        for (let i = len - 1; i >= 0; i--) {
          if (this.downloadList[i].isChecked) {
            this.dhs.downloadPic4IE(this.downloadList[i].fileAddress, this.downloadList[i].fileName);
          }
        }
      } else {
        for (let i = len - 1; i >= 0; i--) {
          if (this.downloadList[i].isChecked) {
            this.ieDownloader(i);
          }
        }
      }
    } else {
      for (let i = len - 1; i >= 0; i--) {
        if (this.downloadList[i].isChecked) {
          this.fileList.push(i);
        }
      }
      if (this.fileList.length > 0) {
        this.downloadEvent();
      }
    }
  }

  downloadEvent() {
    const index = this.fileList.pop();
    if (this.isChrome && this.pageType.type === 'pictureRecord') {
      this.dhs.downloadPic4Chrome(this.downloadList[index].fileName, this.downloadList[index].fileAddress);
    } else {
      this.dhs.aDownloadFunc(this.downloadList[index].fileName, this.downloadList[index].fileAddress);
    }
    if (this.fileList.length > 0) {
      const that = this;
      setTimeout(() => {
        that.downloadEvent();
      }, 200);
    }
  }

  onSelectFile(index: number) {
    if (this.isIE) {
      return;
    }
    this.Re2.setAttribute(this.el.nativeElement.querySelector('.preview-iframe'), 'src', this.downloadList[index].fileAddress + '?view');
    this.previewUrl = this.downloadList[index].fileAddress;
  }

  ieDownloader(index: number) {
    this.Re2.setAttribute(this.el.nativeElement.querySelector('.preview-iframe'), 'src', this.downloadList[index].fileAddress);
    this.previewUrl = this.downloadList[index].fileAddress;
  }

  pageUp() {
    if (this.nowPage > 1) {
      this.inquireBody(this.nowPage - 1);
    }
  }

  pageDoubleUp() {
    if (this.nowPage !== 1) {
      this.inquireBody(1);
    }
  }

  pageDown() {
    if (this.nowPage < this.ttlPage) {
      this.inquireBody(this.nowPage + 1);
    }
  }

  pageDoubleDown() {
    if (this.nowPage !== this.ttlPage) {
      this.inquireBody(this.ttlPage);
    }
  }

  onDelete() {
    if (!this.downloadList) {
      return;
    }
    const len = this.downloadList.length;
    const deleteList = [];
    for (let i = len - 1; i >= 0; i--) {
      if (this.downloadList[i].isChecked) {
        deleteList.push(this.downloadList[i].fileName);
      }
    }
    if (deleteList.length <= 0) {
      return;
    }
    if ((this.locker && this.locker.getLocker('onDelete', 'waitForDelete', true))) {
      return;
    }
    const data = {
      type: this.deleteType,
      name: deleteList
    };
    this.cfg.deleteReview(data).subscribe(
      res => {
        const msg = this.resError.isErrorCode(res);
        if (msg) {
          this.logger.error(msg, 'onDelete:');
          this.inquireBody(this.nowPage);
          this.tips.setRbTip('deleteFail');
        } else {
          let page = 1;
          if (this.nowPage <= this.ttlPage) {
            page = this.nowPage;
          } else if (this.ttlPage > 1) {
            page = this.ttlPage - 1;
          }
          this.waitInquireBody(page, 100);
        }
      },
      err => {
        this.logger.error(err, 'onDelete:deleteReview:');
        if ((this.locker && this.locker.isLock('onDelete'))) {
          this.locker.releaseLocker('onDelete');
        }
      }
    );
  }

  waitInquireBody(page: number, wait: number) {
    const that = this;
    setTimeout(() => {
      that.inquireBody(page);
      that.tips.setRbTip('deleteSuccess');
      if ((that.locker && that.locker.isLock('onDelete'))) {
        that.locker.releaseLocker('onDelete');
      }
    }, wait);
  }
}
