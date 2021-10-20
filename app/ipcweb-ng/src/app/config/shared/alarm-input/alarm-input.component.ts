import { Component, OnInit, ViewChild, ElementRef, Renderer2 } from '@angular/core';
import { FormBuilder } from '@angular/forms';

import { ConfigService } from 'src/app/config.service';
import { TimeTableOption } from 'src/app/config/shared/time-table/TimeTableInterface';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { BoolEmployee, EmployeeItem } from 'src/app/shared/func-service/employee.service';
import { TipsService } from 'src/app/tips/tips.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { LastNavService } from 'src/app/shared/func-service/last-nav.service';

@Component({
  selector: 'app-alarm-input',
  templateUrl: './alarm-input.component.html',
  styleUrls: ['./alarm-input.component.scss']
})
export class AlarmInputComponent implements OnInit {

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
  ) { }

  isChrome: boolean = false;
  private navGroup: string = 'alarmInput';
  private linkagePath = 'ai_0';
  selectedTab = 'armingTime';
  private lastActiveTab: string = 'armingTime';
  private ScheduleForm: string = '';
  private LinkageForm: any;
  private scheduleId: number = 4;
  tabOption: TimeTableOption = {
    isInit: false,
    isType: false,
    isEnabledTop: false,
    isAdvance: false,
    advancePara: [],
    id: 4,
    paraId: NaN,
    pageId: 'armingTimeTab',
    advanceId: -1,
  };
  employee = new BoolEmployee();
  epBank: EmployeeItem[] = [
    {
      name: 'init',
      task: ['time', 'linkage']
    },
    {
      name: 'save',
      task: ['time', 'linkage']
    }
  ];

  menuItems: Array<string> = [
    'armingTime',
    'linkageMode',
  ];

  ngOnInit(): void {
    // this.isChrome = this.ieCss.getChromeBool();
    // // this.lns.getLastNav(this.navGroup, this.el);
    // this.employee.hire(this.epBank[0]);

    // this.cfg.getSchedule(this.scheduleId).subscribe(
    //   res => {
    //     this.resError.analyseRes(res);
    //     this.ScheduleForm = res['sSchedulesJson'];
    //     this.employee.doTask(this.epBank[0].name, this.epBank[0].task[0], true, true);
    //   },
    //   err => {
    //     console.error(err);
    //     this.employee.doTask(this.epBank[0].name, this.epBank[0].task[0], false, true);
    //   }
    // );

    // this.cfg.getLinkageInterface(this.linkagePath).subscribe(
    //   res => {
    //     this.resError.analyseRes(res);
    //     this.LinkageForm = res;
    //     this.employee.doTask(this.epBank[0].name, this.epBank[0].task[1], true, true);
    //   },
    //   err => {
    //     console.error(err);
    //     this.employee.doTask(this.epBank[0].name, this.epBank[0].task[1], false, true);

    //   }
    // );
  }


  onSubmit() {
    this.employee.hire(this.epBank[1]);
    this.getInfoFromChild();

    this.saveSchedule();

    this.pfs.formatInt(this.LinkageForm);
    this.cfg.putLinkageInterface(this.LinkageForm, this.linkagePath).subscribe(
      res => {
        this.resError.analyseRes(res);
        this.employee.doTask(this.epBank[1].name, this.epBank[1].task[1], true, true);
        this.LinkageForm = res;
      },
      err => {
        console.log(err);
        this.employee.doTask(this.epBank[1].name, this.epBank[1].task[1], false, true);
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
        console.error(err);
        this.employee.doTask(this.epBank[1].name, this.epBank[1].task[1], false, true);
      }
    );
  }

  onClickTab(tabName: string) {
    if (tabName !== this.lastActiveTab) {
      this.lns.setLastNav(this.navGroup, tabName + 'Tab');
      this.selectedTab = tabName;
      this.getInfoFromChild();
      this.setChildPara(tabName);
      this.lastActiveTab = tabName;
    }
  }

  getInfoFromChild(childName: string = this.lastActiveTab) {
    switch (childName) {
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
      case 'armingTime':
        this.armingChild.initDrawer(this.ScheduleForm);
        break;
      case 'linkageMode':
        this.linkageChild.setLinkageForm(this.LinkageForm);
        break;
    }
  }

  setChildPara(childName: string) {
    this.pfs.waitNavActive(5000, childName + 'Tab')
      .then(() => {
        this.setInfoForChild(childName);
      })
      .catch(() => {
        this.tips.showInitFail();
      });
  }
}
