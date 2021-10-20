import { Component, OnInit, OnDestroy } from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { LayoutService } from 'src/app/shared/func-service/layout.service';

@Component({
  selector: 'app-config-event',
  templateUrl: './config-event.component.html',
  styleUrls: ['./config-event.component.scss']
})
export class ConfigEventComponent implements OnInit, OnDestroy {

  constructor(
    private route: ActivatedRoute,
    private los: LayoutService,
    private pfs: PublicFuncService,
  ) { }

  selectedMenuItem: string = 'MotionDetect';
  private losOb: any;
  private layoutGroup: Array<string>;
  private paramOb: any;

  ngOnInit() {
    this.losOb = this.los.FinishSignal.subscribe(
      (si: boolean) => {
        if (si) {
          this.layoutFunc();
          if (this.losOb) {
            this.losOb.unsubscribe();
            this.losOb = null;
          }
        }
      }
    );
  }

  layoutFunc() {
    if (this.paramOb) {
      this.paramOb.unsubscribe();
      this.paramOb = null;
    }
    this.layoutGroup = this.los.getThirdLt('config', 'Event');
    this.paramOb = this.route.queryParamMap.subscribe(params => {
      let menu = params.get('menu');
      if (!this.pfs.isInArrayString(this.layoutGroup, menu) && this.layoutGroup.length > 0) {
        menu = this.layoutGroup[0];
      }
      if (menu === 'MotionDetect') {
        this.selectedMenuItem = 'MotionDetect';
      } else if (menu === 'AlarmInput') {
        this.selectedMenuItem = 'AlarmInput';
      } else if (menu === 'AlarmOutput') {
        this.selectedMenuItem = 'AlarmOutput';
      } else if (menu === 'Abnormal') {
        this.selectedMenuItem = 'Abnormal';
      } else if (menu === 'IntrusionDetection') {
        this.selectedMenuItem = 'IntrusionDetection';
      } else {
        this.selectedMenuItem = 'MotionDetect';
      }
    });
  }

  ngOnDestroy() {
    if (this.losOb) {
      this.losOb.unsubscribe();
    }
    if (this.paramOb) {
      this.paramOb.unsubscribe();
    }
  }
}
