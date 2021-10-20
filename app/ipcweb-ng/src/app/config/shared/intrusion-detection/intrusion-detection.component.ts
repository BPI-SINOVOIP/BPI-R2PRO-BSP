import { Component, OnInit, OnDestroy, AfterViewInit, Renderer2, ElementRef, ViewChild } from '@angular/core';
import { FormBuilder } from '@angular/forms';

import { ConfigService } from 'src/app/config.service';
import { async } from '@angular/core/testing';
import { TimeTableOption } from 'src/app/config/shared/time-table/TimeTableInterface';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { BoolEmployee, EmployeeItem } from 'src/app/shared/func-service/employee.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { TipsService } from 'src/app/tips/tips.service';
import { LastNavService } from 'src/app/shared/func-service/last-nav.service';
import { DiyHttpService } from 'src/app/shared/func-service/diy-http.service';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-intrusion-detection',
  templateUrl: './intrusion-detection.component.html',
  styleUrls: ['./intrusion-detection.component.scss']
})
export class IntrusionDetectionComponent implements OnInit, AfterViewInit, OnDestroy {

  @ViewChild('region', {static: false}) regionChild: any;
  @ViewChild('arming', {static: false}) armingChild: any;
  @ViewChild('linkage', {static: false}) linkageChild: any;

  constructor(
    private cfg: ConfigService,
    private fb: FormBuilder,
    private Re2: Renderer2,
    private el: ElementRef,
    private ieCss: IeCssService,
    private resError: ResponseErrorService,
    private tips: TipsService,
    private pfs: PublicFuncService,
    private lns: LastNavService,
    private dhs: DiyHttpService,
  ) { }

  private logger: Logger = new Logger('intrusion-detection');
  isChrome: boolean = false;
  private navGroup: string = 'instrusion';
  private src: string = '';
  private lastActiveTab: string = '';
  selectedTab = 'regionSetting';
  private linkagePath = 'vri_0';
  private LinkageForm: any;
  private ScheduleForm: string = '';
  private scheduleId: number = 1;
  tabOption: TimeTableOption = {
    isInit: false,
    isType: false,
    isEnabledTop: false,
    isAdvance: false,
    advancePara: [],
    id: 1,
    paraId: NaN,
    pageId: 'armingTimeTab',
    advanceId: -1,
  };
  private urlOb: any;
  employee = new BoolEmployee();

  menuItems: Array<string> = [
    'regionSetting',
    'armingTime',
    'linkageMode',
  ];

  tabItems: Array<string> = [
    'regionSettingTab',
    'armingTimeTab',
    'linkageModeTab',
  ];

  epBank: EmployeeItem[] = [
    {
      name: 'init',
      task: ['region', 'src', 'schedule', 'linkage']
    },
    {
      name: 'save',
      task: ['region', 'schedule', 'linkage']
    }
  ];

  private normalSize: any;
  private isSetVal: boolean = false;

  IntrusionForm = this.fb.group({
    iEnabled: 0,
    iHeight: 0,
    iPositionX: 0,
    iPositionY: 0,
    iProportion: 0,
    iSensitivityLevel: 0,
    iTimeThreshold: 0,
    iWidth: 0,
  });

  ngOnInit(): void {
    this.isChrome = this.ieCss.getChromeBool();
    this.lns.getLastNav(this.navGroup, this.el);
    this.employee.hire(this.epBank[0]);
    this.cfg.getIntrusionRegionInfo().subscribe(
      res => {
        this.resError.analyseRes(res);
        this.IntrusionForm.patchValue(res.regionalInvasion);
        this.normalSize = res.normalizedScreenSize;
        this.employee.doTask(this.epBank[0].name, this.epBank[0].task[0], true, true);
      },
      err => {
        this.logger.error(err, 'ngOnInit:getIntrusionRegionInfo:');
        this.employee.doTask(this.epBank[0].name, this.epBank[0].task[0], false, true);
      }
    );

    this.urlOb = this.dhs.getStreamUrl().subscribe(
      (msg: string) => {
        if (msg) {
          this.src = msg;
          this.employee.doTask(this.epBank[0].name, this.epBank[0].task[1], true, true);
        } else {
          this.src = null;
          this.tips.setRbTip('getVideoUrlFail');
          this.employee.doTask(this.epBank[0].name, this.epBank[0].task[1], false, true);
        }
        this.urlOb.unsubscribe();
        this.urlOb = null;
      }
    );

    this.cfg.getLinkageInterface(this.linkagePath).subscribe(
      res => {
        this.LinkageForm = res;
        this.employee.doTask(this.epBank[0].name, this.epBank[0].task[3], true, true);
      },
      err => {
        this.logger.error(err, 'ngOnInit:getLinkageInterface:');
        this.employee.doTask(this.epBank[0].name, this.epBank[0].task[3], false, true);
      }
    );

    this.cfg.getSchedule(this.scheduleId).subscribe(
      res => {
        this.ScheduleForm = res['sSchedulesJson'];
        this.employee.doTask(this.epBank[0].name, this.epBank[0].task[2], true, true);
      },
      err => {
        this.logger.error(err, 'ngOnInit:getSchedule:');
        this.employee.doTask(this.epBank[0].name, this.epBank[0].task[2], false, true);
      }
    );

    let waitNavName = this.lns.getLastNavName(this.navGroup);
    if (!this.pfs.isInArrayString(this.tabItems, waitNavName)) {
      waitNavName = 'regionSettingTab';
    }
    this.employee.observeTask(this.epBank[0].name, 5000)
      .then(() => {
        this.pfs.waitNavActive(5000, waitNavName)
          .then(() => {
            if (this.employee.getWorkResult(this.epBank[0].name)) {
              this.setInfoForChild();
              this.isSetVal = true;
            } else {
              this.tips.showInitFail();
            }
            this.employee.dismissOne(this.epBank[0].name);
          })
          .catch((error) => {
            this.logger.error(error, 'ngOnInit:observeTask:waitNavActive:catch');
            this.tips.showInitFail();
            this.employee.dismissOne(this.epBank[0].name);
          });
      })
      .catch((error) => {
        if (error !== undefined) {
          this.logger.error(error, 'ngOnInit:observeTask:catch');
          this.tips.setRbTip('getParaFailFreshPlease');
          this.employee.dismissOne(this.epBank[0].name);
        }  
      });
  }

  ngAfterViewInit() {
    this.Re2.setAttribute(document.getElementById('intrusion-save'), 'hidden', 'true');
  }

  ngOnDestroy() {
    if (this.urlOb) {
      this.urlOb.unsubscribe();
    }
  }

  onSubmit() {
    this.employee.hire(this.epBank[1]);
    // region submit
    this.getInfoFromChild();
    this.pfs.formatInt(this.IntrusionForm.value);
    this.cfg.putIntrusionRegionInfo(this.IntrusionForm.value).subscribe(
      res => {
        this.employee.doTask(this.epBank[1].name, this.epBank[1].task[0], true, true);
        this.resError.analyseRes(res);
        this.IntrusionForm.patchValue(res.regionalInvasion);
      },
      err => {
        this.logger.error(err, 'onSubmit:putIntrusionRegionInfo:');
        this.employee.doTask(this.epBank[1].name, this.epBank[1].task[0], false, true);
      }
    );

    this.saveSchedule();

    this.pfs.formatInt(this.LinkageForm);
    this.cfg.putLinkageInterface(this.LinkageForm, this.linkagePath).subscribe(
      res => {
        this.resError.analyseRes(res);
        this.employee.doTask(this.epBank[1].name, this.epBank[1].task[2], true, true);
        this.LinkageForm = res;
      },
      err => {
        this.logger.error(err, 'onSubmit:putLinkageInterface:');
        this.employee.doTask(this.epBank[1].name, this.epBank[1].task[2], false, true);
      }
    );

    this.employee.observeTask(this.epBank[1].name, 10000)
      .then(
        () => {
          if (this.employee.getWorkResult(this.epBank[1].name)) {
            this.setInfoForChild();
            this.tips.showSaveSuccess();
          } else {
            this.tips.showSaveFail();
          }
          this.employee.dismissOne(this.epBank[1].name);
        }
      )
      .catch(
        () => {
          this.logger.error('onSubmit:observeTask:catch:');
          this.tips.showSaveFail();
          this.employee.dismissOne(this.epBank[1].name);
        }
      );
  }

  saveSchedule() {
    const data = {
      sSchedulesJson: this.ScheduleForm,
    };
    this.cfg.setSchedule(this.scheduleId, data).subscribe(
      res => {
        this.employee.doTask(this.epBank[1].name, this.epBank[1].task[1], true, true);
        this.resError.analyseRes(res);
        this.ScheduleForm = res['sSchedulesJson'];
      },
      err => {
        this.logger.error(err, 'saveSchedule:');
        this.employee.doTask(this.epBank[1].name, this.epBank[1].task[1], false, true);
      }
    );
  }

  onClickTab(tabName: string) {
    if (tabName !== this.lastActiveTab) {
      this.lns.setLastNav(this.navGroup, tabName + 'Tab');
      this.selectedTab = tabName;
      if (this.isSetVal) {
        this.getInfoFromChild();
      }
      switch (tabName) {
        case 'regionSetting':
          this.Re2.setAttribute(document.getElementById('intrusion-save'), 'hidden', 'true');
          this.initPlayer(tabName);
          break;
        case 'armingTime':
          this.Re2.removeAttribute(document.getElementById('intrusion-save'), 'hidden');
          (this.lastActiveTab === 'regionSetting') ? this.pausePlayer() : null;
          // this.resizeTimeTable(tabName);
          break;
        case 'linkageMode':
          this.Re2.removeAttribute(document.getElementById('intrusion-save'), 'hidden');
          (this.lastActiveTab === 'regionSetting') ? this.pausePlayer() : null;
          break;
      }
      this.setChildPara(tabName);
      this.lastActiveTab = tabName;
    }
  }

  setChildPara(childName: string) {
    this.pfs.waitNavActive(5000, childName + 'Tab')
      .then(() => {
        this.setInfoForChild(childName);
      })
      .catch(() => {
        // this.tips.showInitFail();
      });
  }

  pausePlayer() {
    if (this.lastActiveTab === 'regionSetting') {
      try {
        this.regionChild.pausePlayer();
      } catch {}
    }
  }

  getInfoFromChild(childName: string = this.lastActiveTab) {
    switch (childName) {
      case 'regionSetting':
        const formValue = this.regionChild.getRegionInfo();
        formValue.iEnabled = Number(this.IntrusionForm.value.iEnabled);
        this.IntrusionForm.patchValue(formValue);
        break;
      case 'armingTime':
        this.ScheduleForm = this.armingChild.getsJsonString();
        break;
      case 'linkageMode':
        this.LinkageForm = this.linkageChild.getLikageForm();
        break;
    }
  }

  setInfoForChild(childName: string = this.lastActiveTab) {
    switch (childName) {
      case 'regionSetting':
        if (this.IntrusionForm.value && this.src && this.normalSize) {
          this.regionChild.initComponent(this.IntrusionForm.value, this.src, this.normalSize);
        }
        break;
      case 'armingTime':
        if (this.ScheduleForm) {
          this.armingChild.initDrawer(this.ScheduleForm);
        }
        break;
      case 'linkageMode':
        if (this.LinkageForm) {
          this.linkageChild.setLinkageForm(this.LinkageForm);
        }
        break;
    }
  }


  async initPlayer(tabName: string) {
    this.pfs.waitNavActive(5000, tabName + 'Tab')
      .then(() => {
        // this.regionChild.resizePlayer();
        this.regionChild.playEntry();
      })
      .catch(() => {
        this.tips.setRbTip('initPlayerFailFreshPlease');
      });
  }

  async resizeTimeTable(tabName: string) {
    this.pfs.waitNavActive(5000, tabName + 'Tab')
      .then(() => {
        // this.armingChild.resizeTable();
      })
      .catch(() => {
        this.tips.setRbTip('initScheduleFailFreshPlease');
      });
  }
}
