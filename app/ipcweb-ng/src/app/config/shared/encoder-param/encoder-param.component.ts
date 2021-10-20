import { Component, OnInit, OnDestroy, ElementRef, Renderer2, ViewChild } from '@angular/core';
import { FormBuilder, Validators, FormControl } from '@angular/forms';

import { ConfigService } from 'src/app/config.service';
import { MenuGroup } from '../../MenuGroup';
import { VideoEncoderInterface } from './VideoEncoderInterface';
import { isNumberJudge } from 'src/app/shared/validators/benumber.directive';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { TipsService } from 'src/app/tips/tips.service';
import { LockerService } from 'src/app/shared/func-service/lock-service.service';
import { LayoutService } from 'src/app/shared/func-service/layout.service';
import { VideoDefaultPara, DisabledOp } from './VideoEncoderInterface';
import { BoolEmployee, EmployeeItem } from 'src/app/shared/func-service/employee.service';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-encoder-param',
  templateUrl: './encoder-param.component.html',
  styleUrls: ['./encoder-param.component.scss']
})

export class EncoderParamComponent implements OnInit, OnDestroy {

  constructor(
    private cfgService: ConfigService,
    private fb: FormBuilder,
    private Re2: Renderer2,
    private el: ElementRef,
    private ieCss: IeCssService,
    private resError: ResponseErrorService,
    private tips: TipsService,
    private pfs: PublicFuncService,
    private los: LayoutService,
  ) { }

  private logger: Logger = new Logger('encoder-param');

  private locker = new LockerService(this.tips, this.logger);
  isIE: boolean = true;
  isChrome: boolean = false;
  private ctOb: any;
  employee = new BoolEmployee();

  formatNameFunc = this.pfs.formName2translateName;
  checkTypeFunc = this.pfs.checkMenuType;
  getNormalRange = this.pfs.getRange;
  objectKeys = this.pfs.objectKeys;

  public encoderMenuGroups: any = {};
  layoutArray: Array<string> = [];
  private stream: VideoEncoderInterface;
  private dynamicMenu: Array<string> = [];
  private dynamicList: Array<string> = [];
  streamType: string = 'mainStream';
  private formChange: any;
  private rawSmart: string = 'open';
  private lastSmart: string;

  epBank: EmployeeItem[] = [
    {
      name: 'init',
      task: ['capbility', 'para']
    }
  ];

  rangeSet = {
    iStreamSmooth: {
      min: 1,
      max: 100,
      step: 1,
    }
  };

  limitRange = {
    iGOP: {
      max: 400,
      min: 0,
      oMin: 0,
      oMax: 400,
    },
    iStreamSmooth: {
      min: 1,
      max: 100,
      step: 1,
      oMin: 1,
      oMax: 100,
      oStep: 1,
    },
    shot: {
      max: 120,
      min: 1,
      oMin: 1,
      oMax: 120,
    }
  };

  iMinRateLimit = {
    // min: 2,
    // max: 98 * 1000,
    defaultMin: 2,
    targetMin: 0,
    min: 0,
    max: 0,
    tip: 'targetBetweenMaxAndMin',
    over: false,
    auto: true,
    autoOver: false,
    isTarget: false,
  };

  videoEncoderInterfaceForm = this.fb.group({
    iGOP: ['', [Validators.required, Validators.min(this.limitRange.iGOP.min),
      Validators.max(this.limitRange.iGOP.max), isNumberJudge]],
    iMaxRate: [''],
    iMinRate: ['', isNumberJudge],
    iTargetRate: [''],
    iStreamSmooth: ['', [Validators.required, Validators.min(this.limitRange.iStreamSmooth.min),
      Validators.max(this.limitRange.iStreamSmooth.max), isNumberJudge]],
    sFrameRate: [''],
    sFrameRateIn: [''],
    sH264Profile: [''],
    sOutputDataType: [''],
    sRCMode: [''],
    sRCQuality: [''],
    sResolution: [''],
    sSVC: [''],
    sSmart: [''],
    sStreamType: [''],
    sVideoType: [''],
  });

  get iGOP(): FormControl {
    return this.videoEncoderInterfaceForm.get('iGOP') as FormControl;
  }

  get iMinRate(): FormControl {
    return this.videoEncoderInterfaceForm.get('iMinRate') as FormControl;
  }

  disabledDict = {
    sSmart: {
      disableKey: ['open'],
      disableValue: ['iGOP', 'sRCMode', 'sH264Profile', 'sSVC', 'iStreamSmooth', 'sRCQuality'],
    },
    sRCMode: {
      disableKey: ['CBR'],
      disableValue: ['sRCQuality'],
    },
    sOutputDataType: {
      disableKey: ['H.265'],
      disableValue: ['sH264Profile'],
    },
    All: {
      disableKey: [''],
      // 'sRCQuality', 'sH264Profile'
      disableValue: ['sVideoType', 'sSVC', 'iStreamSmooth'],
    },
  };

  private disabledOption: Array<DisabledOp> = [];

  streamDisable = {
    mainStream: {},
    subStream: {
      sOutputDataType: 'H.264',
      sSmart: 'close',
    },
    thirdStream: {
      sSmart: 'close',
    },
  };

  oldPara = {
    iStreamSmooth: NaN,
  };

  streamCapbility = {};

  private disabledSet = {};
  RCQualityLevel2Num = {};

  ngOnInit() {
    this.isChrome = this.ieCss.getChromeBool();
    this.isIE = this.ieCss.getIEBool();
    this.employee.hire(this.epBank[0]);
    this.cfgService.getDefaultPara(4).subscribe(
      res => {
        this.resError.analyseRes(res);
        const rst: VideoDefaultPara = JSON.parse(res.toString());
        this.encoderMenuGroups = rst.static;
        this.disabledOption = rst.disabled;
        this.streamCapbility = rst.dynamic;
        for (const item of this.objectKeys(rst.dynamic)) {
          this.dynamicList.push(item);
        }
        this.layoutArray = rst.layout['encoder'];
        this.employee.doTask(this.epBank[0].name, this.epBank[0].task[0], true, true);
      },
      err => {
        this.logger.error(err, 'ngOnInit:getDefaultPara:');
        this.employee.doTask(this.epBank[0].name, this.epBank[0].task[0], false, true);
      }
    );

    this.updateVideoEncoder();

    this.employee.observeTask(this.epBank[0].name, 5000)
      .then(
        () => {
          this.updateCapbility();
          this.startObserveChange();
        }
      )
      .catch(
        () => {
          this.tips.showInitFail();
        }
      );
  }

  ngOnDestroy() {
    if (this.formChange) {
      this.formChange.unsubscribe();
    }
    if (this.ctOb) {
      this.ctOb.unsubscribe();
    }
  }

  startObserveChange() {
    this.formChange = this.videoEncoderInterfaceForm.valueChanges.subscribe(
      change => {
        if (!this.locker.getLocker('formChange', '', false, false)) {
          this.recoveryEnabled();
          this.disabledSet = {};
          for (const item of this.disabledOption) {
            if (item.name === 'unspport') {
              for (const disabKey of this.objectKeys(item.options)) {
                item.options[disabKey] ? this.videoEncoderInterfaceForm.get(disabKey).setValue(item.options[disabKey]) : null;
                this.disabledSet[disabKey] = true;
              }
            } else {
              const formVal = this.videoEncoderInterfaceForm.value[item.name];
              if (item.options[formVal] !== undefined) {
                for (const disabKey of this.objectKeys(item.options[formVal])) {
                  const shortItem = item.options[formVal][disabKey];
                  shortItem ? this.videoEncoderInterfaceForm.get(disabKey).setValue(shortItem) : null;
                  this.disabledSet[disabKey] = true;
                }
              }
            }
          }
          this.setDisabled();
          if (this.locker) {
            this.locker.releaseLocker('formChange');
          }
        }
      }
    );
    this.videoEncoderInterfaceForm.get('sStreamType').setValue(this.videoEncoderInterfaceForm.value.sStreamType);
  }

  updateCapbility() {
    this.removeLastDynamicMenu();
    for (const key of this.objectKeys(this.streamCapbility)) {
      if (this.videoEncoderInterfaceForm.value[key]) {
        for (const secondKey of this.objectKeys(this.streamCapbility[key])) {
          if (this.videoEncoderInterfaceForm.value[key] === secondKey) {
            for (const thirdKey of this.objectKeys(this.streamCapbility[key][secondKey])) {
              this.dynamicMenu.push(thirdKey);
              this.encoderMenuGroups[thirdKey] = this.streamCapbility[key][secondKey][thirdKey];
            }
          }
        }
      }
    }
  }

  removeLastDynamicMenu() {
    for (const item of this.dynamicMenu) {
      if (this.encoderMenuGroups[item]) {
        delete this.encoderMenuGroups[item];
      }
    }
    this.dynamicMenu = [];
  }

  recoveryEnabled() {
    for (const key of this.objectKeys(this.disabledSet)) {
      this.videoEncoderInterfaceForm.get(key).enable();
    }
  }

  setDisabled() {
    for (const key of this.objectKeys(this.videoEncoderInterfaceForm.value)) {
      if (this.disabledSet[key]) {
        this.videoEncoderInterfaceForm.get(key).disable();
      } else {
        this.videoEncoderInterfaceForm.get(key).enable();
      }
    }
  }

  updateVideoEncoder() {
    this.cfgService.getVideoEncoderInterface(this.streamType).subscribe(
      (res: VideoEncoderInterface) => {
        this.resError.analyseRes(res);
        this.rawSmart = res.sSmart;
        this.lastSmart = res.sSmart;
        this.locker.getLocker('onDynamicChange', '', false, false);
        this.videoEncoderInterfaceForm.patchValue(res);
        this.locker.releaseLocker('onDynamicChange');
        this.onAutoChange();
        this.employee.doTask(this.epBank[0].name, this.epBank[0].task[1], true, null);
      },
      err => {
        this.logger.error(err, 'updateVideoEncoder:');
        this.employee.doTask(this.epBank[0].name, this.epBank[0].task[1], false, null);
      });
  }

  onDynamicChange(newValue: string, key: string) {
    if (this.locker.isLock('onDynamicChange')) {
      return;
    }
    if (!this.pfs.isInArrayString(this.dynamicList, key)) {
      return;
    }
    if (key === 'sStreamType') {
      if (!this.streamType.match(newValue)) {
        this.streamType = newValue;
        this.updateVideoEncoder();
        this.updateCapbility();
      }
    } else if (key === 'sSmart') {
      if (!this.lastSmart.match(newValue)) {
        this.lastSmart = newValue;
        this.updateCapbility();
        if (newValue.match('open')) {
          this.bitRangeCheck();
          if (!this.checkFrameRate4bug(this.videoEncoderInterfaceForm.value.sFrameRate)) {
            this.videoEncoderInterfaceForm.get('sFrameRate').setValue('4');
          }
        }
      }
    }
  }

  onSubmit() {
    if (this.rawSmart !== this.videoEncoderInterfaceForm.value.sSmart && this.videoEncoderInterfaceForm.value.sSmart) {
      if (this.rawSmart === 'close') {
        this.tips.showCTip('samrtOpenTip');
      } else {
        this.tips.showCTip('samrtCloseTip');
      }
      this.ctOb = this.tips.ctAction.subscribe(
        change => {
          if (change === 'onYes') {
            this.submitFunc();
            this.tips.setCTPara('close');
          } else if (change === 'onNo') {
            this.ctOb.unsubscribe();
            this.ctOb = null;
          }
        }
      );
    } else {
      this.submitFunc();
    }
  }

  submitFunc() {
    if (this.locker.getLocker('submitFunc', 'wait4config', true, true)) {
      return;
    }
    this.checkEnableOnSubmit(true);
    this.streamSwitch(true);
    this.pfs.formatInt(this.videoEncoderInterfaceForm.value);
    this.cfgService.setVideoEncoderInterface(this.videoEncoderInterfaceForm.value).subscribe(
      res => {
        this.resError.analyseRes(res);
        this.rawSmart = res.sSmart;
        this.locker.getLocker('onDynamicChange', '', false, false);
        this.videoEncoderInterfaceForm.patchValue(res);
        this.locker.releaseLocker('onDynamicChange');
        this.tips.showSaveSuccess();
        this.los.updateResolution();
        this.checkEnableOnSubmit(false);
        this.streamSwitch(false);
        this.locker.releaseLocker('submitFunc');
      },
      err => {
        this.logger.error(err, 'submitFunc');
        this.checkEnableOnSubmit(false);
        this.streamSwitch(false);
        this.tips.showSaveFail();
        this.locker.releaseLocker('submitFunc');
      });
  }

  rangeAndNumber(controlName: string, change: number) {
    if (!isNaN(this.oldPara[controlName]) && this.oldPara[controlName] !== change) {
      if (change > this.limitRange[controlName].max) {
        change = this.limitRange[controlName].max;
      } else if (change < this.limitRange[controlName].min) {
        change = this.limitRange[controlName].min;
      }
      this.oldPara[controlName] = change;
      this.videoEncoderInterfaceForm.get(controlName).setValue(change);
    } else {
      this.oldPara[controlName] = change;
    }
  }

  bitRangeCheck() {
    if (!this.locker.getLocker('bitRange', '', false)) {
      if (this.videoEncoderInterfaceForm.value.sSmart === 'open') {
        this.iMinRateLimit.targetMin = this.iMinRateLimit.defaultMin;
        if (Number(this.videoEncoderInterfaceForm.value.iMaxRate) < this.iMinRateLimit.targetMin) {
          let maxBitrateMenu = [];
          for (const item of this.encoderMenuGroups) {
            if (item.name === 'maxRate') {
              maxBitrateMenu = item.items;
            }
          }
          for (const item of maxBitrateMenu) {
            if (item >= this.iMinRateLimit.targetMin) {
              this.videoEncoderInterfaceForm.get('iMaxRate').setValue(item);
              break;
            }
          }
        }
        // check limit range
        this.iMinRateLimit.min = Math.max(this.iMinRateLimit.defaultMin, Number(this.videoEncoderInterfaceForm.value.iMaxRate) / 8);
        this.iMinRateLimit.max = Number(this.videoEncoderInterfaceForm.value.iMaxRate);
        const num = Number(this.videoEncoderInterfaceForm.value.iMinRate);
        this.iMinRateLimit.over = num > this.iMinRateLimit.max || num < this.iMinRateLimit.min;
        if (!this.iMinRateLimit.auto) {
          const tNum = Number(this.videoEncoderInterfaceForm.value.iTargetRate);
          this.iMinRateLimit.autoOver = tNum < num || tNum > Number(this.videoEncoderInterfaceForm.value.iMaxRate);
        } else {
          const averageRate = Math.ceil((num + Number(this.videoEncoderInterfaceForm.value.iMaxRate)) / 2);
          this.videoEncoderInterfaceForm.get('iTargetRate').enable();
          this.videoEncoderInterfaceForm.get('iTargetRate').setValue(averageRate);
          this.onAutoChange();
          this.iMinRateLimit.autoOver = false;
        }
      } else {
        this.iMinRateLimit.targetMin = 0;
        this.iMinRateLimit.over = false;
        this.iMinRateLimit.autoOver = false;
      }
      this.locker.releaseLocker('bitRange');
    }
  }

  onAutoChange() {
    if (this.iMinRateLimit.auto) {
      const averageRate = Math.ceil(
        (Number(this.videoEncoderInterfaceForm.value.iMinRate) + Number(this.videoEncoderInterfaceForm.value.iMaxRate)) / 2);
      this.videoEncoderInterfaceForm.get('iTargetRate').enable();
      this.videoEncoderInterfaceForm.get('iTargetRate').setValue(averageRate);
      this.videoEncoderInterfaceForm.get('iTargetRate').disable();
    } else {
      this.videoEncoderInterfaceForm.get('iTargetRate').enable();
    }
  }

  checkEnableOnSubmit(start: boolean) {
    if (start) {
      if (this.videoEncoderInterfaceForm.value.sSmart === 'open') {
        if (this.iMinRateLimit.isTarget) {
          this.iMinRateLimit.auto ? this.videoEncoderInterfaceForm.get('iTargetRate').enable() : null;
        } else {
          this.videoEncoderInterfaceForm.get('iTargetRate').disable();
        }
      } else {
        this.videoEncoderInterfaceForm.get('iTargetRate').disable();
        this.videoEncoderInterfaceForm.get('iMinRate').disable();
      }
    } else {
      this.videoEncoderInterfaceForm.get('iMinRate').enable();
      this.onAutoChange();
    }
  }

  // limit by sFrameRateIn not for bug
  checkFrameRate4bug(item: string) {
    if (this.videoEncoderInterfaceForm.value.sSmart === 'open') {
      const parNum = this.pfs.string2Number(item);
      if (parNum <= 2) {
        return false;
      }
    }
    return true;
  }

  specialCheck(key: string) {
    const specialList = ['iMaxRate'];
    for (const item of specialList) {
      if (item === key) {
        return true;
      }
    }
    return false;
  }

  checkDynamicOption(deal: any) {
    const returnList = [];
    let min = NaN;
    let max = NaN;
    if (deal['dynamicRange']['max']) {
      max = this.pfs.string2Number(this.videoEncoderInterfaceForm.get(deal['dynamicRange']['max']).value);
      if (deal['dynamicRange']['maxRate']) {
        max *= this.pfs.string2Number(deal['dynamicRange']['maxRate']);
      }
    }
    if (deal['dynamicRange']['min']) {
      min = this.pfs.string2Number(this.videoEncoderInterfaceForm.get(deal['dynamicRange']['min']).value);
      if (deal['dynamicRange']['minRate']) {
        min *= this.pfs.string2Number(deal['dynamicRange']['minRate']);
      }
    }
    for (const item of deal['options']) {
      const testNum = this.pfs.string2Number(item.toString());
      if (isNaN(min) ? true : testNum >= min) {
        if (isNaN(max) ? true : testNum <= max) {
          returnList.push(item);
        }
      }
    }
    return returnList;
  }

  getDynamicRangeM(deal: any, key: string): number {
    if (deal['type']) {
      const type = deal['type'];
      if (deal[type][key]) {
        const rst = this.pfs.string2Number(this.videoEncoderInterfaceForm.get(deal[type][key]).value);
        const rate = deal[type][key + 'Rate'];
        return rst * rate;
      }
    } else {
      this.logger.error(deal, 'getDynamicRangeM:find no type:');
      return NaN;
    }
  }

  streamSwitch(swi: boolean) {
    let streamSelect = this.el.nativeElement.querySelector('.disabled-mark');
    if (!streamSelect) {
      this.logger.error('streamSwitch: no stream selector');
      return;
    }
    if (swi) {
      this.Re2.setAttribute(streamSelect, 'disabled', 'true');
    } else {
      this.Re2.removeAttribute(streamSelect, 'disabled');
    }
    streamSelect = null;
  }
}
