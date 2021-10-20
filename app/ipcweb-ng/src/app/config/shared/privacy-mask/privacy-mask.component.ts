import { Component, OnInit, AfterViewInit, OnDestroy, ViewChild, ElementRef, Renderer2, Input } from '@angular/core';
import { FormBuilder, FormGroup, FormArray } from '@angular/forms';

import { ConfigService } from 'src/app/config.service';
import { PrivacyMaskInterface } from './PrivacyMaskInterface';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { TipsService } from 'src/app/tips/tips.service';
import { LockService } from 'src/app/shared/func-service/lock-service.service';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-privacy-mask',
  templateUrl: './privacy-mask.component.html',
  styleUrls: ['./privacy-mask.component.scss']
})
export class PrivacyMaskComponent implements OnInit, AfterViewInit, OnDestroy {

  @Input() option: any;
  @ViewChild('player', {static: true}) playerChild: any;
  @ViewChild('player', {read: ElementRef}) playerDom: ElementRef;

  constructor(
    private fb: FormBuilder,
    private el: ElementRef,
    private Re2: Renderer2,
    private cfgService: ConfigService,
    private ieCss: IeCssService,
    private resError: ResponseErrorService,
    private tips: TipsService,
    private pfs: PublicFuncService,
  ) { }

  // private lock = new LockService(this.tips);
  isChrome: boolean = false;
  private logger: Logger = new Logger('privacyMask');

  private iNormalizedScreenHeight: number;
  private iNormalizedScreenWidth: number;
  private viewInit: boolean = false;
  private vChange: any;
  // set for number of mask
  maskNum: number = 1;
  private saveOb: any;

  playerOption = {
    isReshape: true,
    expendBtn: ['save'],
    speName: 'privacy-mask',
  };

  observeForm = this.fb.group({
    enabled: false,
    button: [''],
  });

  MaskForm = this.fb.group({
    normalizedScreenSize: this.fb.group({
      iNormalizedScreenHeight: [''],
      iNormalizedScreenWidth: [''],
    }),
    privacyMask: this.fb.array([
      {
        iMaskHeight: [''],
        iMaskWidth: [''],
        iPositionX: [''],
        iPositionY: [''],
        id: [''],
        iPrivacyMaskEnabled: [''],
      },
      {
        iMaskHeight: [''],
        iMaskWidth: [''],
        iPositionX: [''],
        iPositionY: [''],
        id: [''],
        iPrivacyMaskEnabled: [''],
      },
      {
        iMaskHeight: [''],
        iMaskWidth: [''],
        iPositionX: [''],
        iPositionY: [''],
        id: [''],
        iPrivacyMaskEnabled: [''],
      },
      {
        iMaskHeight: [''],
        iMaskWidth: [''],
        iPositionX: [''],
        iPositionY: [''],
        id: [''],
        iPrivacyMaskEnabled: [''],
      }
    ])
  });

  get normalizedScreenSize(): FormGroup {
    return this.MaskForm.get('normalizedScreenSize') as FormGroup;
  }
  get privacyMask(): FormArray {
    return this.MaskForm.get('privacyMask') as FormArray;
  }


  ngOnInit(): void {
    this.getUrl(5000)
      .then(
        () => {
          this.wait2Init(5000)
            .then(
              () => {
                this.playEntry(this.option['src']);
              }
            )
            .catch(
              () => {
                this.tips.setRbTip('initPlayerFailFreshPlease');
              }
            );
        }
      )
      .catch(
        () => {
          this.tips.setRbTip('getVideoUrlFail');
        }
      );
    this.isChrome = this.ieCss.getChromeBool();
    this.cfgService.getPrivacyMaskInterface().subscribe(
      (res: PrivacyMaskInterface) => {
        this.resError.analyseRes(res);
        this.MaskForm.patchValue(res);
        this.observeForm.get('enabled').patchValue(this.MaskForm.value.privacyMask[0].iPrivacyMaskEnabled);
        this.iNormalizedScreenWidth = res.normalizedScreenSize.iNormalizedScreenWidth;
        this.iNormalizedScreenHeight = res.normalizedScreenSize.iNormalizedScreenHeight;
        if (this.viewInit) {
          this.initDrawArray();
        }
      },
      err => {
        this.tips.setRbTip('getParaFailFreshPlease');
      }
    );

    this.vChange = this.observeForm.valueChanges.subscribe(change => {
      for (let i = 0; i < this.maskNum; i++) {
        this.MaskForm.value.privacyMask[i].iPrivacyMaskEnabled = Number(change.enabled);
      }
    });
  }

  ngAfterViewInit(): void {
    this.playerChild.hideBigPlayBtn();
    this.viewInit = true;
    this.initDrawArray();
  }

  ngOnDestroy() {
    if (this.saveOb) {
      this.saveOb.unsubscribe();
    }
    if (this.vChange) {
      this.vChange.unsubscribe();
    }
    this.stopPlayer();
  }

  initDrawArray() {
    if (this.iNormalizedScreenHeight && this.iNormalizedScreenWidth) {
      this.playerChild.pointEnabled = true;
      this.playerChild.isDrawing = false;
      this.playerChild.initDrawer(this.iNormalizedScreenWidth, this.iNormalizedScreenHeight);
      for (let i = 0; i < this.maskNum; i++) {
        if (this.MaskForm.value.privacyMask[i].iMaskWidth === 0 &&
          this.MaskForm.value.privacyMask[i].iMaskHeight === 0) {
            this.playerChild.pushDrawArray(
              true, 0, 0, '', 0, 0, true
            );
          } else {
            this.playerChild.pushDrawArray(
              true, this.MaskForm.value.privacyMask[i].iPositionX, this.MaskForm.value.privacyMask[i].iPositionY,
              '', this.MaskForm.value.privacyMask[i].iMaskWidth, this.MaskForm.value.privacyMask[i].iMaskHeight,
              true
            );
          }
      }
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

  setMaskCanvas(): void {
    this.playerChild.reshapeCanvas();
    this.initDrawArray();
  }

  onSubmit() {
    this.pfs.formatInt(this.MaskForm.value);
    for (let i = 0; i < this.maskNum; i++) {
      const cell = this.playerChild.getDrawArray(i);
      if ( cell.width === 0 || cell.height === 0) {
        this.MaskForm.value.privacyMask[i].iMaskWidth = 0;
        this.MaskForm.value.privacyMask[i].iMaskHeight = 0;
        this.MaskForm.value.privacyMask[i].iPositionX = 0;
        this.MaskForm.value.privacyMask[i].iPositionY = 0;
      } else {
        this.MaskForm.value.privacyMask[i].iMaskWidth = cell.width;
        this.MaskForm.value.privacyMask[i].iMaskHeight = cell.height;
        this.MaskForm.value.privacyMask[i].iPositionX = cell.x;
        this.MaskForm.value.privacyMask[i].iPositionY = cell.y;
      }
    }
    this.cfgService.setPrivacyMaskInterface(this.MaskForm.value).subscribe(
      res => {
        this.resError.analyseRes(res);
        this.MaskForm.patchValue(res);
        // this.initDrawArray();
        this.tips.showSaveSuccess();
      },
      err => {
        this.logger.error(err, 'onSubmit:setPrivacyMaskInterface:');
        this.tips.showSaveFail();
      }
    );
  }

  playEntry(src: string) {
    if (this.viewInit && src) {
      this.playerChild.displayUrl = src;
      this.playerChild.bigBtnPlay();
    }
  }

  pausePlayer() {
    if (this.viewInit && this.playerChild) {
    this.playerChild.diyPause();
    this.playerChild.destroyWhenSwitch();
    }
  }

  stopPlayer() {
    if (this.viewInit && this.playerChild && this.playerChild.isPlaying) {
      this.playerChild.diyStop();
      this.playerChild.destroyWhenSwitch();
    }
  }

  getUrl = (timeoutms: number) => new Promise(
    (resolve, reject) => {
      const waitFunc = () => {
        timeoutms -= 100;
        if (this.option['src']) {
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
