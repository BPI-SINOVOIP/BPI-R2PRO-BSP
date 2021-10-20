import { Component, OnInit, OnDestroy, ViewChild, ElementRef, Renderer2, ChangeDetectorRef } from '@angular/core';
import { TipsService } from '../tips.service';
import { ConfigService } from 'src/app/config.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-center-tips',
  templateUrl: './center-tips.component.html',
  styleUrls: ['./center-tips.component.scss']
})
export class CenterTipsComponent implements OnInit, OnDestroy {

  constructor(
    private Re2: Renderer2,
    private el: ElementRef,
    private tips: TipsService,
    private cfg: ConfigService,
    private pfs: PublicFuncService,
    private cdr: ChangeDetectorRef,
  ) { }

  @ViewChild('modal', {read: ElementRef})
  modalChild: ElementRef;

  private logger: Logger = new Logger('hard-disk-management');
  objectKey = this.pfs.objectKeys;
  tipsList: Array<string> = [];
  centerTip: string;
  tipYes: string = 'yes';
  tipNo: string = 'no';
  private tipContentOb: any;
  private tipParaOb: any;
  tableObj = {
    isShowTable: false,
    table: [],
    init: () => {
      this.tableObj.isShowTable = false;
      this.tableObj.table = [];
    }
  };
  processObj = {
    isShow: false,
    ttl: 0,
    now: 0,
    per: 0,
    uninit: () => {
      this.processObj.isShow = false;
      this.processObj.ttl = 0;
      this.processObj.now = 0;
      this.processObj.per = 0;
    },
    init: (ttl: number) => {
      this.processObj.isShow = true;
      this.processObj.ttl = ttl;
      this.processObj.now = 0;
      this.processObj.per = 0;
    },
    set: (now: number) => {
      this.processObj.now = (now > this.processObj.ttl) ? this.processObj.ttl : now;
      this.processObj.per = Math.ceil((this.processObj.now / this.processObj.ttl) * 100);
    }
  };
  isClickTip: boolean = false;

  ngOnInit(): void {
    this.tipContentOb = this.tips.cContent.subscribe(
      change => {
        this.centerTip = change;
      }
    );
    this.tipParaOb = this.tips.ctPara.subscribe(
      change => {
        if (change.match(':')) {
          const jsObj = JSON.parse(change);
          if (jsObj['table']) {
            this.tableObj.isShowTable = true;
            this.tableObj.table = jsObj['table'];
            this.onShow();
            this.hideYes();
            this.tipNo = 'quit';
            this.cdr.markForCheck();
            this.cdr.detectChanges();
          } else if (jsObj['tips']) {
            this.tipsList = jsObj['tips'];
          } else if (jsObj['ttlProcess']) {
            this.processObj.init(jsObj['ttlProcess']);
            this.pfs.waitAInit(5000, '.process-left.mt-2', this.el)
              .then(
                () => {
                  this.setPercentage(0);
                }
              )
              .catch();
          } else if (jsObj['nowProcess']) {
            this.processObj.set(jsObj['nowProcess']);
            this.setPercentage(this.processObj.per);
          } else if (jsObj['clickTable']) {
            this.tableObj.table = jsObj['clickTable'];
            this.isClickTip = true;
            this.tipNo = 'quit';
            this.hideAll();
            this.showNo();
          }
        } else {
          switch (change) {
            case 'init':
              this.onShow();
              this.showAll();
              this.tipNo = 'no';
              this.tableObj.init();
              break;
            case 'showAll':
              this.showAll();
              break;
            case 'hideAll':
              this.hideAll();
              break;
            case 'hideYes':
              this.hideYes();
              break;
            case 'showYes':
              this.showYes();
              break;
            case 'hideNo':
              this.hideNo();
              this.hideX();
              break;
            case 'showNo':
              this.showNo();
              this.showX();
              break;
            case 'quit':
              this.tipNo = 'quit';
              break;
            case 'no':
              this.tipNo = 'no';
              break;
            case 'restart':
              this.onRestart();
              break;
            case 'reset':
              this.onReset();
              break;
            case 'waitForComplete':
              const that = this;
              setTimeout(that.waitForComplete, 2000);
              break;
            case 'close':
              this.onNo();
              break;
            case 'oneQuie':
              this.hideYes();
              this.tipNo = 'quit';
              break;
            case 'closeProcess':
              this.processObj.isShow = false;
              break;
          }
        }
      }
    );
  }

  ngOnDestroy(): void {
    this.tipContentOb.unsubscribe();
    this.tipParaOb.unsubscribe();
  }

  onRestart() {
    this.hideAll();
    this.tipNo = 'quit';
    this.centerTip = 'deviceRestarting';
    const that = this;
    this.cfg.putRebootSignal().subscribe(
      res => {
        setTimeout(that.waitForComplete, 15000);
      },
      err => {
        setTimeout(that.waitForComplete, 15000);
      }
    );
  }

  onSimpleReset = () => {
    const that = this;
    this.cfg.putFactoryResetSignal().subscribe(
      res => {
        setTimeout(that.waitForComplete, 2000);
      },
      err => {
        this.logger.error(err, 'onSimpleReset:putFactoryResetSignal:');
        setTimeout(that.waitForComplete, 2000);
      }
    );
  }

  getHddNum = (hddList: any): number => {
    if (hddList['error']) {
      this.logger.error(hddList, 'getHddNum:');
      return 0;
    }
    if (hddList instanceof Array) {
      let cnt = 0;
      for (const item of hddList) {
        if (item['sStatus'] && item['sStatus'] === 'mounted') {
          cnt += 1;
        }
      }
      return cnt;
    } else {
      return 0;
    }
  }

  waitHDDList = (target: number, waitTime: number) => {
    this.logger.debug('waitHDDList, target:' + target + ',wait:' + waitTime);
    if (waitTime <= 0) {
      this.onSimpleReset();
    } else if (target <= 0) {
      setTimeout(this.onSimpleReset, 2000);
    }
    this.cfg.getHardDiskManagementInterface().subscribe(
      res => {
        let cnt = this.getHddNum(res);
        if (cnt >= target) {
          if (waitTime >= 19) {
            setTimeout(this.onSimpleReset, 1000);
          } else {
            this.onSimpleReset();
          }
        } else {
          setTimeout(
            () => {
              this.waitHDDList(target, waitTime - 1);
            },
            1000
          );
        }
      },
      err => {
        if (waitTime >= 19) {
          setTimeout(this.onSimpleReset, 1000);
        } else {
          this.onSimpleReset();
        }
        this.logger.error(err, 'waitHDDList:getHardDiskManagementInterface:');
      }
    );
  }

  preReset = (target: number, waitTime: number) => {
    this.cfg.putPreFactoryResetSignal().subscribe(
      dat => {
        if (target <= 0) {
          this.onSimpleReset();
        } else {
          this.waitHDDList(target, waitTime);
        }
      },
      err => {
        this.logger.error(err, 'waitHDDList:putPreFactoryResetSignal:');
        this.onSimpleReset();
      }
    );
  }

  onReset() {
    this.hideAll();
    this.tipNo = 'quit';
    this.centerTip = 'UPGRADE.reseting';
    this.cfg.getHardDiskManagementInterface().subscribe(
      res => {
        const hddNum = this.getHddNum(res);
        this.preReset(hddNum, 20);
      },
      err => {
        this.logger.error(err, 'onReset:getHardDiskManagementInterface:');
        this.preReset(0, 0);
      }
    );
  }

  waitForComplete = () => {
    this.cfg.getFreeRoom().subscribe(
      res => {
        this.centerTip = 'completeTip';
        this.showNo();
        this.showX();
      },
      err => {
        this.waitForComplete();
      }
    );
  }

  onShow() {
    this.tipsList = [];
    this.Re2.setStyle(this.modalChild.nativeElement.querySelector('.modal'), 'display', 'block');
  }

  onYes() {
    this.tips.setCTAction('onYes');
  }

  onNo() {
    this.Re2.setStyle(this.modalChild.nativeElement.querySelector('.modal'), 'display', 'none');
    if (this.tableObj.isShowTable) {
      this.closeTable();
    }
    if (this.processObj.isShow) {
      this.processObj.uninit();
    }
    this.isClickTip = false;
    this.tips.setCTAction('onNo');
  }

  hideYes() {
    this.Re2.setStyle(this.el.nativeElement.querySelector('.red-btn'), 'display', 'none');
  }

  showYes() {
    this.Re2.setStyle(this.el.nativeElement.querySelector('.red-btn'), 'display', 'block');
  }

  hideNo() {
    this.Re2.setStyle(this.el.nativeElement.querySelector('.blue-btn'), 'display', 'none');
  }

  showNo() {
    this.Re2.setStyle(this.el.nativeElement.querySelector('.blue-btn'), 'display', 'block');
  }

  hideX() {
    this.Re2.setStyle(this.el.nativeElement.querySelector('.X'), 'display', 'none');
  }

  showX() {
    this.Re2.setStyle(this.el.nativeElement.querySelector('.X'), 'display', 'block');
  }

  showAll() {
    this.showYes();
    this.showNo();
    this.showX();
  }

  hideAll() {
    this.hideYes();
    this.hideNo();
    this.hideX();
  }

  showTable(table: Array<object>) {
    this.tableObj.isShowTable = true;
    this.tableObj.table = table;
    this.onShow();
    this.hideYes();
    this.tipNo = 'quit';
  }

  closeTable() {
    this.tableObj.isShowTable = false;
    this.showYes();
    this.tipNo = 'no';
  }

  setPercentage(percent: number) {
    this.Re2.setStyle(this.el.nativeElement.querySelector('.process-left.mt-2'), 'width', percent + '%');
    const left = 100 - percent;
    this.Re2.setStyle(this.el.nativeElement.querySelector('.process-right.mt-2'), 'width', left + '%');
  }

  onDetail() {
    this.isClickTip = false;
    this.tipsList = [];
    this.centerTip = '';
    this.tableObj.isShowTable = true;
  }
}
