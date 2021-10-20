import { Component, OnInit, AfterViewInit, OnDestroy, ViewChild, ElementRef, Renderer2, HostListener, Input } from '@angular/core';
import { FormBuilder, Validators, FormControl } from '@angular/forms';

import { isNumberJudge } from 'src/app/shared/validators/benumber.directive';
import { CalculationCompare } from 'src/app/shared/validators/calculation.directive';
import { ConfigService } from 'src/app/config.service';
import { ColorCell, TimeTableOption } from './TimeTableInterface';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { TipsService } from 'src/app/tips/tips.service';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-time-table',
  templateUrl: './time-table.component.html',
  styleUrls: ['./time-table.component.scss']
})
export class TimeTableComponent implements OnInit, AfterViewInit {

  @Input() option: TimeTableOption;

  @ViewChild('canvasbox', { static: true })
  canvasBox: ElementRef<HTMLCanvasElement>;
  @ViewChild('canvasdraw', { static: true })
  canvasDraw: ElementRef<HTMLCanvasElement>;
  @ViewChild('canvasbg', { static: true })
  canvasBg: ElementRef<HTMLCanvasElement>;
  @ViewChild('modify', { static: true })
  modifyChild: ElementRef;

  constructor(
    private el: ElementRef,
    private Re2: Renderer2,
    private fb: FormBuilder,
    private cfg: ConfigService,
    private ieCss: IeCssService,
    private resError: ResponseErrorService,
    private tips: TipsService,
    private pfs: PublicFuncService,
  ) { }

  private ctx: any;
  private drawer: any;
  private rect: any;

  private logger: Logger = new Logger('time-table');

  objectKeys = this.pfs.objectKeys;
  isChrome: boolean = false;
  private devicePixelRatio: number;
  private aspect: number = 8 / 18;
  private isViewInit: boolean = false;
  title: string = 'title';
  private isShowChange: boolean = false;
  selectedType: string = '';
  typeList: ColorCell[];

  advancePara = [
    {
      name: 'test1',
      options: ['1', '2', '3', '4']
    },
    {
      name: 'test1',
      options: ['1', '2', '3', '4']
    },
    {
      name: 'test1',
      options: ['1', '2', '3', '4']
    },
    {
      name: 'test1',
      options: [1, 2, 3, 4]
    },
  ];

  backgroundPara:
  { standHeight: number;
    xName: number;
    nameWidth: number;
    xRect: number;
    rectWidth: number;
    xPic: number;
    picWidth: number;
    fontSize: number;
    hourGap: number;
    backgroundList: any;
  } = {
    standHeight: NaN,
    xName: NaN,
    nameWidth: NaN,
    xRect: NaN,
    rectWidth: NaN,
    xPic: NaN,
    picWidth: NaN,
    fontSize: NaN,
    hourGap: NaN,
    backgroundList: [
      {
        name: '星期一/Mon',
        y: NaN,
      },
      {
        name: '星期二/Tue',
        y: NaN,
      },
      {
        name: '星期三/Wed',
        y: NaN,
      },
      {
        name: '星期四/Thu',
        y: NaN,
      },
      {
        name: '星期五/Fri',
        y: NaN,
      },
      {
        name: '星期六/Sat',
        y: NaN,
      },
      {
        name: '星期日/Sun',
        y: NaN,
      },
    ],
  };

  AdvanceForm = this.fb.group({
    id: [''],
    iEnabled: 0,
  });

  ChangeForm = this.fb.group({
    hour0: [0, [isNumberJudge, Validators.min(0), Validators.max(24)]],
    minute0: [0, [isNumberJudge, Validators.min(0), Validators.max(59)]],
    hour: [0, [isNumberJudge, Validators.min(0), Validators.max(24)]],
    minute: [1, [isNumberJudge, Validators.min(0), Validators.max(59)]],
  }, {validators: CalculationCompare([{hour: 60, minute: 1}, {hour0: 60, minute0: 1}])});

  get hour(): FormControl {
    return this.ChangeForm.get('hour') as FormControl;
  }
  get minute(): FormControl {
    return this.ChangeForm.get('minute') as FormControl;
  }
  get hour0(): FormControl {
    return this.ChangeForm.get('hour0') as FormControl;
  }
  get minute0(): FormControl {
    return this.ChangeForm.get('minute0') as FormControl;
  }

  WeekForm = this.fb.group({
    monday: false,
    tuesday: false,
    wednesday: false,
    thursday: false,
    friday: false,
    saturday: false,
    sunday: false
  });

  signalBank = {
    init: {
      defaultColor: null,
      schedule: null,
    },
    reset: (signalName: string) => {
      for (const key of this.objectKeys(this.signalBank[signalName])) {
        this.signalBank[signalName][key] = null;
      }
    },
  };

  weekPara = {
    line: 0,
    isAllChecked : false,
    number2Weekday: {
      0: 'monday',
      1: 'tuesday',
      2: 'wednesday',
      3: 'thursday',
      4: 'friday',
      5: 'saturday',
      6: 'sunday',
    }
  };

  changeFormPara = {
    isChanging: false,
    blob: NaN,
    line: NaN,
  };

  mouseMove = {
    x: NaN,
    y: NaN,
    x0: NaN,
    y0: NaN,
    clear: () => {
      this.mouseMove.x = NaN;
      this.mouseMove.y = NaN;
      this.mouseMove.x0 = NaN;
      this.mouseMove.y0 = NaN;
    },
    push: (lox: number, loy: number) => {
      this.mouseMove.x0 = this.mouseMove.x;
      this.mouseMove.y0 = this.mouseMove.y;
      this.mouseMove.x = lox;
      this.mouseMove.y = loy;
    },
    deltaX: () => {
      if (isNaN(this.mouseMove.x) || isNaN(this.mouseMove.x0)) {
        return 0;
      } else {
        return this.mouseMove.x - this.mouseMove.x0;
      }
    },
    deltaY: () => {
      if (isNaN(this.mouseMove.y) || isNaN(this.mouseMove.y0)) {
        return 0;
      } else {
        return this.mouseMove.y - this.mouseMove.y0;
      }
    }
  };

  drawerObj = {
    activeLine: NaN,
    isDrawArea: false,
    activeBlob: NaN,
    hideColor: '#f8f8f8',
    fillColor: 'skyblue',
    drawStatus: 'null',
    ponitDire: 'x',
    minGap: 1 / 1440,
    itemNum: 7,
    activePonit: {
      x0: NaN,
      x: NaN,
      y: NaN,
      width: 5,
      height: 5,
      pointLine: NaN,
      clear: () => {
        this.drawerObj.activePonit.x0 = NaN;
        this.drawerObj.activePonit.x = NaN;
        this.drawerObj.activePonit.y = NaN;
      },
      clickOnPoint: (lox: number, loy: number) => {
        if (loy >= this.drawerObj.activePonit.y && loy <= this.drawerObj.activePonit.y + this.drawerObj.activePonit.height) {
          if (lox >= this.drawerObj.activePonit.x && lox <= this.drawerObj.activePonit.x + this.drawerObj.activePonit.width) {
            this.drawerObj.activePonit.ponitDire = 'x';
            return 'x';
          } else if (lox >= this.drawerObj.activePonit.x0 && lox <= this.drawerObj.activePonit.x0 + this.drawerObj.activePonit.width) {
            this.drawerObj.activePonit.ponitDire = 'x0';
            return 'x0';
          } else {
            const line = this.drawerObj.activeLine;
            const blob = this.drawerObj.activeBlob;
            if (lox > this.drawerObj[line][blob]['x0'] * this.backgroundPara.rectWidth &&
            lox < this.drawerObj[line][blob]['x'] * this.backgroundPara.rectWidth) {
              return 'in';
            }
          }
        }
        return 'null';
      }
    },
    0: [],
    1: [],
    2: [],
    3: [],
    4: [],
    5: [],
    6: [],
    type2color: {},
    color2type: {},
    clear: () => {
      for (let i = 0; i < this.drawerObj.itemNum; i++) {
        this.drawerObj[i] = [];
      }
    },
    set: (resp: any) => {
      if (resp['sSchedulesJson'] && resp['sSchedulesJson'].length > 0) {
        this.drawerObj.clear();
        for (let i = 0; i < this.drawerObj.itemNum; i++) {
          for (const timePat of resp['sSchedulesJson'][i]) {
            this.drawerObj[i].push(
              {
                x0: timePat['start'],
                x: timePat['end'],
                color: this.drawerObj.type2color[timePat['type']],
              }
            );
          }
        }
      }
    },
    get: () => {
      const jsonObj = [];
      for (let i = 0; i < this.drawerObj.itemNum; i++) {
        const patObj = [];
        for (const item of this.drawerObj[i]) {
          const onePat = {
            start: item['x0'],
            end: item['x'],
            type: this.drawerObj.color2type[item['color']],
          };
          patObj.push(onePat);
        }
        jsonObj.push(patObj);
      }
      return JSON.stringify(jsonObj);
    },
    getOrder: (localPercentage: number, line: number = this.drawerObj.activeLine): number => {
      const len = this.drawerObj[this.drawerObj.activeLine].length;
      if (len === 0) {
        return 0;
      } else if (localPercentage < this.drawerObj[line][0]['x0']) {
        return 0;
      } else if (localPercentage > this.drawerObj[line][len - 1]['x']) {
        return len;
      } else {
        for (let i = 0; i < len - 1; i++) {
          if (localPercentage > this.drawerObj[line][i]['x'] && localPercentage < this.drawerObj[line][i + 1]['x0']) {
            return i + 1;
          }
        }
      }
    },
    doubleCheck: (pointName: string, deltaX: number) => {
      const line = this.drawerObj.activeLine;
      const blob = this.drawerObj.activeBlob;
      if (this.drawerObj[line][blob]['x'] - this.drawerObj[line][blob]['x0'] <= 6) {
        if (deltaX > 0) {
          return 'x';
        } else {
          return 'x0';
        }
      } else {
        return pointName;
      }
    },
    percentage2Time: (percentage: number) => {
      if (percentage < 0) {
        return '00:00';
      } else if (percentage > 1) {
        return '24:00';
      }
      const minutes = Math.round(percentage * 1440);
      const hour = parseInt((minutes / 60).toString(), 10);
      const minute = minutes % 60;
      let timeString = '';
      if (hour < 10) {
        timeString = '0' + hour + ':';
      } else {
        timeString = hour + ':';
      }
      if (minute < 10) {
        timeString = timeString + '0' + minute;
      } else {
        timeString += minute;
      }
      return timeString;
    },
  };

  ngOnInit(): void {
    this.isChrome = this.ieCss.getChromeBool();
    this.devicePixelRatio = window.devicePixelRatio;
    if (this.option) {
      if (this.option['isInit']) {
        this.cfg.getSchedule(this.option['id']).subscribe(
          res => {
            this.resError.analyseRes(res);
            if (res['sSchedulesJson']) {
              // const jsonRst = JSON.parse(res['sSchedulesJson']);
              // this.drawerObj.set({sSchedulesJson: jsonRst});
              this.signalBank.init.schedule = res['sSchedulesJson'];
            } else {
              this.signalBank.init.schedule = false;
            }
          },
          err => {
            this.signalBank.init.schedule = false;
            this.logger.error(err, 'ngOnInit:getSchedule:');
          }
        );
        if (this.option['paraId'] >= 0) {
          this.cfg.getDefaultPara(this.option['paraId']).subscribe(
            res => {
              this.typeList = JSON.parse(res.toString());
              this.selectedType = this.typeList[0].name;
              for (const item of this.typeList) {
                this.drawerObj.type2color[item.name] = item.color;
                this.drawerObj.color2type[item.color] = item.name;
              }
              this.signalBank.init.defaultColor = true;
            },
            err => {
              this.logger.error(err, 'ngOnInit:getDefaultPara:');
              this.signalBank.init.defaultColor = false;
            }
          );
        }
        if (this.option['advanceId'] === 0) {
          this.cfg.getPlanAdvancePara(this.option['advanceId']).subscribe(
            res => {
              this.AdvanceForm.patchValue(res);
            }
          );
        }
        this.waitSignal('init', 10000)
          .then(
            () => {
              this.reshapeCanvas();
              this.initDrawer(this.signalBank.init.schedule);
            })
          .catch(
            () => {
              this.waitForTip('initFailFreshPlease');
            }
          );
      }
    }
  }

  ngAfterViewInit(): void {
    this.isViewInit = true;
    this.resizeCanvas(this.aspect);
    this.ctx = this.canvasBg.nativeElement.getContext('2d');
    this.drawer = this.canvasDraw.nativeElement.getContext('2d');
    this.drawBackGround();

    this.canvasDraw.nativeElement.addEventListener('mousedown', (e: any) => {
      this.isShowChange = true;
      this.mouseMove.push(e.clientX - this.rect.left, e.clientY - this.rect.top);
      if (!isNaN(this.drawerObj.activeLine)) {
        const clickCopyBtn = this.isClickOnCopyBtn(this.mouseMove.x, this.mouseMove.y);
        if (isNaN(clickCopyBtn)) {
          let pointName = 'null';
          if (!isNaN(this.drawerObj.activeBlob)) {
            pointName = this.drawerObj.activePonit.clickOnPoint(this.mouseMove.x, this.mouseMove.y);
          }
          if (pointName === 'null') {
            this.isClickOnDrawArea(this.mouseMove.x);
          } else if (pointName !== 'in') {
            this.drawerObj.drawStatus = 'resize';
          } else {
            this.drawerObj.drawStatus = 'move';
          }
        } else {
          this.onShowModify(clickCopyBtn, e.clientX, e.clientY);
        }
      }
      if (this.drawerObj.drawStatus !== 'null') {
        this.onNo('.modal');
        this.onNo('.modify-table');
      }
    });

    this.canvasDraw.nativeElement.addEventListener('mousemove', (e: any) => {
      this.isShowChange = false;
      this.mouseMove.push(e.clientX - this.rect.left, e.clientY - this.rect.top);
      if (this.drawerObj.drawStatus === 'null') {
        this.checkClickLine(this.mouseMove.y);
      } else {
        this.shapeBlob(this.mouseMove.x, this.mouseMove.y, this.mouseMove.deltaX());
        this.drawTimeTip();
      }
      if (e.clientX < this.rect.left + 1
        || e.clientX > this.rect.right - 5
        || e.clientY < this.rect.top + 1
        || e.clientY > this.rect.bottom - 3) {
          this.drawerObj.drawStatus = 'null';
          this.clearTimeTip();
        }
    });

    this.canvasDraw.nativeElement.addEventListener('mouseup', (e: any) => {
      // this.mouseMove.clear();
      if (this.drawerObj.drawStatus === 'new' && (this.drawerObj[this.drawerObj.activeLine][this.drawerObj.activeBlob]['x']
        - this.drawerObj[this.drawerObj.activeLine][this.drawerObj.activeBlob]['x0']) < this.drawerObj.minGap) {
          const line = this.drawerObj.activeLine;
          this.drawerObj[this.drawerObj.activeLine].splice(this.drawerObj.activeBlob, 1);
          this.drawerObj.activeBlob = NaN;
          this.clearLine(line);
          this.drawLine(line);
      }
      this.clearTimeTip();
      this.drawerObj.drawStatus = 'null';
      if (this.isShowChange && !isNaN(this.drawerObj.activeBlob)) {
        const line = this.drawerObj.activeLine;
        const blob = this.drawerObj.activeBlob;
        const loy = this.backgroundPara.backgroundList[line].y;
        const lox = Math.ceil(this.drawerObj[line][blob]['x'] * this.backgroundPara.rectWidth) + this.backgroundPara.xRect;
        const lox0 = Math.ceil(this.drawerObj[line][blob]['x0'] * this.backgroundPara.rectWidth) + this.backgroundPara.xRect;
        this.mouseMove.push(e.clientX - this.rect.left, e.clientY - this.rect.top);
        if (this.mouseMove.y >= loy && this.mouseMove.y <= loy + this.backgroundPara.standHeight && this.mouseMove.x >= lox0
          && this.mouseMove.x <= lox) {
            this.setXY(e.clientX, e.clientY);
            this.onShow();
        }
      }
      this.mouseMove.clear();
    });
  }

  waitForTip(tip: string) {
    this.pfs.waitNavActive(20000, this.option.pageId)
      .then(
        () => {
          this.tips.setRbTip(tip);
        }
      )
      .catch();
  }

  waitSignal = (itemName: string, timeoutms: number) => new Promise(
    (resolve, reject) => {
      const checkNotNull = () => {
        timeoutms -= 100;
        let nullNum = 0;
        for (const key of this.objectKeys(this.signalBank[itemName])) {
          if (this.signalBank[itemName][key] === null) {
            nullNum += 1;
          }
        }
        if (nullNum === 0) {
          resolve();
        } else if (timeoutms <= 0) {
          reject();
        } else {
          setTimeout(checkNotNull, 100);
        }
      };
      checkNotNull();
    }
  )

  @HostListener('window:resize', ['$event'])
  onResize(event): void {
    if (this.isViewInit) {
      this.resizeCanvas(this.aspect);
      this.drawPic();
    }
  }

  reshapeCanvas() {
    if (this.isViewInit) {
      this.resizeCanvas(this.aspect);
      this.drawPic();
    }
  }

  drawPic() {
    this.clearCtx();
    this.drawBackGround();
    this.drawAll();
  }

  drawBackGround() {
    this.ctx.font = (this.backgroundPara.fontSize * 0.8).toString() + 'px bold 宋体';
    this.ctx.fillStyle = '#353a40';
    this.ctx.textAlign = 'left';
    this.ctx.textBaseline = 'top';
    this.ctx.strokeStyle = '#e4e7ec';
    this.ctx.lineWidth = 2;
    // write name
    for (let i = 0; i < this.drawerObj.itemNum; i++) {
      this.ctx.fillText(
        this.backgroundPara.backgroundList[i].name, this.backgroundPara.xName, this.backgroundPara.backgroundList[i].y
        );
    }
    // set cursor size and draw rect and cursor
    this.ctx.fillStyle = '#e4e7ec';
    this.ctx.font = Math.ceil(this.backgroundPara.fontSize / 2).toString() + 'px bold 宋体';
    for (let i = 0; i < this.drawerObj.itemNum; i++) {
      this.ctx.fillRect(
        this.backgroundPara.xRect, this.backgroundPara.backgroundList[i].y, this.backgroundPara.rectWidth, this.backgroundPara.standHeight
        );
      this.ctx.strokeRect(
        this.backgroundPara.xRect, this.backgroundPara.backgroundList[i].y, this.backgroundPara.rectWidth, this.backgroundPara.standHeight
        );
      this.drawCousor(this.backgroundPara.xRect, this.backgroundPara.backgroundList[i].y);
    }
    const img = new Image();
    img.src = '/assets/images/icon_copy_time_table.png';
    const that = this;
    img.onload = () => {
      for (let i = 0; i < this.drawerObj.itemNum; i++) {
        that.ctx.drawImage(
          img, that.backgroundPara.xPic, that.backgroundPara.backgroundList[i].y,
          that.backgroundPara.picWidth * 0.7, that.backgroundPara.standHeight * 0.8
          );
        that.hideAll();
      }
    };
  }

  drawCousor(x: number, y: number) {
    this.ctx.fillStyle = '#353a40';
    const height = this.backgroundPara.standHeight / 2;
    const gap = this.backgroundPara.hourGap;
    for (let i = 0; i <= 24; i++) {
      this.ctx.moveTo (x + i * gap, y);
      this.ctx.lineTo (x + i * gap, y - height);
      this.ctx.lineWidth = 1;
      this.ctx.stroke();
    }
    for (let i = 0; i <= 12; i++) {
      this.ctx.fillText((2 * i).toString(), x + i * gap * 2, y - height * 2);
    }
    this.ctx.fillStyle = '#e4e7ec';
  }

  clearCtx() {
    this.ctx.clearRect(0, 0, this.rect.width, this.rect.height);
  }

  resizeCanvas(aspect: number = this.aspect) {
    const boxWidth = this.canvasBox.nativeElement.clientWidth * this.devicePixelRatio;
    const boxHeight = boxWidth * aspect + 10;
    this.canvasBg.nativeElement.width = boxWidth;
    this.canvasBg.nativeElement.height = boxHeight;
    this.canvasDraw.nativeElement.width = boxWidth;
    this.canvasDraw.nativeElement.height = boxHeight;
    // define x of background
    const xGap = Math.ceil(boxWidth * 0.01);
    const perWidth = Math.ceil(boxWidth * 0.032);
    this.backgroundPara.hourGap = perWidth;
    this.backgroundPara.nameWidth = perWidth * 4;
    this.backgroundPara.rectWidth = perWidth * 24;
    this.backgroundPara.picWidth = perWidth;
    this.backgroundPara.xName = xGap;
    this.backgroundPara.xRect = this.backgroundPara.xName + this.backgroundPara.nameWidth + xGap;
    this.backgroundPara.xPic = this.backgroundPara.xRect + this.backgroundPara.rectWidth + xGap;
    // define y of background
    const perHeight = Math.ceil((boxWidth * aspect) / 39);
    this.backgroundPara.fontSize = Math.ceil(1.6 * perHeight);
    this.backgroundPara.standHeight = 2 * perHeight;
    for (let i = 0; i < this.drawerObj.itemNum; i++) {
      this.backgroundPara.backgroundList[i].y = perHeight * 3 + i * 5 * perHeight;
    }
    this.rect = this.canvasBg.nativeElement.getBoundingClientRect();
  }

  setXY(x: number, y: number) {
    this.Re2.setStyle(this.el.nativeElement.querySelector('.modal'), 'top', y + 'px');
    this.Re2.setStyle(this.el.nativeElement.querySelector('.modal'), 'left', x + 'px');
  }

  onNo(name: string = '') {
    this.changeFormPara.isChanging = false;
    this.Re2.setStyle(this.el.nativeElement.querySelector(name), 'display', 'none');
  }

  onShow() {
    if (!this.changeFormPara.isChanging) {
      this.changeFormPara.isChanging = true;
      this.changeFormPara.line = this.drawerObj.activeLine;
      this.changeFormPara.blob = this.drawerObj.activeBlob;
      this.onSetChangeForm();
      this.Re2.setStyle(this.el.nativeElement.querySelector('.modal'), 'display', 'block');
    }
  }

  onSetChangeForm() {
    const line = this.changeFormPara.line;
    const blob = this.changeFormPara.blob;
    const x = this.drawerObj[line][blob]['x'];
    const x0 = this.drawerObj[line][blob]['x0']
    const ms0 = Math.round(x0 * 1440);
    const h0 = parseInt((ms0 / 60).toString(), 10);
    const m0 = ms0 % 60;
    const ms = Math.round(x * 1440);
    const h = parseInt((ms / 60).toString(), 10);
    const m = ms % 60;
    this.ChangeForm.patchValue(
      {
        hour0: h0,
        minute0: m0,
        hour: h,
        minute: m,
      }
    );
  }

  onDelete() {
    const line = this.changeFormPara.line;
    const blob = this.changeFormPara.blob;
    if (!isNaN(line) && !isNaN(blob)) {
      this.drawerObj[line].splice(blob, 1);
      this.clearLine(line);
      this.drawLine(line);
      this.onNo('.modal');
    }
  }

  onDeleteOne() {
    const line = this.drawerObj.activePonit.pointLine;
    const blob = this.drawerObj.activeBlob;
    if (!isNaN(line) && !isNaN(blob)) {
      this.drawerObj[line].splice(blob, 1);
      this.drawerObj.activePonit.clear();
      this.clearLine(line);
      this.drawLine(line);
    }
  }

  onDeleteAll() {
    for (let i = 0; i < this.drawerObj.itemNum; i++) {
      this.drawerObj[i] = [];
    }
    this.drawerObj.activeLine = NaN;
    this.drawerObj.activeBlob = NaN;
    this.drawerObj.activePonit.clear();
    this.clearAll();
    this.drawAll();
  }

  onSave() {
    const line = this.changeFormPara.line;
    const blob = this.changeFormPara.blob;
    const ms = (Number(this.ChangeForm.value.hour0) * 60 + Number(this.ChangeForm.value.minute0)) / 1440;
    const me = (Number(this.ChangeForm.value.hour) * 60 + Number(this.ChangeForm.value.minute)) / 1440;
    const len = this.drawerObj[line].length;
    let leftEdge = 0;
    let rightEdge = 1;
    // make sure edge
    if (blob >= 1) {
      leftEdge = this.drawerObj[line][blob - 1]['x'];
    }
    if (blob < len - 1) {
      rightEdge = this.drawerObj[line][blob + 1]['x0'];
    }
    if (ms < leftEdge || me > rightEdge) {
      this.tips.showCTip('timeCanNotBeOverlapping');
      this.tips.setCTPara('oneQuie');
      return;
    }
    this.drawerObj[line][blob]['x0'] = ms;
    this.drawerObj[line][blob]['x'] = me;
    this.clearLine(line);
    this.drawLine(line);
    this.onNo('.modal');
  }

  onShowModify(line: number, x: number, y: number) {
    if (!this.changeFormPara.isChanging) {
      this.weekPara.line = line;
      this.changeFormPara.isChanging = true;
      this.Re2.setStyle(this.modifyChild.nativeElement, 'display', 'block');
      const lox = x - this.modifyChild.nativeElement.clientWidth;
      this.Re2.setStyle(this.modifyChild.nativeElement, 'top', y + 'px');
      this.Re2.setStyle(this.modifyChild.nativeElement, 'left', lox + 'px');
      for (let i = 0; i < this.drawerObj.itemNum; i++) {
        if (i === line) {
          this.WeekForm.get(this.weekPara.number2Weekday[i]).setValue(true);
          this.WeekForm.get(this.weekPara.number2Weekday[i]).disable();
        } else {
          this.WeekForm.get(this.weekPara.number2Weekday[i]).setValue(false);
          this.WeekForm.get(this.weekPara.number2Weekday[i]).enable();
        }
      }
    }
  }

  selectAll() {
    this.weekPara.isAllChecked = !this.weekPara.isAllChecked;
    for (let i = 0; i < this.drawerObj.itemNum; i++) {
      if (this.WeekForm.get(this.weekPara.number2Weekday[i]).enabled) {
        this.WeekForm.get(this.weekPara.number2Weekday[i]).setValue(this.weekPara.isAllChecked);
      }
    }
  }

  onConfirm() {
    const lineList = this.drawerObj[this.weekPara.line];
    for (let i = 0; i < this.drawerObj.itemNum; i++) {
      if (this.WeekForm.get(this.weekPara.number2Weekday[i]).enabled && this.WeekForm.get(this.weekPara.number2Weekday[i]).value) {
        this.drawerObj[i] = [];
        for (const item of lineList) {
          if (item['color']) {
            this.drawerObj[i].push({x: item.x, x0: item.x0, color: item.color});
          } else {
            this.drawerObj[i].push({x: item.x, x0: item.x0});
          }
        }
        this.clearLine(i);
        this.drawLine(i);
      }
    }
    this.onNo('.modify-table');
  }

  hideCopyBtn(index: number) {
    if (!isNaN(index) && this.backgroundPara.backgroundList[index]) {
      this.drawer.fillStyle = this.drawerObj.hideColor;
      this.drawer.fillRect(this.backgroundPara.xPic, this.backgroundPara.backgroundList[index].y,
        this.backgroundPara.picWidth, this.backgroundPara.standHeight);
    }
  }

  hideAll() {
    for (let i = 0; i < this.drawerObj.itemNum; i++) {
      this.hideCopyBtn(i);
    }
  }

  checkClickLine(y: number): number {
    for (let i = 0; i < this.drawerObj.itemNum; i++) {
      const loY = this.backgroundPara.backgroundList[i].y;
      if (y >= loY && y <= loY + this.backgroundPara.standHeight) {
        this.hideCopyBtn(this.drawerObj.activeLine);
        this.drawerObj.activeLine = i;
        this.showCopyBtn(this.drawerObj.activeLine);
        return i;
      }
    }
    this.hideCopyBtn(this.drawerObj.activeLine);
    this.drawerObj.activeLine = NaN;
    return NaN;
  }

  setActivePoint() {
    this.clearLine(this.drawerObj.activePonit.pointLine);
    this.drawLine(this.drawerObj.activePonit.pointLine);
    const line = this.drawerObj.activeLine;
    this.drawerObj.activePonit.pointLine = line;
    const blob = this.drawerObj.activeBlob;
    if (isNaN(line) || isNaN(blob)) {
      this.drawerObj.activePonit.clear();
      return;
    }
    const xs = this.backgroundPara.xRect;
    const width = this.backgroundPara.rectWidth;
    const lox = Math.ceil(xs + this.drawerObj[line][blob]['x'] * width);
    const lox0 = Math.ceil(xs + this.drawerObj[line][blob]['x0'] * width);
    this.drawerObj.activePonit.x = lox - 2;
    this.drawerObj.activePonit.x0 = lox0 - 2;
    this.drawerObj.activePonit.y = Math.ceil(this.backgroundPara.backgroundList[line].y + this.backgroundPara.standHeight / 2) - 2;
    this.drawPoint();
  }

  showCopyBtn(index: number) {
    if (!isNaN(index) && this.backgroundPara.backgroundList[index]) {
      this.drawer.clearRect(this.backgroundPara.xPic, this.backgroundPara.backgroundList[index].y,
        this.backgroundPara.picWidth, this.backgroundPara.standHeight);
    }
  }

  isClickOnDrawArea(lox: number) {
    if (!isNaN(this.drawerObj.activeLine)) {
      this.drawerObj.isDrawArea = (lox >= this.backgroundPara.xRect && lox <= this.backgroundPara.xRect + this.backgroundPara.rectWidth);
      const len = this.drawerObj[this.drawerObj.activeLine].length;
      lox = lox - this.backgroundPara.xRect;
      if (this.drawerObj.isDrawArea) {
        for (let i = 0; i < len; i++) {
          if (lox >= (this.drawerObj[this.drawerObj.activeLine][i]['x0'] * this.backgroundPara.rectWidth - 1) &&
            lox <= this.drawerObj[this.drawerObj.activeLine][i]['x'] * this.backgroundPara.rectWidth + 1) {
            this.drawerObj.activeBlob = i;
            this.drawerObj.drawStatus = 'move';
            this.setActivePoint();
            return true;
          }
        }
        if (len < 8) {
          const localPercentage = lox / this.backgroundPara.rectWidth;
          const order = this.drawerObj.getOrder(localPercentage);
          this.drawerObj[this.drawerObj.activeLine].splice(order, 0,
            {x: localPercentage, x0: localPercentage, color: this.drawerObj.type2color[this.selectedType]});
          this.drawerObj.activeBlob = order;
          this.drawerObj.drawStatus = 'new';
          this.drawerObj.activePonit.ponitDire = 'null';
          this.drawerObj.activePonit.clear();
          return true;
        }
      }
    }
    this.drawerObj.activeBlob = NaN;
    this.drawerObj.activePonit.clear();
    this.clearLine(this.drawerObj.activePonit.activeLine);
    this.drawLine(this.drawerObj.activePonit.activeLine);
    this.drawerObj.drawStatus = 'null';
    return false;
  }

  shapeBlob(x: number, y: number, deltaX: number) {
    this.clearTimeTip();
    switch (this.drawerObj.drawStatus) {
      case 'move':
        this.moveBolb(deltaX);
        break;
      case 'resize':
        this.resizeBlob(this.drawerObj.activePonit.ponitDire, deltaX);
        break;
      case 'new':
        this.newBlob('', deltaX);
        break;
      case 'null':
        return;
    }
    this.drawTimeTip();
  }

  moveBolb(deltaX: number) {
    const line = this.drawerObj.activeLine;
    const blob = this.drawerObj.activeBlob;
    const len = this.drawerObj[line].length;
    this.clearLine(line);
    let leftEdge = 0;
    let rightEdge = 1;
    const percentageDeltaX = deltaX / this.backgroundPara.rectWidth;
    let xleft = this.drawerObj[line][blob]['x0'] + percentageDeltaX;
    let xright = this.drawerObj[line][blob]['x'] + percentageDeltaX;
    // make sure edge
    if (blob >= 1) {
      leftEdge = this.drawerObj[line][blob - 1]['x'];
    }
    if (blob < len - 1) {
      rightEdge = this.drawerObj[line][blob + 1]['x0'];
    }
    // adjust by edge
    if (xleft < leftEdge) {
      xright = xright + leftEdge - xleft;
      xleft = leftEdge;
    } else if (xright > rightEdge) {
      xleft = xleft - (xright - rightEdge);
      xright = rightEdge;
    }
    this.drawerObj[line][blob]['x0'] = xleft;
    this.drawerObj[line][blob]['x'] = xright;
    this.drawLine(line);
    this.setActivePoint();
  }

  changeBlob(pointName: string, deltaX: number) {
    const line = this.drawerObj.activeLine;
    const blob = this.drawerObj.activeBlob;
    const len = this.drawerObj[line].length;
    const percentageDeltaX = deltaX / this.backgroundPara.rectWidth;
    let nowX = this.drawerObj[line][blob][pointName] + percentageDeltaX;
    let leftEdge = 0;
    let rightEdge = 1;
    if (pointName === 'x') {
      leftEdge = this.drawerObj[line][blob]['x0'] + this.drawerObj.minGap;
      if (blob < len - 1) {
        rightEdge = this.drawerObj[line][blob + 1]['x0'];
      }
    } else {
      rightEdge = this.drawerObj[line][blob]['x'] - this.drawerObj.minGap;
      if (blob >= 1) {
        leftEdge = this.drawerObj[line][blob - 1]['x'];
      }
    }
    if (nowX < leftEdge) {
      nowX = leftEdge;
    } else if (nowX > rightEdge) {
      nowX = rightEdge;
    }
    this.drawerObj[line][blob][pointName] = nowX;
  }

  newBlob(pointName: string, deltaX: number) {
    const line = this.drawerObj.activeLine;
    const blob = this.drawerObj.activeBlob;
    this.clearBlob();
    if (!isNaN(line) && !isNaN(blob)) {
      if (pointName === '' && this.drawerObj[line][blob]['x'] === this.drawerObj[line][blob]['x0'] &&
        this.drawerObj.activePonit.ponitDire === 'null') {
        if (deltaX > 0) {
          pointName = 'x';
        } else {
          pointName = 'x0';
        }
        this.drawerObj.ponitDire = pointName;
      } else if (pointName === '') {
        pointName = this.drawerObj.ponitDire;
      }
      this.changeBlob(pointName, deltaX);
    }
    this.drawBlob();
  }

  resizeBlob(pointName: string, deltaX: number) {
    const line = this.drawerObj.activeLine;
    this.clearLine(line);
    this.changeBlob(pointName, deltaX);
    this.drawLine(line);
    this.setActivePoint();
  }

  clearLine(line: number) {
    if (isNaN(line)) {
      return;
    }
    this.drawer.clearRect(0, this.backgroundPara.backgroundList[line].y, this.backgroundPara.xPic, this.backgroundPara.standHeight + 1);
  }

  clearAll() {
    for (let i = 0; i < this.drawerObj.itemNum; i++) {
      this.clearLine(i);
    }
  }

  clearBlob(line: number = this.drawerObj.activeLine, blob: number = this.drawerObj.activeBlob) {
    const x = Math.ceil(this.drawerObj[line][blob]['x'] * this.backgroundPara.rectWidth + this.backgroundPara.xRect);
    const x0 = Math.ceil(this.drawerObj[line][blob]['x0'] * this.backgroundPara.rectWidth + this.backgroundPara.xRect);
    const width = x - x0 + 1;
    const y = this.backgroundPara.backgroundList[line].y;
    this.drawer.clearRect(x0, y, width, this.backgroundPara.standHeight + 1);
  }

  drawBlob(line: number = this.drawerObj.activeLine, blob: number = this.drawerObj.activeBlob) {
    if (this.drawerObj[line][blob]['color']) {
      this.drawer.fillStyle = this.drawerObj[line][blob]['color'];
    } else {
      this.drawer.fillStyle = this.drawerObj.fillColor;
    }
    const x = Math.ceil(this.drawerObj[line][blob].x * this.backgroundPara.rectWidth + this.backgroundPara.xRect);
    const x0 = Math.ceil(this.drawerObj[line][blob].x0 * this.backgroundPara.rectWidth + this.backgroundPara.xRect);
    const width = x - x0 + 1;
    const y = this.backgroundPara.backgroundList[line].y + 1;
    this.drawer.fillRect(x0, y, width, this.backgroundPara.standHeight);
  }

  drawAll() {
    // this.showCopyBtn(this.drawerObj.activeLine);
    for (let i = 0; i < this.drawerObj.itemNum; i++) {
      const len = this.drawerObj[i].length;
      for (let j = 0; j < len; j++) {
        this.drawBlob(i, j);
      }
    }
  }

  drawLine(line: number = this.drawerObj.activeLine) {
    if (isNaN(line)) {
      return;
    }
    const len = this.drawerObj[line].length;
    for (let i = 0; i < len; i++) {
      this.drawBlob(line, i);
    }
  }

  drawEdge() {
    const line = this.drawerObj.activeLine;
    const blob = this.drawerObj.activeBlob;
    if (!isNaN(line) && !isNaN(blob)) {
      const x = Math.ceil(this.drawerObj[line][blob].x * this.backgroundPara.rectWidth + this.backgroundPara.xRect);
      const x0 = Math.ceil(this.drawerObj[line][blob].x0 * this.backgroundPara.rectWidth + this.backgroundPara.xRect);
      const y = this.backgroundPara.backgroundList[line].y + 1;
      this.drawer.strokeStyle = '#404040';
      this.drawer.lineWidth = 1;
      this.drawer.setLineDash([1]);
      this.drawer.beginPath();
      this.drawer.moveTo(x0, y);
      this.drawer.lineTo(x, y);
      this.drawer.lineTo(x, y + this.backgroundPara.standHeight - 1);
      this.drawer.lineTo(x0, y + this.backgroundPara.standHeight - 1);
      this.drawer.lineTo(x0, y);
      this.drawer.closePath();
      this.drawer.stroke();
      this.drawer.setLineDash([]);
    }
  }

  drawPoint() {
    if (!isNaN(this.drawerObj.activePonit.x)) {
      this.drawEdge();
      this.drawer.strokeStyle = '#353a40';
      this.drawer.fillStyle = '#ffffff';
      this.drawer.strokeRect(this.drawerObj.activePonit.x, this.drawerObj.activePonit.y,
        this.drawerObj.activePonit.width, this.drawerObj.activePonit.height);
      this.drawer.strokeRect(this.drawerObj.activePonit.x0, this.drawerObj.activePonit.y,
        this.drawerObj.activePonit.width, this.drawerObj.activePonit.height);
      this.drawer.fillRect(this.drawerObj.activePonit.x, this.drawerObj.activePonit.y,
        this.drawerObj.activePonit.width, this.drawerObj.activePonit.height);
      this.drawer.fillRect(this.drawerObj.activePonit.x0, this.drawerObj.activePonit.y,
        this.drawerObj.activePonit.width, this.drawerObj.activePonit.height);
    }
  }

  isClickOnCopyBtn(x: number, y: number) {
    if (isNaN(this.drawerObj.activeLine)) {
      return NaN;
    } else {
      if (x >= this.backgroundPara.xPic && x <= this.backgroundPara.xPic + this.backgroundPara.picWidth) {
        for (let i = 0; i < this.drawerObj.itemNum; i++) {
          if (y >= this.backgroundPara.backgroundList[i].y &&
            y <= this.backgroundPara.backgroundList[i].y + this.backgroundPara.standHeight) {
              return i;
          }
        }
        return NaN;
      } else {
        return NaN;
      }
    }
  }

  drawTimeTip() {
    const line = this.drawerObj.activeLine;
    const blob = this.drawerObj.activeBlob;
    const start = this.backgroundPara.xRect;
    const rectWidth = this.backgroundPara.rectWidth;
    const x = this.drawerObj[line][blob]['x'] * rectWidth + start;
    const x0 = this.drawerObj[line][blob]['x0'] * rectWidth + start;
    const y = this.backgroundPara.backgroundList[line].y;
    const height = this.backgroundPara.standHeight;
    const cell = Math.ceil(Math.min(height / 5, rectWidth / 120));
    this.drawTimeBlock(x0, y, cell);
    this.drawTimeContent(x0, y, cell, this.drawerObj.percentage2Time(this.drawerObj[line][blob]['x0']));
    this.drawTimeBlock(x, y, cell);
    this.drawTimeContent(x, y, cell, this.drawerObj.percentage2Time(this.drawerObj[line][blob]['x']));
  }

  drawTimeBlock(x0: number, y: number, cell: number) {
    const gap = 2;
    this.drawer.fillStyle = 'black';
    this.drawer.beginPath();
    this.drawer.moveTo(x0, y - gap);
    this.drawer.lineTo(x0 - cell, y - gap - cell);
    this.drawer.lineTo(x0 - cell * 5, y - gap - cell);
    this.drawer.lineTo(x0 - cell * 5, y - gap - cell * 4);
    this.drawer.lineTo(x0 + cell * 5, y - gap - cell * 4);
    this.drawer.lineTo(x0 + cell * 5, y - gap - cell);
    this.drawer.lineTo(x0 + cell, y - gap - cell);
    this.drawer.lineTo(x0, y - gap);
    this.drawer.closePath();
    this.drawer.fill();
  }

  drawTimeContent(x0: number, y: number, cell: number, content: string) {
    const start = x0 - Math.ceil(content.length * cell / 2);
    this.drawer.font = (cell * 2).toString() + 'px bold 宋体';
    this.drawer.fillStyle = '#ffffff';
    this.drawer.fillText(content, start, y - cell * 2);
  }

  clearTimeTip() {
    const line = this.drawerObj.activeLine;
    if (!isNaN(line)) {
      const height = this.backgroundPara.standHeight;
      const width = this.backgroundPara.xPic + this.backgroundPara.picWidth;
      this.drawer.clearRect(0, this.backgroundPara.backgroundList[line].y - height, width, height - 1);
    }
  }

  initDrawer(jsString: string) {
    if (jsString) {
      const jsonRst = JSON.parse(jsString);
      this.drawerObj.set({sSchedulesJson: jsonRst});
    }
    this.drawAll();
  }

  getsJsonString() {
    return this.drawerObj.get();
  }

  parentInitDrawer(drawObj: any) {
    this.drawerObj.set(drawObj);
  }

  onSelfSave() {
    const data = {
      sSchedulesJson: this.drawerObj.get(),
    };
    this.cfg.setSchedule(this.option['id'], data).subscribe(
      res => {
        this.resError.analyseRes(res);
        this.initDrawer(res['sSchedulesJson']);
        this.tips.showSaveSuccess();
      },
      err => {
        this.tips.showSaveFail();
        this.logger.error(err, 'onSelfSave:setSchedule:');
      }
    );
    if (this.option.isAdvance) {
      this.pfs.formatInt(this.AdvanceForm.value);
      this.cfg.setPlanAdvancePara(this.option.advanceId, this.AdvanceForm.value).subscribe(
        res => {
          this.resError.analyseRes(res);
          this.AdvanceForm.patchValue(res);
        },
        err => {
          this.logger.error(err, 'onSelfSave:setPlanAdvancePara:');
          console.error(err);
        }
      );
    }
  }
}
