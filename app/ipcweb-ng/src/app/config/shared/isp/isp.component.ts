import { Component, OnInit, AfterViewInit, OnDestroy, ViewChild, ElementRef, Renderer2, Input } from '@angular/core';
import { ConfigService } from 'src/app/config.service';
import { FormBuilder, FormGroup, Validators, FormControl } from '@angular/forms';
import { ImageInterface, ScenarioInterface } from './ImageInterface';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { TipsService } from 'src/app/tips/tips.service';
import { BoolEmployee, EmployeeItem } from 'src/app/shared/func-service/employee.service';
import { isNumberJudge } from 'src/app/shared/validators/benumber.directive';
import { LockService } from 'src/app/shared/func-service/lock-service.service';
import Logger from 'src/app/logger';
import { prepareSyntheticListenerFunctionName } from '@angular/compiler/src/render3/util';
import { DiyHttpService } from 'src/app/shared/func-service/diy-http.service';

@Component({
  selector: 'app-isp',
  templateUrl: './isp.component.html',
  styleUrls: ['./isp.component.scss']
})
export class IspComponent implements OnInit, AfterViewInit, OnDestroy {

  @Input() option: any;
  @ViewChild('player', {static: true}) playerChild: any;
  @ViewChild('player', {read: ElementRef}) playerDom: ElementRef;

  constructor(
    private cfgService: ConfigService,
    private fb: FormBuilder,
    private Re2: Renderer2,
    private resError: ResponseErrorService,
    private tips: TipsService,
    private pfs: PublicFuncService,
    private el: ElementRef,
    private dhs: DiyHttpService,
  ) { }
  private logger: Logger = new Logger('isp');

  private lock = new LockService(this.tips);
  private employee = new BoolEmployee();
  private initFlag = false;
  transferControlName = this.pfs.formName2translateName;
  checkType = this.pfs.checkMenuType;
  getRange = this.pfs.getRange;
  layoutKey: string = 'layout';
  staticKey: string = 'static';
  dynamicKey: string = 'dynamic';
  rangeKey: string = 'range';
  typeKey: string = 'type';
  disabledKey: string = 'disabled';
  pageLayoutKey = 'pageLayout';
  private lastDynamicKey = 'lastDynamic';
  private lastRangeKey = 'lastRange';
  private isChange: boolean = false;
  private ctObserver: any;
  private isInit: boolean = false;
  private isViewInit: boolean = false;
  private disabledBank = {};
  private setDBank = {};
  private disableInterestList = [];
  scenario = '';
  selectId: number;
  selectedCard: string = '';
  restartList = ['sGrayScaleMode'];
  ctDict = {
    // BLC: {
    //   sHDR: {
    //     old: 'close',
    //     tip: 'HDRAlterTip',
    //     // tip: 'HDRAlterTipNoReboot',
    //     isReboot: true,
    //   }
    // },
    imageEnhancement: {
      sDehaze: {
        old: 'close',
        tip: 'dehazeAlterTip',
        isReboot: false,
      }
    }
  };

  cardList: Array<string> = [
    'image_adjustment',
    'image_exposure',
    'image_night_to_day',
    'image_blc',
    'image_white_blance',
    'image_enhancement',
    'image_video_adjustment'
  ];

  groupNameDict =  {
    image_adjustment: 'imageAdjustment',
    image_exposure: 'exposure',
    image_night_to_day: 'nightToDay',
    image_blc: 'BLC',
    image_white_blance: 'whiteBlance',
    image_enhancement: 'imageEnhancement',
    image_video_adjustment: 'videoAdjustment',
  };

  group2path = {
    imageAdjustment: 'adjustment',
    exposure: 'exposure',
    nightToDay: 'night-to-day',
    BLC: 'blc',
    whiteBlance: 'white-blance',
    imageEnhancement: 'enhancement',
    videoAdjustment: 'video-adjustment',
  };

  capDict: any = {
    image_adjustment: {},
    image_exposure: {},
    image_night_to_day: {},
    image_blc: {},
    image_white_blance: {},
    image_enhancement: {},
    image_video_adjustment: {},
  };

  scenarioMenu: Array<string> = [
    'normal',
    'backlight',
    'frontlight',
    'lowIllumination',
    'custom1',
    'custom2',
  ];

  playerOption = {
    isReshape: true,
    speName: 'isp',
  };

  videoInputModeMenu: Array<string> = [
    'close',
    '1920*1080@25fps',
    '2048*1536@20fps',
  ];

  optionsTransfer = {
    iHDRLevel: {
      1: 'auto',
      2: 'weak',
      3: 'moderate',
      4: 'strong',
      5: 'super',
    }
  };

  optionsFilter = {
    sWhiteBlanceStyle: [
      'lockingWhiteBalance',
      'fluorescentLamp',
      'incandescent',
      'warmLight',
      'naturalLight'
    ],
    iHDRLevel: [
      5,
    ],
    sHDR: [
      'HDR3'
    ]
  };

  itemFilter = [
    'iImageRotation',
    'sFEC'
  ]

  scenarioForm = this.fb.group({
    sScenario: ['']
  });

  objectKeys = this.pfs.objectKeys;

  imageForm = this.fb.group({
    id: [''],
    imageAdjustment: this.fb.group({
      iBrightness: [''],
      iContrast: [''],
      iSaturation: [''],
      iSharpness: [''],
      iHue: ['']
    }),
    exposure: this.fb.group({
      sIrisType: [''],
      iAutoIrisLevel: [''],
      sExposureTime: [''],
      iExposureGain: [''],
      sGainMode: [''],
      sExposureMode: [''],
    }),
    nightToDay: this.fb.group({
      sNightToDay: [''],
      iNightToDayFilterLevel: [''],
      iNightToDayFilterTime: [''],
      sBeginTime: [''],
      sEndTime: [''],
      // modify st et time switch time name
      sDawnTime: [''],
      sDuskTime: [''],
      sIrcutFilterAction: [''],
      sOverexposeSuppressType: [''],
      sOverexposeSuppress: [''],
      sFillLightMode: [''],
      sBrightnessAdjustmentMode: [''],
      iLightBrightness: [''],
      iDistanceLevel: [''],
    }),
    BLC: this.fb.group({
      sWDR: [''],
      iWDRLevel: [''],
      sHDR: [''],
      iHDRLevel: [''],
      sHLC: [''],
      iHLCLevel: [''],
      iDarkBoostLevel: [''],
      sBLCRegion: [''],
      iBLCStrength: [''],
      iBLCRegionHeight: [''],
      iBLCRegionWidth: [''],
      iPositionX: [''],
      iPositionY: ['']
    }),
    whiteBlance: this.fb.group({
      sWhiteBlanceStyle: [''],
      iWhiteBalanceBlue: [''],
      iWhiteBalanceGreen: [''],
      iWhiteBalanceRed: [''],
    }),
    imageEnhancement: this.fb.group({
      sNoiseReduceMode: [''],
      iDenoiseLevel: [''],
      iSpatialDenoiseLevel: [''],
      iTemporalDenoiseLevel: [''],
      sDehaze: [''],
      iDehazeLevel: [''],
      sDIS: [''],
      sGrayScaleMode: [''],
      iImageRotation: [''],
      sFEC: [''],
      sDistortionCorrection: [''],
      iLdchLevel: [''],
      iFecLevel: [''],
    }),
    videoAdjustment: this.fb.group({
      sImageFlip: [''],
      sSceneMode: [''],
      sPowerLineFrequencyMode: [''],
    })
  });
  get imageAdjustment(): FormGroup {
    return this.imageForm.get('imageAdjustment') as FormGroup;
  }
  get exposure(): FormGroup {
    return this.imageForm.get('exposure') as FormGroup;
  }
  get nightToDay(): FormGroup {
    return this.imageForm.get('nightToDay') as FormGroup;
  }
  get BLC(): FormGroup {
    return this.imageForm.get('BLC') as FormGroup;
  }
  get whiteBlance(): FormGroup {
    return this.imageForm.get('whiteBlance') as FormGroup;
  }
  get imageEnhancement(): FormGroup {
    return this.imageForm.get('imageEnhancement') as FormGroup;
  }
  get videoAdjustment(): FormGroup {
    return this.imageForm.get('videoAdjustment') as FormGroup;
  }

  private urlChange: any;

  ngOnInit() {
    this.employee.hireCap();

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
              (error) => {
                this.logger.error(error, 'ngOnInit:getUrl:then:catch');
                this.tips.setRbTip('initPlayerFailFreshPlease');
              }
            );
        }
      )
      .catch(
        (error) => {
          this.logger.error(error, 'ngOnInit:getUrl:catch');
          this.tips.setRbTip('getVideoUrlFail');
        }
      );

    this.cfgService.getScenarioInterface().subscribe((res: ScenarioInterface) => {
      this.resError.analyseRes(res);
      this.scenarioForm.patchValue(res);
      this.scenario = res.sScenario;
    });

    this.cfgService.getImageInterface().subscribe((res: ImageInterface) => {
      // disalbed follow 2 when not check status
      // this.ctDict.BLC.sHDR.old = res.BLC.sHDR;
      this.ctDict.imageEnhancement.sDehaze.old = res.imageEnhancement.sDehaze;
      this.setAlarmTip(res);
      this.imageForm.patchValue(res);
      this.employee.doCap(1, 1);
    });

    this.cfgService.getDefaultPara('isp').subscribe(
      res => {
        for (const item of this.cardList) {
          this.capDict[item] = JSON.parse(res[item]);
          this.capDict[item][this.lastDynamicKey] = [];
          this.capDict[item][this.lastRangeKey] = {};
          this.capDict[item][this.pageLayoutKey] = this.capDict[item].static;
          if (this.capDict[item][this.disabledKey]) {
            for (const pat0 of this.capDict[item][this.disabledKey]) {
              this.disabledBank[pat0.name] = pat0.options;
              this.disableInterestList.push(pat0.name);
            }
          }
        }
        this.employee.doCap(0, 1);
      }
    );

    this.employee.getCapObserver(5000)
      .then(
        () => {
          for (const item of this.cardList) {
            if (this.capDict[item][this.staticKey]) {
              for (const partKey of this.objectKeys(this.capDict[item].static)) {
                if (this.capDict[item].static[partKey][this.typeKey] === this.rangeKey) {
                  this.capDict[item][this.lastRangeKey][partKey] = this.imageForm.value[this.groupNameDict[item]][partKey];
                  this.imageForm.get(this.groupNameDict[item]).get(partKey).setValidators(
                    [Validators.min(this.capDict[item].static[partKey][this.rangeKey].min),
                     Validators.max(this.capDict[item].static[partKey][this.rangeKey].max),
                     Validators.required,
                     isNumberJudge
                    ]);
                }
              }
            }
            if (this.capDict[item][this.dynamicKey]) {
              for (const firstKey of this.objectKeys(this.capDict[item].dynamic)) {
                for (const secondKey of this.objectKeys(this.capDict[item].dynamic[firstKey])) {
                  for (const thirdKey of this.objectKeys(this.capDict[item].dynamic[firstKey][secondKey])) {
                    if (this.capDict[item].dynamic[firstKey][secondKey][thirdKey][this.typeKey] === this.rangeKey) {
                      // console.log(firstKey, secondKey, thirdKey);
                      this.capDict[item][this.lastRangeKey][thirdKey] = this.imageForm.value[this.groupNameDict[item]][thirdKey];
                      this.imageForm.get(this.groupNameDict[item]).get(thirdKey).setValidators(
                        [Validators.min(this.capDict[item].dynamic[firstKey][secondKey][thirdKey][this.rangeKey].min),
                         Validators.max(this.capDict[item].dynamic[firstKey][secondKey][thirdKey][this.rangeKey].max),
                         Validators.required,
                         isNumberJudge
                        ]);
                    }
                  }
                }
              }
            }
          }
          this.initFlag = true;
          this.updateDynamicCap();
          // forbid Scenario
          this.scenarioForm.get('sScenario').disable();
          this.isInit = true;
        }
      )
      .catch(
        (error) => {
          this.logger.error(error, 'ngOnInit:getCapObserver:catch:');
          this.tips.showInitFail();
        }
      );
  }

  ngAfterViewInit() {
    this.isViewInit = true;
    const playerButton = this.playerDom.nativeElement.querySelectorAll('.blue-btn');
    for (const btn of playerButton) {
      this.Re2.setAttribute(btn, 'hidden', 'true');
    }
    // this.playerChild.hideDrawer();
    // this.playEntry(this.option['src']);
  }

  ngOnDestroy() {
    if (this.ctObserver) {
      this.ctObserver.unsubscribe();
    }
    this.stopPlayer();
  }

  rmDynamicCap() {
    for (const title of this.cardList) {
      for (const key of this.capDict[title][this.lastDynamicKey]) {
        delete this.capDict[title][this.pageLayoutKey][key];
      }
      this.capDict[title][this.lastDynamicKey] = [];
    }
  }

  updateDynamicCap(cardTitle: string = null, layoutItem: string = null, change: any = null) {
    if (!this.initFlag) {
      return;
    }
    if (this.checkAlarmTip(cardTitle, layoutItem)) {
      return;
    }
    this.rmDynamicCap();
    for (const formName of this.cardList) {
      for (const firstKey of this.objectKeys(this.capDict[formName][this.staticKey])) {
        this.checkDynamicCap(formName, firstKey);
      }
    }
    if (cardTitle && layoutItem) {
      this.submitOne(this.groupNameDict[cardTitle]);
    }
    this.checkDisabledCap(layoutItem, change);
  }

  checkDynamicCap(formName: string, firstKey: string) {
    const groupName = this.groupNameDict[formName];
    if (!this.capDict[formName][this.dynamicKey]) {
      return;
    }
    if (this.capDict[formName][this.dynamicKey][firstKey]) {
      for (const secondKey of this.objectKeys(this.capDict[formName][this.dynamicKey][firstKey])) {
        if (this.imageForm.value[groupName][firstKey] === secondKey) {
          for (const thirdKey of this.objectKeys(this.capDict[formName][this.dynamicKey][firstKey][secondKey])) {
            this.capDict[formName][this.lastDynamicKey].push(thirdKey);
            this.capDict[formName][this.pageLayoutKey][thirdKey] = this.capDict[formName][this.dynamicKey][firstKey][secondKey][thirdKey];
            this.checkDynamicCap(formName, thirdKey);
          }
        }
      }
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
        if (this.isViewInit) {
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

  setIspCanvas(): void {
    this.playerChild.reshapeCanvas();
  }

  onScenarioChange(value: string) {
    if (!this.isInit) {
      return;
    }
    if (value !== this.scenario) {
      this.scenario = value;
      this.scenarioForm.get('sScenario').setValue(value);
      this.cfgService.setScenarioInterface(this.scenarioForm.value).subscribe(
        res => {
          this.resError.analyseRes(res);
          this.cfgService.getImageInterface().subscribe((dat: ImageInterface) => {
            this.setAlarmTip(dat);
            this.imageForm.patchValue(dat);
          });
        },
        err => {
          this.logger.error(err, 'onScenarioChange:');
        });
    }
  }

  observeCTAction(groupName: string, controlName: string, isReoot: boolean = false) {
    if (!this.ctObserver) {
      this.ctObserver = this.tips.ctAction.subscribe(
        action => {
          if (action === 'onYes') {
            this.submitOne(groupName, isReoot);
            if (!isReoot) {
              this.ctObserver.unsubscribe();
              this.tips.setCTPara('close');
              this.ctObserver = null;
            }
          } else if (action === 'onNo') {
            this.imageForm.get(groupName).get(controlName).setValue(this.ctDict[groupName][controlName].old);
            this.ctObserver.unsubscribe();
            this.ctObserver = null;
          }
        }
      );
    }
  }

  playEntry(src: string) {
    if (this.isViewInit && src) {
      this.playerChild.displayUrl = src;
      this.playerChild.bigBtnPlay();
    }
  }

  pausePlayer() {
    if (this.isViewInit && this.playerChild) {
    this.playerChild.diyPause();
    this.playerChild.destroyWhenSwitch();
    }
  }

  stopPlayer() {
    if (this.isViewInit && this.playerChild && this.playerChild.isPlaying) {
      this.playerChild.diyStop();
    }
  }

  onSelectCard(cardTitle: string) {
    if (this.selectedCard === cardTitle) {
      this.Re2.setStyle(document.getElementById(cardTitle), 'display', 'none');
      this.selectedCard = '';
    } else {
      if (this.selectedCard !== '') {
        this.Re2.setStyle(document.getElementById(this.selectedCard), 'display', 'none');
      }
      this.Re2.setStyle(document.getElementById(cardTitle), 'display', 'block');
      this.selectedCard = cardTitle;
    }
  }

  isSelected(cardTitle: string) {
    return cardTitle === this.selectedCard;
  }

  onRangeChange(change: number, formKey: string, controlName: string) {
    if (change !== this.capDict[formKey][this.lastRangeKey][controlName]) {
      this.capDict[formKey][this.lastRangeKey][controlName] = change;
      this.imageForm.get(this.groupNameDict[formKey]).get(controlName).setValue(change);
    }
  }

  onSubmitPart(cardTitle: string, layoutItem: string, type: string) {
    if (!this.isInit) {
      return;
    }
    const groupName = this.groupNameDict[cardTitle];
    switch (type) {
      case 'number':
        const nowNum = this.imageForm.value[groupName][layoutItem];
        const minNum = this.capDict[cardTitle][this.pageLayoutKey][layoutItem].range.min;
        if (!Number(nowNum)) {
          this.imageForm.get(groupName).get(layoutItem).setValue(minNum);
          break;
        }
        const maxNum = this.capDict[cardTitle][this.pageLayoutKey][layoutItem].range.max;
        if (nowNum < minNum) {
          this.imageForm.get(groupName).get(layoutItem).setValue(minNum);
        } else if (nowNum > maxNum) {
          this.imageForm.get(groupName).get(layoutItem).setValue(maxNum);
        }
        break;
      case 'time':
        const st = 'sBeginTime';
        const et = 'sEndTime';
        if (this.imageForm.value['nightToDay'][et].match(':') && this.imageForm.value['nightToDay'][st].match(':')) {
          const start = this.imageForm.value['nightToDay'][st].split(':');
          const end = this.imageForm.value['nightToDay'][et].split(':');
          const startSeconds = Number(start[0]) * 3600 + Number(start[1]) * 60 + Number(start[2]);
          const endSeconds = Number(end[0]) * 3600 + Number(end[1]) * 60 + Number(end[2]);
          if (endSeconds > startSeconds && startSeconds + 10 > endSeconds) {
            this.tips.setRbTip('intervalTimeShouldOver10');
            return;
          }
        } else {
          this.tips.setRbTip('intervalTimeShouldOver10');
          return;
        }
        break;
      case 'range':
        break;
    }
    this.submitOne(groupName);
  }

  submitOne(groupName: string, isReboot: boolean = false, isAppRestart = false) {
    if (!this.isInit || this.lock.checkLock('submitOne')) {
      return;
    }
    this.lock.lock('submitOne');
    this.pfs.formatInt(this.imageForm.value[groupName]);
    const path = this.group2path[groupName];
    this.cfgService.setImageInterfacePart(this.imageForm.value[groupName], path, this.imageForm.value['id']).subscribe(
      res => {
        this.resError.analyseRes(res, 'saveFail');
        this.setAlarmTip(res, groupName);
        this.imageForm.get(groupName).patchValue(res);
        if (isReboot) {
          this.tips.setCTPara('restart');
        } else if (isAppRestart) {
          this.tips.setRbTip('appRestart');
        } else {
          this.tips.showSaveSuccess();
        }
        this.lock.unlock('submitOne');
      },
      err => {
        if (isReboot) {
          this.tips.setCTPara('close');
        }
        this.tips.showSaveFail();
        this.lock.unlock('submitOne');
      }
    );
  }

  setAlarmTip(res: any, groupName: string = '') {
    if (groupName && this.ctDict[groupName]) {
      for (const secondK of this.objectKeys(this.ctDict[groupName])) {
        if (res[secondK]) {
          this.ctDict[groupName][secondK].old = res[secondK];
        }
      }
    } else if (!groupName) {
      for (const firstK of this.objectKeys(this.ctDict)) {
        if (res[firstK]) {
          for (const secondK of this.objectKeys(this.ctDict[firstK])) {
            if (res[firstK][secondK]) {
              this.ctDict[firstK][secondK].old = res[firstK][secondK];
            }
          }
        }
      }
    }
  }

  checkAlarmTip(cardTitle: string = null, layoutItem: string = null): boolean {
    if (!this.isInit) {
      return false;
    }
    const groupName = this.groupNameDict[cardTitle];
    if (this.ctObserver) {
      return true;
    } else if (!groupName) {
      return false;
    } else if (!this.ctDict[groupName]) {
      return false;
    } else if (!this.ctDict[groupName][layoutItem]) {
      return false;
    }
    if (this.imageForm.value[groupName][layoutItem] !== this.ctDict[groupName][layoutItem].old) {
      let ctTip = '';
      if (typeof(this.ctDict[groupName][layoutItem].tip) === 'string') {
        ctTip = this.ctDict[groupName][layoutItem].tip;
      } else if (this.ctDict[groupName][layoutItem].tip instanceof Object) {
        ctTip = this.ctDict[groupName][layoutItem].tip[this.imageForm.value[groupName][layoutItem]];
      } else {
        this.tips.setRbTip('capbilityError');
        return false;
      }
      if (ctTip) {
        this.tips.showCTip(ctTip);
        this.observeCTAction(groupName, layoutItem, this.ctDict[groupName][layoutItem].isReboot);
        return true;
      } else {
        this.tips.setRbTip('capbilityError');
      }
    }
    return false;
  }

  disableSelecotor(idName: string) {
    const item = document.getElementById(idName);
    if (item) {
      this.Re2.setAttribute(item, 'disabled', 'true');
    }
  }

  enableSelector(idName: string) {
    const item = document.getElementById(idName);
    if (item) {
      this.Re2.removeAttribute(item, 'disabled', 'true');
    }
  }

  checkSelectorEnabled(layoutItem: string, change: any) {
    if (this.disabledBank[layoutItem]) {
      for (const dKey of this.objectKeys(this.disabledBank[layoutItem])) {
        if (change === dKey) {
          for (const sKey of this.objectKeys(this.disabledBank[layoutItem][dKey])) {
            this.setDBank[sKey] = true;
            this.disableSelecotor(layoutItem);
          }
        } else {
          for (const sKey of this.objectKeys(this.disabledBank[layoutItem][dKey])) {
            if (this.setDBank[sKey]) {
              this.setDBank[sKey] = false;
              this.enableSelector(sKey);
            }
          }
        }
      }
    }
  }

  lockSetDBank() {
    for (const formName of this.cardList) {
      const groupName = this.groupNameDict[formName];
      for (const firstKey of this.objectKeys(this.imageForm.value[groupName])) {
        if (this.pfs.isInArrayString(this.objectKeys(this.setDBank), firstKey)) {
          this.imageForm.get(groupName).get(firstKey).disable();
          this.setDBank[firstKey] = groupName;
        }
      }
    }
  }

  releaseSetDBank() {
    for (const sKey of this.objectKeys(this.setDBank)) {
      if (this.imageForm.get(this.setDBank[sKey].toString())) {
        this.imageForm.get(this.setDBank[sKey]).get(sKey).enable();
      }
    }
    this.setDBank = {};
  }

  checkDisabledCap(itemName: string = null, change: any = null) {
    this.releaseSetDBank();
    for (const formName of this.cardList) {
      const groupName = this.groupNameDict[formName];
      for (const firstKey of this.objectKeys(this.imageForm.value[groupName])) {
        if (this.pfs.isInArrayString(this.disableInterestList, firstKey)) {
          if (firstKey === itemName) {
            for (const dKey of this.objectKeys(this.disabledBank[firstKey])) {
              if (dKey === change) {
                for (const sKey of this.objectKeys(this.disabledBank[firstKey][dKey])) {
                  this.setDBank[sKey] = true;
                }
              }
            }
          } else {
            for (const dKey of this.objectKeys(this.disabledBank[firstKey])) {
              if (dKey === this.imageForm.value[groupName][firstKey]) {
                for (const sKey of this.objectKeys(this.disabledBank[firstKey][dKey])) {
                  this.setDBank[sKey] = true;
                }
              }
            }
          }
        }
      }
    }
    this.lockSetDBank();
  }

  htmlOptionTransfer(selector: string, op: any) {
    op = op.toString();
    if (this.optionsTransfer[selector] && this.optionsTransfer[selector][op]) {
      return this.optionsTransfer[selector][op];
    }
    return op;
  }

  htmlOptionFilter(selector: string, list: Array<any>) {
    if (!this.optionsFilter[selector]) {
      return list;
    }
    const rst = [];
    for (const item of list) {
      if (this.optionsFilter[selector].indexOf(item) < 0) {
        rst.push(item);
      }
    }
    return rst;
  }

  isItemEnable(layoutItem: any) {
    let rst = true;
    for (const lit of this.itemFilter) {
      if (lit == layoutItem) {
        rst = false;
        break;
      }
    }
    return rst;
  }
}
