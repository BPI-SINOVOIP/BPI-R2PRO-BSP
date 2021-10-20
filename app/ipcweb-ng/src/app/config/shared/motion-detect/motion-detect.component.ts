import { Component, OnInit, OnDestroy, ViewChild, ElementRef, Renderer2 } from '@angular/core';
import { FormBuilder } from '@angular/forms';

import { ConfigService } from 'src/app/config.service';
import { StreamURLInterface } from 'src/app/preview/StreamURLInterface';
import { TimeTableOption } from 'src/app/config/shared/time-table/TimeTableInterface';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { BoolEmployee, EmployeeItem } from 'src/app/shared/func-service/employee.service';
import { TipsService } from 'src/app/tips/tips.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { LastNavService } from 'src/app/shared/func-service/last-nav.service';
import { DiyHttpService } from 'src/app/shared/func-service/diy-http.service';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-motion-detect',
  templateUrl: './motion-detect.component.html',
  styleUrls: ['./motion-detect.component.scss']
})
export class MotionDetectComponent implements OnInit, OnDestroy{

  @ViewChild('region', {static: false}) regionChild: any;
  @ViewChild('arming', {static: false}) armingChild: any;
  @ViewChild('linkage', {static: false}) linkageChild: any;

  constructor(
    private cfg: ConfigService,
    private fb: FormBuilder,
    private el: ElementRef,
    private Re2: Renderer2,
    private ieCss: IeCssService,
    private resError: ResponseErrorService,
    private tips: TipsService,
    private pfs: PublicFuncService,
    private lns: LastNavService,
    private dhs: DiyHttpService,
  ) { }

  isChrome: boolean = false;

  private logger: Logger = new Logger('motion-detect');
  private navGroup: string = 'motion';
  private linkagePath = 'vmd_0';
  private lastActiveTab: string = 'regionSetting';
  private src: string = '';
  selectedTab = 'regionSetting';
  private ScheduleForm: string = '';
  private LinkageForm: any;
  private scheduleId: number = 0;
  private isSetVal: boolean = false;
  private urlOb: any;
  tabOption: TimeTableOption = {
    isInit: false,
    isType: false,
    isEnabledTop: false,
    isAdvance: false,
    advancePara: [],
    id: 0,
    paraId: NaN,
    pageId: 'armingTimeTab',
    advanceId: -1,
  };
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

  RegionForm = this.fb.group({
    iColumnGranularity: 22,
    iEndTriggerTime: 500,
    iHighlightEnabled: 1,
    iMotionDetectionEnabled: 1,
    iRowGranularity: 18,
    iSamplingInterval: 2,
    iSensitivityLevel: 0,
    iStartTriggerTime: 500,
    id: [''],
    sGridMap: '0000012345456000000000000000000000000000000000000000000000000000000000000000000000000000000000000000',
    sRegionType: 'grid'
  });

  ngOnInit(): void {
    this.isChrome = this.ieCss.getChromeBool();
    this.lns.getLastNav(this.navGroup, this.el);
    this.employee.hire(this.epBank[0]);
    this.cfg.getMotionRegionInterface().subscribe(
      res => {
        this.resError.analyseRes(res);
        this.RegionForm.patchValue(res);
        this.employee.doTask(this.epBank[0].name, this.epBank[0].task[0], true, true);
      },
      err => {
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

    this.Re2.setAttribute(document.getElementById('intrusion-save'), 'hidden', 'true');

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
              this.logger.error('get para init fail');
              this.tips.showInitFail();
            }
            this.employee.dismissOne(this.epBank[0].name);
          })
          .catch((error) => {
            this.logger.error(error, 'ngOnInit:catch:observeTask:then:catch:');
            if (error) {
              this.tips.showInitFail();
            }
            this.employee.dismissOne(this.epBank[0].name);
          });
      })
      .catch((error) => {
        if (error !== undefined) {
          this.logger.error(error, 'ngOnInit:catch:observeTask:catch:');
          this.tips.setRbTip('getParaFailFreshPlease');
          this.employee.dismissOne(this.epBank[0].name);
        }
      });
  }

  ngOnDestroy() {
    if (this.urlOb) {
      this.urlOb.unsubscribe();
    }
  }

  onSubmit() {
    this.employee.hire(this.epBank[1]);
    this.getInfoFromChild();
    this.pfs.formatInt(this.RegionForm.value);
    this.cfg.putMotionRegionInterface(this.RegionForm.value).subscribe(
      res => {
        this.employee.doTask(this.epBank[1].name, this.epBank[1].task[0], true, true);
        this.resError.analyseRes(res);
        this.RegionForm.patchValue(res);
      },
      err => {
        this.logger.log(err, 'onSubmit:putMotionRegionInterface:');
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
        this.logger.log(err, 'onSubmit:putLinkageInterface:');
        this.employee.doTask(this.epBank[1].name, this.epBank[1].task[2], false, true);
      }
    );

    this.employee.observeTask(this.epBank[1].name, 5000)
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
        (error) => {
          this.logger.log(error, 'onSubmit:observeTask:');
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

  getInfoFromChild(childName: string = this.lastActiveTab) {
    switch (childName) {
      case 'regionSetting':
        const formValue = this.regionChild.getMotionRegionForm();
        this.RegionForm.patchValue(formValue);
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
        if (this.RegionForm.value && this.src) {
          this.regionChild.initComponent(this.RegionForm.value, this.src);
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

  setChildPara(childName: string) {
    this.pfs.waitNavActive(5000, childName + 'Tab')
      .then(() => {
        this.setInfoForChild(childName);
      })
      .catch((error) => {
        this.logger.error(error, 'setChildPara fail, chlidName is' + childName);
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

  async initPlayer(tabName: string) {
    this.pfs.waitNavActive(5000, tabName + 'Tab')
      .then(() => {
        this.setInfoForChild(tabName);
        this.regionChild.playEntry();
      })
      .catch((error) => {
        this.logger.error(error, 'initPlayer:catch');
        this.tips.setRbTip('initPlayerFailFreshPlease');
      });
  }

  async resizeTimeTable(tabName: string) {
    this.pfs.waitNavActive(5000, tabName + 'Tab')
      .then(() => {
        // this.setInfoForChild(tabName);
        this.armingChild.resizeTable();
      })
      .catch((error) => {
        this.logger.error(error, 'resizeTimeTable:catch:');
        this.tips.setRbTip('initScheduleFailFreshPlease');
      });
  }
}
