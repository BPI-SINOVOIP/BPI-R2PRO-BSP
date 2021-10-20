import { Component, OnInit, ViewChild, OnDestroy, AfterViewInit, ElementRef, Renderer2 } from '@angular/core';
import { FormBuilder, Validators, FormControl } from '@angular/forms';

import { RegionalInvasion, NormalizedScreenSize} from 'src/app/config/shared/intrusion-detection/IntrusionInterface';
import { TipsService } from 'src/app/tips/tips.service';
import { isNumberJudge } from 'src/app/shared/validators/benumber.directive';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';

import Logger from 'src/app/logger';

@Component({
  selector: 'app-intrusion-region',
  templateUrl: './intrusion-region.component.html',
  styleUrls: ['./intrusion-region.component.scss']
})
export class IntrusionRegionComponent implements OnInit, AfterViewInit, OnDestroy {

  @ViewChild('player', {static: true}) playerChild: any;
  @ViewChild('player', {read: ElementRef}) playerDom: ElementRef;

  constructor(
    private fb: FormBuilder,
    private el: ElementRef,
    private Re2: Renderer2,
    private tips: TipsService,
    private ieCss: IeCssService,
  ) { }
  private logger: Logger = new Logger('instrusion-region');

  private viewInit: boolean = false;
  private iNormalizedScreenHeight: number;
  private iNormalizedScreenWidth: number;
  private src = '';
  private isIe: boolean = true;

  playerOption = {
    isReshape: true,
    speName: 'intrusion-region',
  };

  RegionForm = this.fb.group({
    iEnabled: 0,
    iHeight: 92,
    iPositionX: 0,
    iPositionY: 0,
    iProportion: [50 , [Validators.required, Validators.min(0), Validators.max(100), isNumberJudge]],
    iSensitivityLevel: [50 , [Validators.required, Validators.min(0), Validators.max(100), isNumberJudge]],
    iTimeThreshold: [3 , [Validators.required, Validators.min(0), isNumberJudge]],
    iWidth: 120,
  });

  oldForm = {
    iProportion: NaN,
    iSensitivityLevel: NaN,
    iTimeThreshold: NaN,
  };

  get iTimeThreshold(): FormControl {
    return this.RegionForm.get('iTimeThreshold') as FormControl;
  }

  get iProportion(): FormControl {
    return this.RegionForm.get('iProportion') as FormControl;
  }

  get iSensitivityLevel(): FormControl {
    return this.RegionForm.get('iSensitivityLevel') as FormControl;
  }

  ngOnInit(): void {
    this.isIe = this.ieCss.getIEBool();
  }

  ngAfterViewInit() {
    this.playerChild.hideBigPlayBtn();
    this.viewInit = true;
  }

  ngOnDestroy() {
    try {
      this.playerChild.diyStop();
    } catch {}
    this.playerChild.destroyWhenSwitch();
  }

  initDrawArray() {
    if (this.iNormalizedScreenHeight && this.iNormalizedScreenWidth) {
      this.playerChild.pointEnabled = true;
      this.playerChild.isDrawing = false;
      this.playerChild.initDrawer(this.iNormalizedScreenWidth, this.iNormalizedScreenHeight);
      if (this.RegionForm.value.iWidth === 0 && this.RegionForm.value.iHeight === 0) {
        this.playerChild.pushDrawArray(
          true, 0, 0, '', 0, 0, true
        );
      } else {
        this.playerChild.pushDrawArray(
          true, this.RegionForm.value.iPositionX, this.RegionForm.value.iPositionY,
          '', this.RegionForm.value.iWidth, this.RegionForm.value.iHeight,
          true
        );
      }
      this.playerChild.drawPic();
    }
  }

  onSubmit() {
    if (this.isIe) {
      const btn = document.getElementById('intrusion-save');
      btn.click();
    } else {
      const event = new MouseEvent('click');
      const btn = document.getElementById('intrusion-save');
      btn.dispatchEvent(event);
    }
  }

  getRegionInfo() {
    const cell = this.playerChild.getDrawArray(0);
    if ( cell.width === 0 || cell.height === 0) {
      this.RegionForm.value.iWidth = 0;
      this.RegionForm.value.iHeight = 0;
      this.RegionForm.value.iPositionX = 0;
      this.RegionForm.value.iPositionY = 0;
    } else {
      this.RegionForm.value.iWidth = cell.width;
      this.RegionForm.value.iHeight = cell.height;
      this.RegionForm.value.iPositionX = cell.x;
      this.RegionForm.value.iPositionY = cell.y;
    }
    return this.RegionForm.value;
  }

  onContentChange(change: number, name: string) {
    if (!isNaN(this.oldForm[name]) && Number(change) !== this.oldForm[name] && !isNaN(Number(change))) {
      change = Number(change);
      this.oldForm[name] = change;
      this.RegionForm.get(name).setValue(change);
    } else if (isNaN(Number(change)) && !isNaN(this.oldForm[name])) {
      this.RegionForm.get(name).setValue(this.oldForm[name]);
    } else if (isNaN(this.oldForm[name])) {
      this.oldForm[name] = change;
    }
  }

  playEntry() {
    if (this.viewInit && this.src) {
      this.playerChild.displayUrl = this.src;
      this.playerChild.bigBtnPlay();
    }
  }

  pausePlayer() {
    if (this.viewInit && this.playerChild) {
    this.playerChild.diyPause();
    }
  }

  stopPlayer() {
    if (this.viewInit && this.playerChild && this.playerChild.isPlaying) {
      this.playerChild.diyStop();
    }
  }

  resizePlayer() {
    this.playerChild.reshapeCanvas();
  }

  initComponent(initInfo: RegionalInvasion, url: string, normalSize: NormalizedScreenSize) {
    this.src = url;
    this.RegionForm.patchValue(initInfo);
    this.iNormalizedScreenWidth = normalSize.iNormalizedScreenWidth;
    this.iNormalizedScreenHeight = normalSize.iNormalizedScreenHeight;
    this.waitViewInit(5000)
      .then(() => {
        this.initDrawArray();
        this.playEntry();
      })
      .catch((error) => {
        this.logger.error(error, 'initComponent:waitViewInit:');
        this.tips.setRbTip('initFailFreshPlease');
      });
  }

  waitViewInit = (timeoutms: number) => new Promise(
    (resolve, reject) => {
      const isViewInitComplete = () => {
        timeoutms -= 100;
        if (this.viewInit) {
          resolve();
        } else if (timeoutms <= 0) {
          reject();
        } else {
          setTimeout(isViewInitComplete, 100);
        }
      };
      isViewInitComplete();
    }
  )

}
