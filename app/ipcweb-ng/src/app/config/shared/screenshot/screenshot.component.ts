import { Component, OnInit, ViewChild, ElementRef, Renderer2 } from '@angular/core';
import { FormBuilder, Validators, FormControl } from '@angular/forms';

import { ConfigService } from 'src/app/config.service';
import { isNumberJudge } from 'src/app/shared/validators/benumber.directive';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { BoolEmployee, EmployeeItem } from 'src/app/shared/func-service/employee.service';
import { TipsService } from 'src/app/tips/tips.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { DiyHttpService } from 'src/app/shared/func-service/diy-http.service';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-screenshot',
  templateUrl: './screenshot.component.html',
  styleUrls: ['./screenshot.component.scss']
})
export class ScreenshotComponent implements OnInit {

  constructor(
    private fb: FormBuilder,
    private el: ElementRef,
    private Re2: Renderer2,
    private cfg: ConfigService,
    private ieCss: IeCssService,
    private resError: ResponseErrorService,
    private tips: TipsService,
    private pfs: PublicFuncService,
    private dhs: DiyHttpService,
  ) { }

  objectKeys = this.pfs.objectKeys;
  private logger: Logger = new Logger('screenshot');
  isChrome: boolean = false;
  private employee = new BoolEmployee();
  imageType = ['JPEG', ];
  resolutionList = ['2688*1520', ];
  qualityOptions = [1, 5, 10];
  timingUnit = [];
  eventUnit = [];
  quality2number = {
    low: 1,
    middle: 5,
    high: 10
  };

  number2quality = {
    '1': 'low',
    '5': 'middle',
    '10': 'high'
  };

  time2number = {
    milliseconds: 1,
    seconds: 1000,
    minutes: 1000 * 60,
    hours: 1000 * 60 * 60,
    days: 1000 * 60 * 60 * 24
  };

  limitRange = {
    timing: {
      max: 604800000,
      min: 1000,
      oMin: 1000,
      oMax: 604800000,
    },
    event: {
      max: 65535,
      min: 1000,
      oMin: 1000,
      oMax: 65535,
    },
    shot: {
      max: 120,
      min: 1,
      oMin: 1,
      oMax: 120,
    }
  };

  timingPara = {
    timingTimeType: 'milliseconds',
    lastTimingTimeType: 'milliseconds',
  };

  eventPara = {
    timingTimeType: 'milliseconds',
    lastTimingTimeType: 'milliseconds',
  };

  TimingForm = this.fb.group({
    iEnabled: 1,
    sImageType: 'JPEG',
    sResolution: '2688*1520',
    iImageQuality: 10,
    iShotInterval: 2000,
  });

  EventForm = this.fb.group({
    iEnabled: 1,
    sImageType: 'JPEG',
    sResolution: '2688*1520',
    iImageQuality: 10,
    iShotInterval: 2000,
    iShotNumber: 1,
  });

  get iShotIntervalTiming(): FormControl {
    return this.TimingForm.get('iShotInterval') as FormControl;
  }

  get iShotIntervalEvent(): FormControl {
    return this.EventForm.get('iShotInterval') as FormControl;
  }

  get iShotNumber(): FormControl {
    return this.EventForm.get('iShotNumber') as FormControl;
  }

  ngOnInit(): void {
    this.isChrome = this.ieCss.getChromeBool();
    this.cfg.getDefaultPara(0).subscribe(
      res => {
        this.resError.analyseRes(res);
        const js = JSON.parse(res.toString());
        if (js['static']) {
          if (js['static']['sResolution']) {
            const ob = this.dhs.getReferParaOption(js['static']['sResolution']).subscribe(
              (msg: string) => {
                this.resolutionList = JSON.parse(msg);
              }
            );
          }
          this.limitRange.shot.min = js['static']['iShotNumber']['range']['min'];
          this.limitRange.shot.max = js['static']['iShotNumber']['range']['max'];
          this.imageType = js['static']['sImageType']['options'] ? js['static']['sImageType']['options'] : this.imageType;
          this.qualityOptions = js['static']['iImageQuality']['options'];
        }
        if (js['relation']) {
          this.number2quality = js['relation']['iImageQuality'] ? js['relation']['iImageQuality'] : this.quality2number;
        }
        if (js['dynamic']) {
          this.limitRange.timing.min = js['dynamic']['id']['0']['iShotInterval']['range']['min'];
          this.limitRange.timing.max = js['dynamic']['id']['0']['iShotInterval']['range']['max'];
          this.timingUnit = js['dynamic']['id']['0']['timeUnit']['options'];
          this.limitRange.event.min = js['dynamic']['id']['1']['iShotInterval']['range']['min'];
          this.limitRange.event.max = js['dynamic']['id']['1']['iShotInterval']['range']['max'];
          this.eventUnit = js['dynamic']['id']['1']['timeUnit']['options'];
        }
        // tslint:disable-next-line: forin
        for (const key in this.limitRange) {
          this.limitRange[key].oMin = this.limitRange[key].min;
          this.limitRange[key].oMax = this.limitRange[key].max;
        }
        this.EventForm.get('iShotNumber').setValidators(
          [Validators.min(this.limitRange.shot.min), Validators.max(this.limitRange.shot.max), Validators.required, isNumberJudge]);
        this.EventForm.get('iShotNumber').updateValueAndValidity();
        this.EventForm.get('iShotInterval').setValidators(
          [Validators.min(this.limitRange.event.min), Validators.max(this.limitRange.event.max), Validators.required, isNumberJudge]);
        this.EventForm.get('iShotInterval').updateValueAndValidity();
        this.TimingForm.get('iShotInterval').setValidators(
          [Validators.min(this.limitRange.timing.min), Validators.max(this.limitRange.timing.max), Validators.required, isNumberJudge]);
        this.EventForm.get('iShotInterval').updateValueAndValidity();
        this.getData();
      },
      err => {
        this.pfs.waitNavActive(20000, 'ScreenshotParaTab')
          .then(
            () => {
              this.tips.setRbTip('getParaFailFreshPlease');
            }
          )
          .catch(
            (error) => {
              this.logger.error(error, 'ngOnInit:getDefaultPara(0):waitNavActive:catch:');
            }
          );
        this.logger.error(err, 'ngOnInit:getDefaultPara(0):');
      }
    );
  }

  disabledFunc() {
    this.timingPara.timingTimeType = 'seconds';
    this.onTimingChange('seconds');
    this.eventPara.timingTimeType = 'seconds';
    this.onEventChange('seconds');
    if (this.time2number.milliseconds) {
      delete this.time2number.milliseconds;
    }
  }

  getData() {
    const ep: EmployeeItem = {
      name: 'getData',
      task: ['timing', 'event'],
    };
    this.employee.hire(ep);
    this.cfg.getScreenshotTimingInfo().subscribe(
      res => {
        this.TimingForm.patchValue(res);
        this.employee.doTask(ep.name, ep.task[0], true);
      },
      err => {
        this.logger.error(err, 'getData:getScreenshotTimingInfo:');
        this.employee.doTask(ep.name, ep.task[0], false);
      }
    );
    this.cfg.getScreenshotEventInfo().subscribe(
      res => {
        this.EventForm.patchValue(res);
        this.employee.doTask(ep.name, ep.task[1], true);
      },
      err => {
        this.logger.error(err, 'getData:getScreenshotEventInfo:');
        this.employee.doTask(ep.name, ep.task[1], false);
      }
    );
    this.employee.observeTask(ep.name, 5000)
      .then(
        () => {
          if (!this.employee.getWorkResult(ep.name)) {
            this.showInitFail();
          } else {
            // set for unsupport function
            this.disabledFunc();
          }
          this.employee.dismissOne(ep.name);
        }
      )
      .catch(
        () => {
          this.showInitFail();
          this.employee.dismissOne(ep.name);
        }
      );
  }

  showInitFail() {
    this.pfs.waitNavActive(20000, 'ScreenshotParaTab')
      .then(
        () => {
          this.tips.showInitFail();
        }
      )
      .catch();
  }

  onTimingChange(change: string) {
    if (change !== this.timingPara.lastTimingTimeType) {
      const proportion = this.time2number[change] / this.time2number[this.timingPara.lastTimingTimeType];
      this.TimingForm.get('iShotInterval').setValue(Math.ceil(this.TimingForm.value.iShotInterval / proportion));
      if (change === 'milliseconds') {
        this.limitRange.timing.min = this.limitRange.timing.oMin;
        this.limitRange.timing.max = this.limitRange.timing.oMax;
      } else {
        this.limitRange.timing.min /= proportion;
        this.limitRange.timing.max /= proportion;
        this.limitRange.timing.min = Math.max(1, this.limitRange.timing.min);
        this.limitRange.timing.min = Math.min(this.limitRange.timing.min, this.limitRange.timing.oMin);
        this.limitRange.timing.max = this.beMinInteger(this.limitRange.timing.max);
      }
      this.TimingForm.get('iShotInterval').setValidators(
        [Validators.min(this.limitRange.timing.min), Validators.max(this.limitRange.timing.max), Validators.required, isNumberJudge]);
      this.TimingForm.get('iShotInterval').updateValueAndValidity();
      this.timingPara.lastTimingTimeType = change;
    }
  }

  onEventChange(change: string) {
    if (change !== this.eventPara.lastTimingTimeType) {
      const proportion = this.time2number[change] / this.time2number[this.eventPara.lastTimingTimeType];
      this.EventForm.get('iShotInterval').setValue(Math.ceil(this.EventForm.value.iShotInterval / proportion));
      if (change === 'milliseconds') {
        this.limitRange.event.min = this.limitRange.event.oMin;
        this.limitRange.event.max = this.limitRange.event.oMax;
      } else {
        this.limitRange.event.min /= proportion;
        this.limitRange.event.max /= proportion;
        this.limitRange.event.min = Math.max(1, this.limitRange.event.min);
        this.limitRange.event.min = Math.min(this.limitRange.event.min, this.limitRange.event.oMin);
        this.limitRange.event.max = this.beMinInteger(this.limitRange.event.max);
      }
      this.EventForm.get('iShotInterval').setValidators(
        [Validators.min(this.limitRange.event.min), Validators.max(this.limitRange.event.max), Validators.required, isNumberJudge]);
      this.EventForm.get('iShotInterval').updateValueAndValidity();
      this.eventPara.lastTimingTimeType = change;
    }
  }

  beMinInteger(num: number) {
    return num - (num % 1);
  }

  onSubmit() {
    const ep = {
      name: 'onSubmit',
      task: ['timing', 'event']
    };
    this.employee.hire(ep);
    this.TimingForm.value.iShotInterval = Number(this.TimingForm.value.iShotInterval) * this.time2number[this.timingPara.timingTimeType];
    this.EventForm.value.iShotInterval = Number(this.EventForm.value.iShotInterval) * this.time2number[this.eventPara.timingTimeType];
    this.pfs.formatInt(this.TimingForm.value);
    this.cfg.putScreenshotTimingInfo(this.TimingForm.value).subscribe(
      res => {
        this.resError.analyseRes(res);
        this.employee.doTask(ep.name, ep.task[0], true);
        res.iShotInterval = Math.ceil(res.iShotInterval / this.time2number[this.timingPara.timingTimeType]);
        this.TimingForm.patchValue(res);
      },
      err => {
        this.employee.doTask(ep.name, ep.task[0], false);
        this.logger.error(err, 'onSubmit:putScreenshotTimingInfo:');
      }
    );
    this.pfs.formatInt(this.EventForm.value);
    this.cfg.putScreenshotEventInfo(this.EventForm.value).subscribe(
      res => {
        this.employee.doTask(ep.name, ep.task[1], true);
        res.iShotInterval = Math.ceil(res.iShotInterval / this.time2number[this.eventPara.timingTimeType]);
        this.EventForm.patchValue(res);
      },
      err => {
        this.employee.doTask(ep.name, ep.task[1], false);
        this.logger.error(err, 'onSubmit:putScreenshotEventInfo:');
      }
    );
    this.employee.observeTask(ep.name, 5000)
      .then(
        () => {
          if (this.employee.getWorkResult(ep.name)) {
            this.tips.showSaveSuccess();
          } else {
            this.tips.showSaveFail();
          }
          this.employee.dismissOne(ep.name);
        }
      )
      .catch(
        () => {
          this.employee.dismissOne(ep.name);
          this.tips.showSaveFail();
        }
      );
  }
}
