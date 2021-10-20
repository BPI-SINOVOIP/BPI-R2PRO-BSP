import { Component, OnInit, AfterViewInit, Renderer2, ElementRef, ViewChild, OnDestroy } from '@angular/core';
import { FormBuilder } from '@angular/forms';

import { ConfigService } from 'src/app/config.service';
import { MenuGroup } from '../../MenuGroup';
import { RegionCropInterface } from './RegionCropInterface';
import { StreamURLInterface } from 'src/app/preview/StreamURLInterface';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { TipsService } from 'src/app/tips/tips.service';
import { LockService } from 'src/app/shared/func-service/lock-service.service';
import { VideoDefaultPara } from '../encoder-param/VideoEncoderInterface';
import { DiyHttpService } from 'src/app/shared/func-service/diy-http.service';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-region-crop',
  templateUrl: './region-crop.component.html',
  styleUrls: ['./region-crop.component.scss']
})
export class RegionCropComponent implements OnInit, AfterViewInit, OnDestroy {

  constructor(
    private Re2: Renderer2,
    private el: ElementRef,
    private fb: FormBuilder,
    private cfg: ConfigService,
    private ieCss: IeCssService,
    private resError: ResponseErrorService,
    private tips: TipsService,
    private pfs: PublicFuncService,
    private dhs: DiyHttpService,
  ) { }

  @ViewChild('player', {static: true}) playerChild: any;
  @ViewChild('player', {read: ElementRef}) playerDom: ElementRef;

  // private lock = new LockService(this.tips);
  isChrome: boolean = false;
  private logger: Logger = new Logger('regionCrop');

  pixelList = [];
  pixelModel: any;
  private iNormalizedScreenHeight: number;
  private iNormalizedScreenWidth: number;
  private pixelPat = /^(.*?)\*(.*?)$/;
  private viewInit: boolean = false;
  private nowResolution: string = '';
  private src = '';
  private urlOb: any;
  isUseful: boolean = true;

  playerOption = {
    isReshape: true,
    speName: 'region-crop',
  };

  RegionForm = this.fb.group({
    iWidth: 640,
    iHeight: 480,
    iRegionClipEnabled: 1,
    iPositionX: 0,
    iPositionY: 0,
    sResolution: ''
  });

  ngOnInit() {
    this.isChrome = this.ieCss.getChromeBool();

    this.cfg.getDefaultPara(4).subscribe(res => {
      this.resError.analyseRes(res);
      const rst: VideoDefaultPara = JSON.parse(res.toString());
      if (this.pfs.isInArrayString(rst.static['sStreamType']['options'], 'thirdStream')) {
        if (rst.dynamic['sStreamType']['thirdStream']) {
          this.pixelList = rst.dynamic['sStreamType']['thirdStream']['sResolution']['options'];
        } else {
          this.RegionForm.disable();
          this.isUseful = false;
        }
      } else {
        this.RegionForm.disable();
        this.isUseful = false;
      }
    });

    this.cfg.getRegionCropInterface().subscribe((res: RegionCropInterface) => {
      this.iNormalizedScreenHeight = res.normalizedScreenSize.iNormalizedScreenHeight;
      this.iNormalizedScreenWidth = res.normalizedScreenSize.iNormalizedScreenWidth;
      this.RegionForm.patchValue(res.regionClip);
      this.RegionForm.value.sResolution = this.RegionForm.value.iWidth + '*'
        +  this.RegionForm.value.iHeight;
      this.RegionForm.patchValue(this.RegionForm.value);
      this.nowResolution = this.RegionForm.value.sResolution;
      if (this.viewInit) {
        this.initDrawArray();
      }
    });

    this.urlOb = this.dhs.getStreamUrl('thirdstream').subscribe(
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
    const playerButton = this.playerDom.nativeElement.querySelectorAll('.blue-btn');
    for (const btn of playerButton) {
      this.Re2.setAttribute(btn, 'hidden', 'true');
    }
    this.playerChild.hideBigPlayBtn();
    this.viewInit = true;
    this.initDrawArray();
    this.playEntry();
  }

  ngOnDestroy() {
    try {
      this.playerChild.diyStop();
    } catch {}
    this.playerChild.destroyWhenSwitch();
    if (this.urlOb) {
      this.urlOb.unsubscribe();
    }
  }

  initDrawArray() {
    if (this.iNormalizedScreenHeight && this.iNormalizedScreenWidth) {
      this.playerChild.pointEnabled = false;
      this.playerChild.isDrawing = false;
      this.playerChild.initDrawer(this.iNormalizedScreenWidth, this.iNormalizedScreenHeight);
      this.playerChild.pushDrawArray(
        true, this.RegionForm.value.iPositionX, this.RegionForm.value.iPositionY,
        '', (this.RegionForm.value.iWidth / 1920) * this.iNormalizedScreenWidth,
        (this.RegionForm.value.iHeight / 1080) * this.iNormalizedScreenHeight,
        true
      );
      this.playerChild.drawPic();
    }
  }

  onSubmit() {
    const cell = this.playerChild.getDrawArray(0);
    this.RegionForm.value.iPositionX = cell.x;
    this.RegionForm.value.iPositionY = cell.y;
    this.pfs.formatInt(this.RegionForm.value);
    this.RegionForm.patchValue(this.RegionForm.value);
    this.RegionForm.get('sResolution').disable();
    this.cfg.putRegionCropInterface(this.RegionForm.value).subscribe(
      (res: RegionCropInterface) => {
        this.resError.analyseRes(res);
        this.RegionForm.patchValue(res.regionClip);
        this.RegionForm.get('sResolution').enable();
        this.RegionForm.value.sResolution = this.RegionForm.value.iWidth + '*'
        +  this.RegionForm.value.iHeight;
        // this.initDrawArray();
        this.tips.showSaveSuccess();
      },
      err => {
        this.logger.error(err, 'onSubmit:putRegionCropInterface:');
        this.tips.showSaveFail();
      }
    );
  }

  onPixelChange(change: string) {
    if (this.nowResolution && this.nowResolution !== change) {
      this.nowResolution = change;
      const pixelRe = this.pixelPat.exec(change);
      this.RegionForm.value.iWidth = Number(pixelRe[1]);
      this.RegionForm.value.iHeight = Number(pixelRe[2]);
      this.initDrawArray();
    }
  }

  playEntry() {
    if (this.viewInit && this.src) {
      this.playerChild.displayUrl = this.src;
      this.playerChild.bigBtnPlay();
    }
  }
}
