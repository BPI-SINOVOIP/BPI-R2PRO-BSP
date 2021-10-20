import { Component, OnInit, OnDestroy, Renderer2, ElementRef, ViewChild } from '@angular/core';

import { ConfigService } from 'src/app/config.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { DiyHttpService, HttpInfo } from 'src/app/shared/func-service/diy-http.service';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { TipsService } from 'src/app/tips/tips.service';
import { LockService } from 'src/app/shared/func-service/lock-service.service';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-upgrade',
  templateUrl: './upgrade.component.html',
  styleUrls: ['./upgrade.component.scss']
})
export class UpgradeComponent implements OnInit, OnDestroy {

  modalChild: ElementRef;

  constructor(
    private cfg: ConfigService,
    private Re2: Renderer2,
    private el: ElementRef,
    private resError: ResponseErrorService,
    private pb: PublicFuncService,
    private diyHttp: DiyHttpService,
    private ieCss: IeCssService,
    private tips: TipsService,
  ) { }

  lock = new LockService(this.tips);
  private ctOb: any;
  upgradeFile: {
    targetSize: number;
    cursorSize: number;
    perSize: number;
    file: any;
    type: any;
    blobFile: any;
    upgradeId: string;
    idPat: any;
    processPat: any;
    upgradeMsg: string;
    types: Array<string>;
  } = {
    targetSize: 0,
    cursorSize: 0,
    perSize: 512 * 1024,
    file: null,
    type: 'text/plain',
    blobFile: [''],
    upgradeId: '',
    idPat: /id=(.*?)$/,
    processPat: /bytes 0-(.*?)$/,
    upgradeMsg: '',
    types: ['img', 'IMG'],
  };

  InfoFile: {
    file: any;
    type: string;
    size: number;
    types: Array<string>;
  } = {
    file: null,
    type: 'text/plain',
    size: 0,
    types: ['db', 'DB'],
  };

  confirmTip: string = '';
  private logger: Logger = new Logger('upgrade');
  private ctObserver: any;
  private tipType: string;
  upgradeOverSize: boolean = false;
  infoOverSize: boolean = false;
  infoTypeError: boolean = false;
  upgradeTypeError: boolean = false;
  upgradeFileName: string = 'noFileSelected';
  uploadFileName: string = 'noFileSelected';
  isIe: boolean = true;

  ngOnInit(): void {
    this.isIe = this.ieCss.getIEBool();
  }

  ngOnDestroy() {
    if (this.lock.checkLock('ctOb')) {
      this.lock.unlock('ctOb');
    }
    if (this.ctOb) {
      this.ctOb.unsubscribe();
    }
  }

  onUpgradeFileChange(file: File[]) {
    this.initProcess('for-upgrade');
    if (!this.pb.checkFileType(file[0].name, this.upgradeFile.types)) {
      this.upgradeFileName = 'noFileSelected';
      this.upgradeFile.file = null;
      this.upgradeTypeError = true;
      return;
    } else {
      this.upgradeTypeError = false;
    }
    this.upgradeFile.file = file[0];
    this.upgradeFile.targetSize = file[0].size;
    this.upgradeFileName = file[0].name;
    this.upgradeFile.cursorSize = 0;
    this.cfg.getFreeRoom().subscribe(
      res => {
        this.resError.analyseRes(res);
        if (this.upgradeFile.targetSize > res.availableDisk) {
          this.upgradeOverSize = true;
        } else {
          this.upgradeOverSize = false;
        }
      });
  }

  onInfoFileChange(file: File[]) {
    this.initProcess('for-upload');
    if (this.pb.checkFileType(file[0].name, ['db', 'DB'])) {
      this.InfoFile.file = file[0];
      this.InfoFile.size = file[0].size;
      this.uploadFileName = file[0].name;
      this.infoTypeError = false;
      this.cfg.getFreeRoom().subscribe(
        res => {
          this.resError.analyseRes(res);
          if (this.InfoFile.size > res.availableDisk) {
            this.infoOverSize = true;
          } else {
            this.infoOverSize = false;
          }
        }
      );
    } else {
      this.infoTypeError = true;
    }
  }

  onUpgrade() {
    this.lock.lock('onUpgrade', true);
    if (this.upgradeFile.file) {
      if (this.upgradeOverSize) {
        this.tips.setRbTip('UPGRADE.oversize');
      } else {
        this.cfg.getUpgradeNum().subscribe(
          res => {
            this.upgradeFile.upgradeId = this.upgradeFile.idPat.exec(res.headers.get('X-Location'))[1];
            const httpInfo: HttpInfo = {
              targetSize: this.upgradeFile.targetSize,
              cursorSize: this.upgradeFile.cursorSize,
              perSize: this.upgradeFile.perSize,
              file: this.upgradeFile.file,
              type: this.upgradeFile.type,
              blobFile: this.upgradeFile.blobFile,
              upgradeId: this.upgradeFile.upgradeId,
              processPat: this.upgradeFile.processPat,
              upgradeMsg: this.upgradeFile.upgradeMsg,
              httpFunc: 'putUpgradeInfo',
              processFunc: 'putUpgradeInfo',
            };
            const ob = this.diyHttp.diyHttp(httpInfo).subscribe(
              (rst: number) => {
                if (rst > 100) {
                  this.setPercentage('for-upgrade', 100);
                  this.sendUpgradeSignal();
                  ob.unsubscribe();
                  this.lock.unlock('onUpgrade');
                } else if (rst === -1) {
                  this.setProcessRed('for-upgrade');
                  this.tips.setRbTip('uploadFail');
                  ob.unsubscribe();
                  this.lock.unlock('onUpgrade');
                } else {
                  this.setProcessBlue('for-upgrade');
                  this.setPercentage('for-upgrade', rst);
                }
              }
            );
          },
          err => {
            this.lock.unlock('onUpgrade');
            this.logger.error(err, 'onUpgrade:getUpgradeNum:');
          }
        );
      }
    } else {
      this.tips.setRbTip('UPGRADE.putFile');
    }
  }

  sendUpgradeSignal() {
    this.upgradeFile.file = null;
    this.upgradeFileName = 'noFileSelected';
    this.cfg.putUpgradeEndSignal(this.upgradeFile.upgradeId).subscribe(
      res => {
        this.tips.showCTip('UPGRADE.upgrading');
        this.tips.setCTPara('hideAll');
        this.tips.setCTPara('quit');
        this.tips.setCTPara('waitForComplete');
      },
      err => {
        this.tips.showCTip('UPGRADE.upgrading');
        this.tips.setCTPara('hideAll');
        this.tips.setCTPara('quit');
        this.tips.setCTPara('waitForComplete');
      }
    );
  }

  onDownload(downloadName: string) {
    if (downloadName === 'deviceInfo') {
      this.cfg.getSysDbUrl().subscribe(
        res => {
          const url = res['location'];
          this.downloadFunc(url, '');
        }
      );
    } else if (downloadName === 'diagnosticInfo') {
      this.cfg.getSysLogUrl().subscribe(
        res => {
          const url = res['location'];
          this.downloadFunc(url, '');
        }
      );
    }
  }

  downloadFunc(url: string, name: string) {
    if (!(name)) {
      const len = url.length;
      for (let i = len - 1; i >= 0; i--) {
        if (url[i] === '/') {
          name = url.slice(i + 1, len);
          break;
        }
      }
    }
    // this.download4IE(url);
    if (this.isIe) {
      this.download4IE(url);
    } else {
      this.diyHttp.aDownloadFunc(name, url);
    }
  }

  download4IE(url: string) {
    this.Re2.setAttribute(this.el.nativeElement.querySelector('.preview-iframe'), 'src', url);
  }

  closeModal() {
    if (!this.lock.checkLock('onConfirm')) {
      this.Re2.setStyle(this.modalChild.nativeElement.querySelector('.modal'), 'display', 'none');
    }
  }

  onShowTip(type: string) {
    const tipDict = {
      restart: 'UPGRADE.restartTip',
      factoryReset: 'UPGRADE.factoryReset'
    };
    this.tips.showCTip(tipDict[type]);
    this.tipType = type;
    this.observeCT();
  }

  observeCT() {
    this.lock.lock('ctOb', true);
    this.ctOb = this.tips.ctAction.subscribe(
      change => {
        if (change === 'onYes') {
          this.tips.setCTPara('hideAll');
          switch (this.tipType) {
            case 'restart':
              this.tips.setCTPara('restart');
              break;
            case 'factoryReset':
              this.tips.setCTPara('reset');
              break;
          }
        }
        this.ctOb.unsubscribe();
        this.ctOb = null;
        this.lock.unlock('ctOb');
      }
    );
  }

  onInput() {
    this.lock.lock('onInputFunc', true);
    if (!this.ctObserver) {
      this.tips.showCTip('inputDBTip');
      this.ctObserver = this.tips.ctAction.subscribe(
        action => {
          if (action === 'onYes') {
            this.onInputFunc();
          } else if (action === 'onNo') {
            this.lock.unlock('onInputFunc');
            this.ctObserver.unsubscribe();
            this.ctObserver = null;
          }
        }
      );
    }
  }

  onInputFunc() {
    if (this.infoOverSize) {
      this.tips.setRbTip('UPGRADE.oversize');
    } else {
      if (this.InfoFile.file) {
        this.cfg.putDeviceInfo(this.InfoFile.file).subscribe(
          res => {
            this.lock.unlock('onInputFunc');
            this.setProcessBlue('for-upload');
            this.setPercentage('for-upload', 100);
            this.tips.setCTip('configingDb');
            this.rebootForDb();
            this.tips.setCTPara('restart');
            this.InfoFile.file = null;
            this.uploadFileName = 'noFileSelected';
          },
          err => {
            this.lock.unlock('onInputFunc');
            this.logger.error(err, 'onInputFunc:putDeviceInfo:');
            this.tips.setRbTip('uploadFail');
            this.setProcessRed('for-upload');
          }
        );
      } else {
        this.tips.setRbTip('UPGRADE.noFile');
      }
    }
  }

  rebootForDb() {
    this.cfg.reboot4DB().subscribe();
  }

  onFileClick(id: string) {
    const event = new MouseEvent('click');
    const fileInputBtn = document.getElementById(id);
    fileInputBtn.dispatchEvent(event);
  }

  setPercentage(cl: string, percent: number) {
    this.Re2.setStyle(this.el.nativeElement.querySelector('.process-left.mt-2.' + cl), 'width', percent + '%');
    const left = 100 - percent;
    this.Re2.setStyle(this.el.nativeElement.querySelector('.process-right.mt-2.' + cl), 'width', left + '%');
  }

  setProcessRed(cl: string) {
    this.Re2.setStyle(this.el.nativeElement.querySelector('.process-left.mt-2.' + cl), 'background-color', 'red');
  }

  setProcessBlue(cl: string) {
    this.Re2.setStyle(this.el.nativeElement.querySelector('.process-left.mt-2.' + cl), 'background-color', '#007fe0');
  }

  initProcess(cl: string) {
    this.setPercentage(cl, 0);
  }
}
