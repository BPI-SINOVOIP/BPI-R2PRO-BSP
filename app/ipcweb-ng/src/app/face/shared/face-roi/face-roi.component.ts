import { Component, OnInit, AfterViewInit, OnDestroy, ViewChild, Input, ElementRef } from '@angular/core';
import { FormBuilder, Validators, FormControl } from '@angular/forms';

import { ConfigService } from 'src/app/config.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { TipsService } from 'src/app/tips/tips.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { DiyHttpService } from 'src/app/shared/func-service/diy-http.service';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-face-roi',
  templateUrl: './face-roi.component.html',
  styleUrls: ['./face-roi.component.scss']
})
export class FaceRoiComponent implements OnInit, AfterViewInit, OnDestroy {

  @ViewChild('player', {static: true}) playerChild: any;
  @ViewChild('player', {read: ElementRef}) playerDom: ElementRef;
  @Input() options: any;

  constructor(
    private cfg: ConfigService,
    private fb: FormBuilder,
    private resError: ResponseErrorService,
    private tips: TipsService,
    private pfs: PublicFuncService,
    private dhs: DiyHttpService,
  ) { }

  playerOption = {
    isReshape: false,
    expendBtn: ['all', 'save', 'undraw'],
    imageMode: true,
  };

  FaceRoi =  this.fb.group({
    iDetectHeight: [NaN],
    iDetectWidth: [NaN],
    iFaceDetectionThreshold: [NaN],
    iFaceMinPixel: [NaN],
    iFaceRecognitionThreshold: [NaN],
    iLeftCornerX: [NaN],
    iLeftCornerY: [NaN],
    iLiveDetectThreshold: [NaN],
    iPromptVolume: [NaN],
    id: [NaN],
    sLiveDetect: [''],
    sLiveDetectBeginTime: [''],
    sLiveDetectEndTime: [''],
    iNormalizedHeight: [''],
    iNormalizedWidth: [''],
  });
  private logger: Logger = new Logger('face-roi');

  private viewInit: boolean = false;
  private urlOb: any;
  private saveOb: any;
  private src = '';

  ngOnInit(): void {
    this.cfg.getFaceParaInterface().subscribe(
      res => {
        this.resError.analyseRes(res);
        this.FaceRoi.patchValue(res);
        this.wait2Init(5000)
          .then(
            () => {
              this.initDrawArray();
            }
          )
          .catch(
            () => {
              this.tips.setRbTip('initPlayerFailFreshPlease');
            }
          );
      },
      err => {
        this.tips.showInitFail();
        this.logger.error(err, 'ngOnInit:getFaceParaInterface:');
      }
    );

    if (!this.playerOption.imageMode) {
      this.urlOb = this.dhs.getStreamUrl().subscribe(
        (msg: string) => {
          if (msg) {
            this.src = msg;
            this.wait2Init(5000)
              .then(
                () => {
                  this.playEntry();
                }
              )
              .catch(
                () => {
                  this.tips.setRbTip('initPlayerFailFreshPlease');
                }
              );
          } else {
            this.tips.setRbTip('getVideoUrlFail');
          }
          if (this.urlOb) {
            this.urlOb.unsubscribe();
            this.urlOb = null;
          }
        }
      );
    } else {
      this.src = '/assets/images/face_roi.png';
      this.wait2Init(5000)
        .then(
          () => {
            this.playEntry();
          }
        )
        .catch(
          () => {
            this.tips.setRbTip('initPlayerFailFreshPlease');
          }
        );
    }

  }

  ngAfterViewInit() {
    this.playerChild.hideBigPlayBtn();
    this.viewInit = true;
  }

  ngOnDestroy() {
    if (this.urlOb) {
      this.urlOb.unsubscribe();
    }

    if (this.saveOb) {
      this.saveOb.unsubscribe();
    }

    try {
      this.playerChild.diyStop();
    } catch {}
    this.playerChild.destroyWhenSwitch();
  }

  onSubmit() {
    this.formatRoiForm();
    this.pfs.formatInt(this.FaceRoi.value);
    this.cfg.putFaceParaInterface(this.FaceRoi.value).subscribe(
      res => {
        this.resError.analyseRes(res);
        this.FaceRoi.patchValue(res);
        this.tips.showSaveSuccess();
      },
      err => {
        this.logger.error(err, 'onSubmit:');
        this.tips.showSaveFail();
      }
    );
  }

  initDrawArray() {
    if (this.FaceRoi.value.iNormalizedHeight && this.FaceRoi.value.iNormalizedWidth) {
      this.playerChild.isDrawing = true;
      this.playerChild.reshapeByResolution(this.FaceRoi.value.iNormalizedHeight, this.FaceRoi.value.iNormalizedWidth);
      this.playerChild.pointEnabled = true;
      this.playerChild.initDrawer(this.FaceRoi.value.iNormalizedWidth, this.FaceRoi.value.iNormalizedHeight);
      this.playerChild.pushDrawArray(
        true, this.FaceRoi.value.iLeftCornerX, this.FaceRoi.value.iLeftCornerY,
        '', this.FaceRoi.value.iDetectWidth, this.FaceRoi.value.iDetectHeight, true
      );
      this.playerChild.drawPic();
      this.saveOb = this.playerChild.SaveSignal.subscribe(
        (signal: boolean) => {
          if (!signal) {
            this.onSubmit();
          }
        }
      );
    }
  }

  formatRoiForm() {
    const cell = this.playerChild.getDrawArray(0);
    this.FaceRoi.value.iDetectHeight = cell.height;
    this.FaceRoi.value.iDetectWidth = cell.width;
    this.FaceRoi.value.iLeftCornerX = cell.x;
    this.FaceRoi.value.iLeftCornerY = cell.y;
    if (this.FaceRoi.value.iDetectHeight === 0 || this.FaceRoi.value.iDetectWidth === 0) {
      this.FaceRoi.value.iLeftCornerX = 0;
      this.FaceRoi.value.iLeftCornerY = 0;
      this.FaceRoi.value.iDetectHeight = this.FaceRoi.value.iNormalizedHeight;
      this.FaceRoi.value.iDetectWidth = this.FaceRoi.value.iNormalizedWidth;
    }
    this.pfs.formatInt(this.FaceRoi.value);
    this.FaceRoi.patchValue(this.FaceRoi.value);
  }

  playEntry() {
    if (this.viewInit && this.src) {
      this.playerChild.displayUrl = this.src;
      this.playerChild.bigBtnPlay();
    }
  }

  wait2Init = (timeoutms: number) => new Promise(
    (resolve, reject) => {
      const waitFunc = () => {
        timeoutms -= 100;
        if (this.viewInit) {
          resolve();
        } else if (timeoutms <= 0) {
          reject();
        } else {
          setTimeout(waitFunc, 100);
        }
      };
      waitFunc();
    }
  )
}
