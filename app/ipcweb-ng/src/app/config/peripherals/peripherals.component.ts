import { Component, OnInit, OnDestroy } from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { LayoutService } from 'src/app/shared/func-service/layout.service';

@Component({
  selector: 'app-peripherals',
  templateUrl: './peripherals.component.html',
  styleUrls: ['./peripherals.component.scss']
})
export class PeripheralsComponent implements OnInit, OnDestroy {

  constructor(
    private route: ActivatedRoute,
    private los: LayoutService,
    private pfs: PublicFuncService,
  ) { }

  selectedMenuItem: string;
  activeTab: string = '';
  private losOb: any;
  private layoutGroup: Array<string>;
  private paramOb: any;

  ngOnInit(): void {
    this.losOb = this.los.FinishSignal.subscribe(
      (si: boolean) => {
        if (si) {
          this.layoutGroup = this.los.getThirdLt('config', 'Peripherals');
          this.paramOb = this.route.queryParamMap.subscribe(params => {
            let menu = params.get('menu');
            if (!this.pfs.isInArrayString(this.layoutGroup, menu) && this.layoutGroup.length > 0) {
              menu = this.layoutGroup[0];
            }
            if (menu === 'GateConfig') {
              this.selectedMenuItem = 'GateConfig';
            } else if (menu === 'ScreenConfig') {
              this.selectedMenuItem = 'ScreenConfig';
            } else {
              this.selectedMenuItem = 'GateConfig';
            }
          });
          if (this.losOb) {
            this.losOb.unsubscribe();
            this.losOb = null;
          }
        }
      }
    );
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
