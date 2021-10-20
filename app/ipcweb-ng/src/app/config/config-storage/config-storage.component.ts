import { Component, OnInit, ViewChild, OnDestroy, ElementRef } from '@angular/core';
import { ActivatedRoute } from '@angular/router';

import { TimeTableOption } from 'src/app/config/shared/time-table/TimeTableInterface';
import { TipsService } from 'src/app/tips/tips.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { LastNavService } from 'src/app/shared/func-service/last-nav.service';
import { LayoutService } from 'src/app/shared/func-service/layout.service';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-config-storage',
  templateUrl: './config-storage.component.html',
  styleUrls: ['./config-storage.component.scss']
})
export class ConfigStorageComponent implements OnInit, OnDestroy {

  @ViewChild('video', {static: false}) videoTimeChild: any;
  @ViewChild('snap', {static: false}) snapTimeChild: any;

  constructor(
    private _route: ActivatedRoute,
    private el: ElementRef,
    private tips: TipsService,
    private pfs: PublicFuncService,
    private lns: LastNavService,
    private los: LayoutService,
  ) { }
  private logger: Logger = new Logger('config-storage');

  private losOb: any;
  private navGroup = 'storage';
  private lastActiveTab: string = 'VideoPlan';
  private paraChange: any;
  private layoutGroup: Array<string>;
  selectedMenuItem: string;
  subMenuItems: Array<string>;

  clickCheckList: Array<string> = ['VideoPlan', 'ScreenshotPlan'];
  subMenuItemsPlanSettings: Array<string> = [
    'VideoPlan',
    'ScreenshotPlan',
    'ScreenshotPara',
  ];

  subMenuItemsStorageManage: Array<string> = [
    'HardDiskManagement',
    'NAS',
    'CloudStorage'
  ];

  tabOption4Video: TimeTableOption = {
    isInit: true,
    isType: true,
    isEnabledTop: true,
    isAdvance: true,
    advancePara: [
      {
        type: 'checkbox',
        title: 'circularWrite'
      },
      {
        type: 'selector',
        title: 'prerecordedTime',
        content: [0, 5, 10, 15, 20, 25, 30, 2147483647],
        transfer: {0: 'notPrerecorded', 5: '5s', 10: '10s', 15: '15s', 20: '20s', 25: '25s', 30: '30s', 2147483647: 'unlimit'},
      },
      {
        type: 'selector',
        title: 'videoDelay',
        content: [5, 10, 30, 60, 120, 300, 600],
        transfer: {5: '5s', 10: '10s', 30: '30s', 60: '1min', 120: '2min', 300: '5min', 600: '10min'}
      },
      {
        type: 'selector',
        title: 'streamType',
        content: ['mainStream', 'subStream', 'thirdStream'],
      }
    ],
    id: 2,
    paraId: 2,
    pageId: 'VideoPlanTab',
    advanceId: 0,
  };

  tabOption4Screenshot: TimeTableOption = {
    isInit: true,
    isType: true,
    isEnabledTop: false,
    isAdvance: false,
    advancePara: [],
    // advancePara: [
    //   {
    //     type: 'selector',
    //     title: 'streamType',
    //     content: ['mainStream', 'subStream'],
    //   }
    // ],
    id: 3,
    paraId: 1,
    pageId: 'ScreenshotPlanTab',
    advanceId: -1,
  };

  selectTab: string;

  ngOnInit() {

    this.losOb = this.los.FinishSignal.subscribe(
      (si: boolean) => {
        if (si) {
          this.layoutGroup = this.los.getThirdLt('config', 'Storage');
          for (const thirdLy of this.layoutGroup) {
            switch (thirdLy) {
              case 'PlanSettings':
                this.subMenuItemsPlanSettings = this.los.getfourthLt('config', 'Storage', 'PlanSettings');
                break;
              case 'StorageManage':
                this.subMenuItemsStorageManage = this.los.getfourthLt('config', 'Storage', 'StorageManage');
                break;
            }
          }
          this.paraChange = this._route.queryParamMap.subscribe(params => {
            let menu = params.get('menu');
            if (!this.pfs.isInArrayString(this.layoutGroup, menu) && this.layoutGroup.length > 0) {
              menu = this.layoutGroup[0];
            }
            if (menu === 'PlanSettings') {
              this.setMenu('PlanSettings');
              this.subMenuItems = this.subMenuItemsPlanSettings;
              this.lastActiveTab = 'VideoPlan';
            } else if (menu === 'StorageManage') {
              this.setMenu('StorageManage');
              this.subMenuItems = this.subMenuItemsStorageManage;
            } else {
              this.setMenu('StorageManage');
              this.subMenuItems = this.subMenuItemsStorageManage;
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
    if (this.paraChange) {
      this.paraChange.unsubscribe();
    }
    if (this.losOb) {
      this.losOb.unsubscribe();
    }
  }

  setMenu(itemName: string) {
    if (itemName !== this.selectedMenuItem) {
      this.selectedMenuItem = itemName;
      this.lns.getLastNav(this.navGroup + itemName, this.el);
    }
  }

  onClickTab(tabName: string) {
    if (this.lastActiveTab === tabName) {
      return;
    } else {
      this.lns.setLastNav(this.navGroup + this.selectedMenuItem, tabName + 'Tab');
      for (const item of this.clickCheckList) {
        if (item === tabName) {
          this.resizeTimeTable(tabName);
          break;
        }
      }
      this.lastActiveTab = tabName;
    }
  }

  resizeTimeTable(tabName: string) {
    this.pfs.waitNavActive(7000, tabName + 'Tab')
      .then(() => {
        switch (tabName) {
          case 'VideoPlan':
            this.videoTimeChild.reshapeCanvas();
            break;
          case 'ScreenshotPlan':
            this.snapTimeChild.reshapeCanvas();
            break;
        }
      })
      .catch((error) => {
        this.logger.error(error, 'resizeTimeTable:waitNavActive:catch');
        this.tips.showInitFail();
      });
  }

}
