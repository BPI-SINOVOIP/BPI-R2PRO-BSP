import { Component, OnInit, ElementRef, OnDestroy, Renderer2 } from '@angular/core';
import { FormBuilder, FormGroup} from '@angular/forms';

import { ConfigService } from 'src/app/config.service';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { TipsService } from 'src/app/tips/tips.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-gate-config',
  templateUrl: './gate-config.component.html',
  styleUrls: ['./gate-config.component.scss']
})
export class GateConfigComponent implements OnInit {

  constructor(
    private fb: FormBuilder,
    private cfg: ConfigService,
    private Re2: Renderer2,
    private el: ElementRef,
    private ieCss: IeCssService,
    private resError: ResponseErrorService,
    private tips: TipsService,
    private pfs: PublicFuncService,
  ) { }

  private logger: Logger = new Logger('gate-config');

  relayStatus: Array<number> = [0, 1];
  relayStatusRelation: any = {
    0: 'normallyOpen',
    1: 'normallyClose',
  };

  bitOption = [26, 34];
  indexOption = [0, ];

  isChrome: boolean = false;

  GateForm = this.fb.group({
    relay: this.fb.group({
      iDuration: [''],
      iEnable: [''],
      iIOIndex: [''],
      iValidLevel: [''],
      id: [''],
    }),
    weigen: this.fb.group({
      iDuration: [''],
      iEnable: [''],
      iWiegandBit: [''],
      id: [''],
    })
  });

  ngOnInit(): void {
    this.isChrome = this.ieCss.getChromeBool();
    this.cfg.getGatePeripherals().subscribe(
      (res) => {
        this.resError.analyseRes(res);
        this.GateForm.patchValue(res);
      },
      err => {
        this.logger.error(err, 'ngOnInit:getGatePeripherals:');
        this.tips.showInitFail();
      }
    );
  }

  onSubmit() {
    this.pfs.formatInt(this.GateForm.value);
    this.cfg.setGatePeripherals(this.GateForm.value).subscribe(
      res => {
        this.resError.analyseRes(res);
        this.GateForm.patchValue(res);
        this.tips.showSaveSuccess();
      },
      err => {
        this.logger.error(err, 'onSubmit:');
        this.tips.showSaveFail();
      }
    );
  }

}
