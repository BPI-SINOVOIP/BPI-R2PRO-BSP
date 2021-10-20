import { Component, OnInit, Input, ViewChild } from '@angular/core';

import { ConfigService } from 'src/app/config.service';
import { AudioInterface } from './AudioInterface';
import { FormBuilder, FormControl } from '@angular/forms';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { TipsService } from 'src/app/tips/tips.service';
import { isNumberJudge } from 'src/app/shared/validators/benumber.directive';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-config-audio',
  templateUrl: './config-audio.component.html',
  styleUrls: ['./config-audio.component.scss']
})
export class ConfigAudioComponent implements OnInit {
  // @Input() selectedMenuItem: string;

  constructor(
    private cfgService: ConfigService,
    private fb: FormBuilder,
    private resError: ResponseErrorService,
    private tips: TipsService,
    private pfs: PublicFuncService,
  ) { }

  private logger: Logger = new Logger('layout-service');

  audioCodecMenu: Array<string> = [
    'G.722.1',
    'G.711ulaw',
    'MP2',
    'G.726',
    'PCM'
  ];

  audioInputMenu: Array<string> = [
    'micIn',
    'lineIn'
  ];

  noiseFilteringMenu: Array<string> = [
    'open',
    'close'
  ];

  sampleRateMenu: Array<number> = [
    16000,
    32000,
    44100,
    48000
  ];

  BitRateMenu: Array<number> = [
    16000,
    32000,
    64000
  ];

  MP2BitRateMenu: Array<number> = [
    32000,
    40000,
    48000,
    56000,
    64000,
    80000,
    96000,
    112000,
    128000,
    144000,
    160000
  ];

  audioInterfaceForm = this.fb.group({
    iBitRate: [''],
    iSampleRate: [''],
    iVolume: ['', isNumberJudge],
    sANS: [''],
    sEncodeType: [''],
    sInput: ['']
  });

  oldPara = {
    iVolume: NaN,
  };

  rangeSet = {
    iVolume: {
      min: 1,
      max: 100,
      step: 1,
    }
  };

  get iVolume(): FormControl {
    return this.audioInterfaceForm.get('iVolume') as FormControl;
  }

  ngOnInit() {
    this.cfgService.getAudioInterface().subscribe((res: AudioInterface) => {
      this.resError.analyseRes(res);
      this.audioInterfaceForm.patchValue(res);
      this.forTest();
    });
  }

  onSubmit() {
    // set for test
    this.audioInterfaceForm.get('sEncodeType').enable();
    this.pfs.formatInt(this.audioInterfaceForm.value);
    this.cfgService.setAudioInterface(this.audioInterfaceForm.value)
      .subscribe(res => {
        this.resError.analyseRes(res);
        this.audioInterfaceForm.patchValue(res);
        // set for test
        this.audioInterfaceForm.get('sEncodeType').disable();
        this.tips.showSaveSuccess();
      },
      err => {
        this.logger.error(err, 'onSubmit:setAudioInterface:');
        this.tips.showSaveFail();
      });
  }

  forTest() {
    this.audioInterfaceForm.get('sEncodeType').setValue('MP2');
    this.audioInterfaceForm.get('sEncodeType').disable();
    this.audioInterfaceForm.get('sInput').disable();
    this.audioInterfaceForm.get('sANS').disable();
    this.audioInterfaceForm.get('iSampleRate').disable();
    this.audioInterfaceForm.get('iBitRate').disable();
  }

  rangeAndNumber(controlName: string, change: number) {
    if (!isNaN(this.oldPara[controlName]) && this.oldPara[controlName] !== Number(change) && !isNaN(Number(change))) {
      change = Number(change);
      if (change > this.rangeSet[controlName].max) {
        change = this.rangeSet[controlName].max;
      } else if (change < this.rangeSet[controlName].min) {
        change = this.rangeSet[controlName].min;
      }
      this.oldPara[controlName] = change;
      this.audioInterfaceForm.get(controlName).setValue(change);
    } else if (!isNaN(this.oldPara[controlName]) && isNaN(Number(change))) {
      this.audioInterfaceForm.get(controlName).setValue(this.oldPara[controlName]);
    } else if (isNaN(this.oldPara[controlName])) {
      this.oldPara[controlName] = change;
    }
  }
}
