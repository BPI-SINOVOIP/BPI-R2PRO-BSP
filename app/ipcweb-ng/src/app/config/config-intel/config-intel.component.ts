import { Component, OnInit, OnDestroy } from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import { LayoutService } from 'src/app/shared/func-service/layout.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';

@Component({
  selector: 'app-config-intel',
  templateUrl: './config-intel.component.html',
  styleUrls: ['./config-intel.component.scss']
})
export class ConfigIntelComponent implements OnInit, OnDestroy {

  constructor(
    private route: ActivatedRoute,
    private los: LayoutService,
    private pfs: PublicFuncService,
  ) { }

  private losOb: any;
  private layoutGroup: Array<string>;
  private routeObserver: any;
  subMenuItemsBasic: Array<string> = [
    'overlay-snap',
    'mask-area',
    'rule-setting',
    'intel-advanced'
  ];
  selectedMenuItem: string = 'overlay-snap';

  ngOnInit() {
    this.losOb = this.los.FinishSignal.subscribe(
      (si: boolean) => {
        if (si) {
          this.layoutGroup = this.los.getThirdLt('config', 'Intel');
          this.routeObserver = this.route.queryParamMap.subscribe(params => {
            let menu = params.get('menu');
            if (!this.pfs.isInArrayString(this.layoutGroup, menu) && this.layoutGroup.length > 0) {
              menu = this.layoutGroup[0];
            }
            if (menu === 'MarkCover') {
              this.selectedMenuItem = 'overlay-snap';
            } else if (menu === 'MaskArea') {
              this.selectedMenuItem = 'mask-area';
            } else if (menu === 'RuleSettings') {
              this.selectedMenuItem = 'rule-setting';
            } else if (menu === 'Advanced') {
              this.selectedMenuItem = 'intel-advanced';
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
    if (this.routeObserver) {
      this.routeObserver.unsubscribe();
    }
    if (this.losOb) {
      this.losOb.unsubscribe();
    }
  }
}
