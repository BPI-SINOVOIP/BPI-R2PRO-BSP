import { Component, OnInit, AfterViewInit, OnDestroy, ViewChild, Input } from '@angular/core';
import { FormBuilder, Validators, FormControl } from '@angular/forms';

import { ConfigService } from 'src/app/config.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { TipsService } from 'src/app/tips/tips.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { FaceSettingOptions } from './FaceParaInterface';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-para-setting',
  templateUrl: './para-setting.component.html',
  styleUrls: ['./para-setting.component.scss']
})
export class ParaSettingComponent implements OnInit, AfterViewInit, OnDestroy {

  @Input() options: FaceSettingOptions;

  constructor(
    private cfg: ConfigService,
    private fb: FormBuilder,
    private resError: ResponseErrorService,
    private tips: TipsService,
    private pfs: PublicFuncService,
  ) { }

  private logger: Logger = new Logger('para-setting');

  detectType = [
    'open',
    'close',
    'schedule'
  ];

  detectType4Gate = [
    'open',
    'close'
  ];

  ParaForm = this.fb.group({
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
    sLiveDetectBeginTime: ['', Validators.required],
    sLiveDetectEndTime: ['', Validators.required],
    iNormalizedHeight: [''],
    iNormalizedWidth: [''],
  });

  get sLiveDetectBeginTime(): FormControl {
    return this.ParaForm.get('sLiveDetectBeginTime') as FormControl;
  }

  get sLiveDetectEndTime(): FormControl {
    return this.ParaForm.get('sLiveDetectEndTime') as FormControl;
  }

  private CheckPara: any;
  private paraChange: any;
  private isCheckPara: boolean = false;

  limitDict = {
    iFaceMinPixel: [0, 960, false],
    iDetectHeight: [0, 1280, false],
    iDetectWidth: [0, 960, false],
    iLeftCornerX: [0, 960, false],
    iLeftCornerY: [0, 1280, false],
    iFaceDetectionThreshold: [0, 100, false],
    iFaceRecognitionThreshold: [0, 100, false],
    iLiveDetectThreshold: [0, 100, false],
    iPromptVolume: [0, 100, false],
  };

  ngOnInit(): void {
    this.cfg.getFaceParaInterface().subscribe(
      res => {
        this.resError.analyseRes(res);
        this.ParaForm.patchValue(res);
      },
      err => {
        this.tips.showInitFail();
        this.logger.error(err, 'ngOnInit:getFaceParaInterface:');
      }
    );
  }

  ngAfterViewInit() {
    this.CheckPara = this.ParaForm.value;
    this.paraChange = this.ParaForm.valueChanges.subscribe(
      change => {
        // tslint:disable-next-line: forin
        if (!this.isCheckPara) {
          this.isCheckPara = true;
          for (const key in change) {
            if (this.limitDict[key]) {
              if (change[key] !== this.CheckPara[key]) {
                if (change[key] > this.limitDict[key][1] || change[key] < this.limitDict[key][0]) {
                  this.ParaForm.get(key).setValue(this.CheckPara[key]);
                  this.limitDict[key][2] = true;
                } else {
                  this.ParaForm.get(key).setValue(this.ParaForm.value[key]);
                  this.limitDict[key][2] = false;
                }
              } else {
                this.ParaForm.get(key).setValue(this.ParaForm.value[key]);
                this.limitDict[key][2] = false;
              }
            } else {
              this.ParaForm.get(key).setValue(this.ParaForm.value[key]);
            }
          }
          this.CheckPara = this.ParaForm.value;
          if (change.sLiveDetect === 'schedule') {
            this.ParaForm.get('sLiveDetectBeginTime').enable();
            this.ParaForm.get('sLiveDetectEndTime').enable();
          } else {
            this.ParaForm.get('sLiveDetectBeginTime').disable();
            this.ParaForm.get('sLiveDetectEndTime').disable();
          }
          this.isCheckPara = false;
        }
      }
    );
  }

  ngOnDestroy() {
    this.paraChange.unsubscribe();
  }

  onSubmit() {
    this.pfs.formatInt(this.ParaForm.value);
    this.cfg.putFaceParaInterface(this.ParaForm.value).subscribe(
      res => {
        this.resError.analyseRes(res);
        this.ParaForm.patchValue(res);
        this.tips.showSaveSuccess();
      },
      err => {
        this.logger.error(err, 'onSubmit:putFaceParaInterface:');
        this.tips.showSaveFail();
      }
    );
  }

}
