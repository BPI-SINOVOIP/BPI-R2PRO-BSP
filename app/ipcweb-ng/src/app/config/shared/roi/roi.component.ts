import { Component, OnInit, AfterViewInit, OnDestroy, ViewChild, ElementRef, Renderer2 } from '@angular/core';
import { FormBuilder, FormGroup, FormArray } from '@angular/forms';

import { ConfigService } from 'src/app/config.service';
import { RoiInterface, RoiPartInterface} from './RoiInterface';
import { StreamURLInterface } from 'src/app/preview/StreamURLInterface';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { TipsService } from 'src/app/tips/tips.service';
import { LockService } from 'src/app/shared/func-service/lock-service.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { DiyHttpService } from 'src/app/shared/func-service/diy-http.service';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-roi',
  templateUrl: './roi.component.html',
  styles: []
})
export class RoiComponent implements OnInit, AfterViewInit, OnDestroy {

  constructor(
    private fb: FormBuilder,
    private el: ElementRef,
    private Re2: Renderer2,
    private cfgService: ConfigService,
    private ieCss: IeCssService,
    private resError: ResponseErrorService,
    private tips: TipsService,
    private pfs: PublicFuncService,
    private dhs: DiyHttpService,
  ) { }

  @ViewChild('player', {static: true}) playerChild: any;
  @ViewChild('player', {read: ElementRef}) playerDom: ElementRef;

  objectKeys = this.pfs.objectKeys;
  private logger: Logger = new Logger('roi');
  // private lock = new LockService(this.tips);
  isChrome: boolean = false;
  codeSet = {};
  regionSet = {};
  levelList: Array<number> = [1, 2, 3, 4, 5, 6];
  private iNormalizedScreenHeight: number;
  private iNormalizedScreenWidth: number;
  private viewInit: boolean = false;
  private urlPart = /^(.*?)Stream$/;
  private src = '';
  private urlOb: any;
  private nowStatus = {
    sStreamType: '',
    iROIId: 0,
  };

  playerOption = {
    isReshape: true,
    speName: 'roi',
  };

  RoiForm = this.fb.group({
    sStreamType: '',
    iStreamEnabled: 1,
    iROIId: 1,
    iROIEnabled: 1,
    sName: '',
    iQualityLevelOfROI: 1,
    iHeight: 1,
    iWidth: 1,
    iPositionX: 1,
    iPositionY: 1,
  });

  ngOnInit() {
    this.isChrome = this.ieCss.getChromeBool();
    this.cfgService.getTTLRoiList().subscribe((res: RoiInterface) => {
      this.resError.analyseRes(res);
      // tslint:disable-next-line: forin
      for (const i in res.ROIRegionList) {
        this.codeSet[res.ROIRegionList[i].sStreamType] = null;
        this.regionSet[res.ROIRegionList[i].iROIId] = null;
      }
      this.iNormalizedScreenHeight = res.normalizedScreenSize.iNormalizedScreenHeight;
      this.iNormalizedScreenWidth = res.normalizedScreenSize.iNormalizedScreenWidth;
      // check smart
      this.checkSmart();
      const that = this;
      this.waitForSmartCheck(10000)
        .then(() => {
          let defaultKey = '';
          // add the first of roi whose smart is close to roiform
          for (const key of this.objectKeys(this.codeSet)) {
            if (this.codeSet[key]) {
              defaultKey = key;
              break;
            }
          }
          for (const item of res.ROIRegionList) {
            if (item.sStreamType === defaultKey) {
              that.RoiForm.patchValue(item);
              if (this.viewInit) {
                this.initDrawArray();
              }
              break;
            }
          }
          that.nowStatus.sStreamType = that.RoiForm.value.sStreamType;
          that.nowStatus.iROIId = that.RoiForm.value.iROIId;
        });
    });

    // this.cfgService.getStreamURLInterface()
    //   .subscribe(
    //     (res: StreamURLInterface[]) => {
    //       this.src = res[1].sURL;
    //       this.playEntry();
    //     },
    //     err => {
    //       this.tips.setRbTip('getVideoUrlFail');
    //     }
    //   );
    this.urlOb = this.dhs.getStreamUrl().subscribe(
      (msg: string) => {
        if (msg) {
          this.src = msg;
          this.playEntry();
        } else {
          this.tips.setRbTip('getVideoUrlFail');
        }
        this.urlOb.unsubscribe();
        this.urlOb = null;
      }
    );

  }

  ngAfterViewInit() {
    this.playerChild.hideBigPlayBtn();
    this.viewInit = true;
    this.initDrawArray();
    this.playEntry();
  }

  ngOnDestroy() {
    if (this.urlOb) {
      this.urlOb.unsubscribe();
    }

    try {
      this.playerChild.diyStop();
    } catch {}
    this.playerChild.destroyWhenSwitch();
  }

  checkSmart() {
    for (const key of this.objectKeys(this.codeSet)) {
      this.cfgService.getVideoEncoderInterface(key).subscribe(
        res => {
          res['sSmart'] === 'open' ? this.codeSet[key] = false : this.codeSet[key] = true;
      });
    }
  }

  waitForSmartCheck = (timeoutms: number) => new Promise(
    (resolve, reject) => {
      const checkNotNull = () => {
        timeoutms -= 100;
        let nullNum = 0;
        for (const key of this.objectKeys(this.codeSet)) {
          if (this.codeSet[key] === null) {
            nullNum += 1;
          }
        }
        if (nullNum === 0) {
          resolve();
        } else if (timeoutms <= 0) {
          resolve();
        } else {
          setTimeout(checkNotNull, 100);
        }
      };
      checkNotNull();
    }
  )

  initDrawArray() {
    if (this.iNormalizedScreenHeight && this.iNormalizedScreenWidth) {
      this.playerChild.pointEnabled = true;
      this.playerChild.initDrawer(this.iNormalizedScreenWidth, this.iNormalizedScreenHeight);
      this.playerChild.pushDrawArray(
        true, this.RoiForm.value.iPositionX, this.RoiForm.value.iPositionY,
        '', this.RoiForm.value.iWidth, this.RoiForm.value.iHeight, true
      );
      this.playerChild.drawPic();
    }
  }

  onSubmit() {
    this.formatRoiForm();
    const url = this.urlPart.exec(this.RoiForm.value.sStreamType)[1] + '-stream';
    this.cfgService.putPartRoiList(
      url, this.RoiForm.value.iROIId, this.RoiForm.value
      ).subscribe(
        res => {
          this.resError.analyseRes(res);
          this.RoiForm.patchValue(res);
          // this.initDrawArray();
          this.tips.showSaveSuccess();
        },
        err => {
          this.logger.error(err, 'onSubmit:putPartRoiList:');
          this.tips.showSaveFail();
        }
      );
  }

  formatRoiForm() {
    const cell = this.playerChild.getDrawArray(0);
    this.RoiForm.value.iHeight = cell.height;
    this.RoiForm.value.iWidth = cell.width;
    this.RoiForm.value.iPositionX = cell.x;
    this.RoiForm.value.iPositionY = cell.y;
    this.pfs.formatInt(this.RoiForm.value);
    this.RoiForm.patchValue(this.RoiForm.value);
  }

  onStreamChange(change: string) {
    if (this.nowStatus.sStreamType && this.nowStatus.sStreamType !== change) {
      this.nowStatus.sStreamType = this.RoiForm.value.sStreamType;
      const url = this.urlPart.exec(change)[1] + '-stream';
      this.cfgService.getPartRoiList(url, this.RoiForm.value.iROIId).subscribe(
        res => {
          this.resError.analyseRes(res);
          this.RoiForm.patchValue(res);
          this.initDrawArray();
        },
        err => {
          this.logger.error(err, 'onStreamChange:getPartRoiList:');
        }
      );
    }
  }

  onRegionChange(change: number) {
    if (this.nowStatus.iROIId && this.nowStatus.iROIId != Number(change)) {
      this.nowStatus.iROIId = this.RoiForm.value.iROIId;
      const url = this.urlPart.exec(this.RoiForm.value.sStreamType)[1] + '-stream';
      const id = Number(change);
      this.cfgService.getPartRoiList(url, id).subscribe(
        res => {
          this.resError.analyseRes(res);
          this.RoiForm.patchValue(res);
          this.initDrawArray();
        },
        err => {
          this.logger.error(err, 'onRegionChange:getPartRoiList:');
        }
      );
    }
  }

  playEntry() {
    if (this.viewInit && this.src) {
      this.playerChild.displayUrl = this.src;
      this.playerChild.bigBtnPlay();
    }
  }
}

