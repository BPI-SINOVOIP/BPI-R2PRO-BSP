import { Component, OnInit, ViewChild, AfterViewInit, OnDestroy } from '@angular/core';
import { FormBuilder } from '@angular/forms';

import { ConfigService } from 'src/app/config.service';
import { MotionRegionInterface } from './MotionRegionInterface';
import { TipsService } from 'src/app/tips/tips.service';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';

import Logger from 'src/app/logger';

@Component({
  selector: 'app-motion-region',
  templateUrl: './motion-region.component.html',
  styleUrls: ['./motion-region.component.scss']
})
export class MotionRegionComponent implements OnInit, AfterViewInit, OnDestroy {

  constructor(
    private fb: FormBuilder,
    private cfg: ConfigService,
    private tips: TipsService,
    private ieCss: IeCssService,
  ) { }

  @ViewChild('player', {static: true}) playerChild: any;
  private logger: Logger = new Logger('motion-region');

  private isViewInit: boolean = false;
  private oldSensitive: number;
  private src: string = '';
  private isIe: boolean = true;

  playerOption = {
    isReshape: true,
    speName: 'motion-region',
  };

  RegionForm = this.fb.group({
    iColumnGranularity: 22,
    iEndTriggerTime: 500,
    iRowGranularity: 18,
    iSamplingInterval: 2,
    iSensitivityLevel: 1,
    iStartTriggerTime: 500,
    id: [''],
    sGridMap: '0000012345456000000000000000000000000000000000000000000000000000000000000000000000000000000000000000',
    sRegionType: 'grid'
  });

  ngOnInit(): void {
    this.isIe = this.ieCss.getIEBool();
  }

  ngAfterViewInit() {
    this.playerChild.hideBigPlayBtn();
    this.isViewInit = true;
  }

  ngOnDestroy() {
    try {
      this.playerChild.diyStop();
    } catch {
      this.logger.log("this.playerChild.diyStop() fail");
    }
    this.playerChild.destroyWhenSwitch();
  }

  initComponent(initInfo: MotionRegionInterface, url: string) {
    this.src = url;
    this.RegionForm.patchValue(initInfo);
    this.oldSensitive = this.RegionForm.value.iSensitivityLevel;
    this.waitViewInit(5000)
      .then(() => {
        this.initDrawArray();
        this.playEntry();
      })
      .catch((error) => {
        this.logger.error(error, 'initComponent:waitViewInit:catch:');
        this.tips.setRbTip('initFailFreshPlease');
      });
  }

  waitViewInit = (timeoutms: number) => new Promise(
    (resolve, reject) => {
      const isViewInitComplete = () => {
        timeoutms -= 100;
        if (this.isViewInit) {
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

  initDrawArray() {
    this.playerChild.motionMode = true;
    this.playerChild.initMotionDrawer(this.RegionForm.value.iColumnGranularity,
      this.RegionForm.value.iRowGranularity, this.RegionForm.value.sGridMap
    );
    this.playerChild.drawPic();
  }

  getMotionRegionForm() {
    this.RegionForm.value.sGridMap = this.playerChild.getMotionString();
    return this.RegionForm.value;
  }

  onRangeChange(change: number) {
    if ((this.oldSensitive || this.oldSensitive === 0) && change != this.oldSensitive) {
      this.oldSensitive = change;
      this.RegionForm.get('iSensitivityLevel').setValue(change);
    }
  }

  playEntry() {
    if (this.isViewInit && this.src) {
      this.playerChild.displayUrl = this.src;
      this.playerChild.bigBtnPlay();
    }
  }

  pausePlayer() {
    if (this.isViewInit && this.playerChild) {
    this.playerChild.diyPause();
    }
  }

  stopPlayer() {
    if (this.isViewInit && this.playerChild && this.playerChild.isPlaying) {
      this.playerChild.diyStop();
    }
  }

  resizePlayer() {
    this.playerChild.reshapeCanvas();
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
}
