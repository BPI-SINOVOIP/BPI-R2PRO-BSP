import { Component, OnInit } from '@angular/core';
import { ConfigService } from 'src/app/config.service';
import { FormBuilder, FormArray } from '@angular/forms';
import { DeviceInfoInterface } from './DeviceInfoInterface';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { TipsService } from 'src/app/tips/tips.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-info',
  templateUrl: './info.component.html',
  styleUrls: ['./info.component.scss']
})
export class InfoComponent implements OnInit {

  constructor(
    private cfgService: ConfigService,
    private fb: FormBuilder,
    private ieCss: IeCssService,
    private resError: ResponseErrorService,
    private tips: TipsService,
    private pfs: PublicFuncService,
  ) { }

  private logger: Logger = new Logger('info');

  isIE: boolean = true;

  infoForm = this.fb.group ({
    infos: this.fb.array([]),
  });

  get infos(): FormArray {
    return this.infoForm.get('infos') as FormArray;
  }

  ngOnInit() {
    this.isIE = this.ieCss.getIEBool();
    this.cfgService.getDeviceInfo().subscribe((res: DeviceInfoInterface[]) => {
      this.resError.analyseRes(res);
      const sortedInfo = new Array<DeviceInfoInterface>();
      const roInfo = new Array<DeviceInfoInterface>();
      res.forEach(info => {
        if (info.ro.match('true')) {
          roInfo.push(info);
        } else {
          sortedInfo.push(info);
        }
      });
      roInfo.forEach(info => {
        sortedInfo.push(info);
      });
      sortedInfo.forEach(info => {
        const group = this.fb.group({
          id: info.id,
          name: info.name,
          value: info.value,
          ro: info.ro,
        });
        this.infos.push(group);
        if (info.ro && info.ro.match('true')) {
          group.get('value').disable();
        }
      });
    });
  }

  onSubmit() {
    for (let index = 0; this.infos.length; index++) {
      const ro = this.infos.at(index).get('ro').value;
      if ( ro && ro.match('true')) {
        break;
      }
      this.cfgService.setDeviceInfo(this.infos.at(index).value)
        .subscribe(res => {
          this.resError.analyseRes(res);
          this.tips.showSaveSuccess();
        },
        err => {
          this.logger.error(err, 'onSubmit:setDeviceInfo:');
          this.tips.showSaveFail();
      });
    }
  }
}
