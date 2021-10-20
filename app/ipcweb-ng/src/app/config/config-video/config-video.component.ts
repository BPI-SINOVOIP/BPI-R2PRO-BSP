import { Component, OnInit, OnDestroy } from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { LayoutService } from 'src/app/shared/func-service/layout.service';

@Component({
  selector: 'app-config-video',
  templateUrl: './config-video.component.html',
  styleUrls: ['./config-video.component.scss']
})
export class ConfigVideoComponent implements OnInit, OnDestroy {
  selectedMenuItem: string;
  activeTab: string = '';
  private paramOb: any;
  subMenuItems: Array<string>;
  subMenuItemsEncodeParam: Array<string> = [];
  subMenuItemsRoi: Array<string> = [];
  subMenuItemsCrop: Array<string> = [];
  private losOb: any;
  private layoutGroup: Array<string>;

  constructor(
    private route: ActivatedRoute,
    private los: LayoutService,
    private pfs: PublicFuncService,
  ) { }

  ngOnInit() {

    this.losOb = this.los.FinishSignal.subscribe(
      (si: boolean) => {
        if (si) {
          this.layoutGroup = this.los.getThirdLt('config', 'Video');
          this.paramOb = this.route.queryParamMap.subscribe(params => {
            let menu = params.get('menu');
            if (!this.pfs.isInArrayString(this.layoutGroup, menu) && this.layoutGroup.length > 0) {
              menu = this.layoutGroup[0];
            }
            if (menu === 'Encoder') {
              this.selectedMenuItem = 'Encoder';
            } else if (menu === 'ROI') {
              this.selectedMenuItem = 'ROI';
            } else if (menu === 'RegionCrop') {
              this.selectedMenuItem = 'RegionCrop';
            } else if (menu === 'AdvancedEncoder') {
              this.selectedMenuItem = 'AdvancedEncoder';
            } else {
              this.selectedMenuItem = 'Encoder';
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
