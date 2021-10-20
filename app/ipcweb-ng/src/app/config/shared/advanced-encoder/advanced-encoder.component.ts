import { Component, OnInit, OnDestroy, ElementRef, Renderer2, ViewChild } from '@angular/core';

import { FormBuilder, Validators, AbstractControl, FormControl, FormGroup } from '@angular/forms';
import { ConfigService } from 'src/app/config.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { TipsService } from 'src/app/tips/tips.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-advanced-encoder',
  templateUrl: './advanced-encoder.component.html',
  styleUrls: ['./advanced-encoder.component.scss']
})
export class AdvancedEncoderComponent implements OnInit {

  constructor(
    private cfg: ConfigService,
    private fb: FormBuilder,
    private Re2: Renderer2,
    private el: ElementRef,
    private resError: ResponseErrorService,
    private tips: TipsService,
    private pfs: PublicFuncService,
  ) { }

  private logger: Logger = new Logger('layout-service');

  objectKeys = this.pfs.objectKeys;

  FormList: Array<Array<FormGroup>> = [];
  streamList = [
    'mainStream',
    'subStream',
    'thirdStream'
  ];

  ngOnInit(): void {
    this.cfg.getAdvanceEnco().subscribe(
      res => {
        this.resError.analyseRes(res);
        this.initFormList(res);
      },
      err => {
        this.tips.showInitFail();
        this.logger.error(err, 'ngOnInit:getAdvanceEnco:');
      }
    );
  }

  initFormList(getInfo: any) {
    // tslint:disable-next-line: forin
    for (const index in getInfo) {
      this.FormList.push([this.fb.group({}), this.fb.group({})]);
      this.appendControl(index, 0, 'sFunction', getInfo[index].sFunction);
      this.appendControl(index, 0, 'sStreamType', getInfo[index].sStreamType);
      const paraList = JSON.parse(getInfo[index].sParameters);
      for (const key of this.objectKeys(paraList)) {
        this.appendControl(index, 1, key, paraList[key]);
      }
    }
  }

  appendControl(index: string|number, pat: string|number, name: string, data: any) {
    this.FormList[index][pat].addControl(name, new FormControl());
    this.FormList[index][pat].get(name).setValue(data);
  }

  onSubmit(index: number|string) {
    const submitForm = {
      sFunction: '',
      sParameters: '',
      sStreamType: '',
    };
    submitForm.sFunction = this.FormList[index][0].value.sFunction;
    submitForm.sStreamType = this.FormList[index][0].value.sStreamType;
    submitForm.sParameters = JSON.stringify(this.FormList[index][1].value);
    this.cfg.putAdvanceEnco(submitForm).subscribe(
      res => {
        this.resError.analyseRes(res);
        this.tips.showSaveSuccess();
      },
      err => {
        this.logger.error(err, 'onSubmit:onSubmit:putAdvanceEnco:');
        this.tips.showSaveFail();
      }
    );
  }
}
