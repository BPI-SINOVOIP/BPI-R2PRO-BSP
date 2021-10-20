import { Component, OnInit, ElementRef, OnDestroy, Renderer2 } from '@angular/core';
import { FormBuilder, FormGroup} from '@angular/forms';

import { ConfigService } from 'src/app/config.service';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { TipsService } from 'src/app/tips/tips.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-screen-config',
  templateUrl: './screen-config.component.html',
  styleUrls: ['./screen-config.component.scss']
})
export class ScreenConfigComponent implements OnInit {

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

  isChrome: boolean = false;
  private logger: Logger = new Logger('screen-config');

  LightForm = this.fb.group({
    iNormalBrightness: [''],
    iSaveEnergyBrightness: [''],
    iSaveEnergyEnable: [''],
    id: [''],
  });

  ngOnInit(): void {
    this.isChrome = this.ieCss.getChromeBool();
    this.cfg.getFillLightPeripherals().subscribe(
      res => {
        this.resError.analyseRes(res);
        this.LightForm.patchValue(res);
      },
      err => {
        this.logger.error('ngOnInit:getFillLightPeripherals:');
        this.tips.showInitFail();
      }
    );
  }

  onSubmit() {
    this.pfs.formatInt(this.LightForm.value);
    this.cfg.setFillLightPeripherals(this.LightForm.value).subscribe(
      res => {
        this.resError.analyseRes(res);
        this.LightForm.patchValue(res);
        this.tips.showSaveSuccess();
      },
      err => {
        this.logger.error('onSubmit:setFillLightPeripherals:');
        this.tips.showSaveFail();
      }
    );
  }

}
