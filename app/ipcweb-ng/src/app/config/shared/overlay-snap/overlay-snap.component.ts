import { Component, OnInit } from '@angular/core';
import { FormBuilder, FormArray, FormControl } from '@angular/forms';

import { OverlaySnapDefaultPara } from './OverlaySnapInterface';
import { ConfigService } from 'src/app/config.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { TipsService } from 'src/app/tips/tips.service';
import { LockService } from 'src/app/shared/func-service/lock-service.service';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { BoolEmployee, EmployeeItem } from 'src/app/shared/func-service/employee.service';
import { isDecimal } from 'src/app/shared/validators/is-decimal.directive';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-overlay-snap',
  templateUrl: './overlay-snap.component.html',
  styleUrls: ['./overlay-snap.component.scss']
})
export class OverlaySnapComponent implements OnInit {

  constructor(
    private ieCss: IeCssService,
    private resError: ResponseErrorService,
    private tips: TipsService,
    private cfg: ConfigService,
    private fb: FormBuilder,
    private pfs: PublicFuncService,
  ) { }

  epBank: EmployeeItem[] = [
    {
      name: 'init',
      task: ['cap', 'para']
    }
  ];

  private logger: Logger = new Logger('overlay-snap');
  private employee = new BoolEmployee();
  testModel: boolean = false;
  // lock = new LockService(this.tips);
  isChrome: boolean = false;
  itemTransfer = this.pfs.translateFormItem;

  coverForm = this.fb.group({
    id: [''],
    iFaceEnabled: [''],
    iStreamOverlayEnabled: [''],
    iImageOverlayEnabled: [''],
    iInfoOverlayEnabled: [''],
    sTargetImageType: [''],
    iFaceRecognitionEnabled: [''],
    iWidthRatio: ['', isDecimal(1)],
    iFaceHeightRatio: ['', isDecimal(1)],
    iBodyHeightRatio: ['', isDecimal(1)],
    sImageQuality: [''],
    infoOverlay: this.fb.array([
      this.fb.group({
        id: 0,
        iEnabled: 1,
        sName: 'deviceNum',
        sInfo: '',
        iOrder: 0,
      }),
      this.fb.group({
        id: 1,
        iEnabled: 1,
        sName: 'snapTime',
        sInfo: '',
        iOrder: 1,
      }),
      this.fb.group({
        id: 2,
        iEnabled: 1,
        sName: 'positonInfo',
        sInfo: '',
        iOrder: 2,
      })
    ]),
  });

  get infoOverlay(): FormArray {
    return this.coverForm.get('infoOverlay') as FormArray;
  }

  get iFaceHeightRatio(): FormControl {
    return this.coverForm.get('iFaceHeightRatio') as FormControl;
  }

  get iBodyHeightRatio(): FormControl {
    return this.coverForm.get('iBodyHeightRatio') as FormControl;
  }

  get iWidthRatio(): FormControl {
    return this.coverForm.get('iWidthRatio') as FormControl;
  }

  orderArray: Array<string> = ['deviceNum', 'snapTime', 'positonInfo'];

  defaultPara: OverlaySnapDefaultPara = {
    capability: {
        SmartCover: {
            sImageQuality: ['best', 'good', 'general'],
            sTargetImageType: ['head'],
        }
    },
    layout: {
        enabled: ['iFaceRecognitionEnabled', 'iStreamOverlayEnabled', 'iImageOverlayEnabled'],
        snap: ['sTargetImageType', 'iWidthRatio', 'sImageQuality'],
        infoEnabled: ['deviceNum', 'positonInfo']
    },
  };

  // isImgTargetOverlap, isOverlapIntelli,
  ngOnInit(): void {
    this.isChrome = this.ieCss.getChromeBool();
    this.employee.hire(this.epBank[0]);
    this.cfg.getDefaultPara(3).subscribe(
      res => {
        this.resError.analyseRes(res);
        this.defaultPara = JSON.parse(res.toString());
        this.employee.numTask(this.epBank, 0, 0, 1, 1);
      },
      err => {
        this.logger.error(err, 'ngOnInit:getDefaultPara(3):');
        this.employee.numTask(this.epBank, 0, 0, 0, 1);
      }
    );
    this.cfg.getOverlaySnap().subscribe(
      res => {
        this.resError.analyseRes(res);
        this.formatRatio(res);
        this.coverForm.patchValue(res);
        this.employee.numTask(this.epBank, 0, 1, 1, 1);
      },
      err => {
        this.logger.error(err, 'ngOnInit:getOverlaySnap:');
        this.employee.numTask(this.epBank, 0, 1, 0, 1);
      }
    );
    this.forTest();
    this.employee.observeTask(this.epBank[0].name, 5000)
      .then(
        () => {
          if (!this.employee.getWorkResult(this.epBank[0].name)) {
            this.tips.showInitFail();
          }
        }
      )
      .catch(
        () => {
          this.tips.showInitFail();
        }
      );
  }

  checkInfoNeed(testName: string) {
    for (const infoName of this.defaultPara.layout.infoEnabled) {
      if (testName === infoName) {
        return true;
      }
    }
    return false;
  }

  onSubmit() {
    // forbid other when ttl is 0
    this.onFaceChange(this.coverForm.value.iFaceEnabled);
    // get order
    // tslint:disable-next-line: forin
    for (const i in this.orderArray) {
      for (const item of this.coverForm.get('infoOverlay').value) {
        if (item.sName === this.orderArray[i]) {
          this.coverForm.get('infoOverlay').value[item.id].iOrder = Number(i);
        }
      }
    }
    this.unformatRatid(this.coverForm.value);
    this.pfs.formatInt(this.coverForm.value);

    this.cfg.setOverlaySnap(this.coverForm.value).subscribe(
      res => {
        this.resError.analyseRes(res);
        this.coverForm.patchValue(res);
        this.tips.showSaveSuccess();
      },
      err => {
        this.logger.error(err, 'onSubmit:setOverlaySnap:');
        this.tips.showSaveFail();
      }
    );
  }

  onUp(index: number) {
    if (index === 0) {
      return;
    }
    const stand = this.orderArray[index - 1];
    this.orderArray[index - 1] = this.orderArray[index];
    this.orderArray[index] = stand;
  }

  onDown(index: number) {
    if (index === (this.orderArray.length - 1)) {
      return;
    }
    const stand = this.orderArray[index + 1];
    this.orderArray[index + 1] = this.orderArray[index];
    this.orderArray[index] = stand;
  }

  forTest() {
    if (localStorage.getItem('test')) {
      this.testModel = Boolean(localStorage.getItem('test'));
    }
  }

  formatRatio(obj: object) {
    obj['iWidthRatio'] = obj['iWidthRatio'] / 10;
    obj['iFaceHeightRatio'] = obj['iFaceHeightRatio'] / 10;
    obj['iBodyHeightRatio'] = obj['iBodyHeightRatio'] / 10;
  }

  unformatRatid(obj: object) {
    obj['iWidthRatio'] = obj['iWidthRatio'] * 10;
    obj['iFaceHeightRatio'] = obj['iFaceHeightRatio'] * 10;
    obj['iBodyHeightRatio'] = obj['iBodyHeightRatio'] * 10;
  }

  onFaceChange(change: number) {
    if (!change) {
      this.coverForm.get('iStreamOverlayEnabled').setValue(0);
      this.coverForm.get('iImageOverlayEnabled').setValue(0);
      this.coverForm.get('iInfoOverlayEnabled').setValue(0);
      this.coverForm.get('iFaceRecognitionEnabled').setValue(0);
    }
  }
}
