import { Component, OnInit, AfterViewInit, OnDestroy, ViewChild, ElementRef, Renderer2, HostListener, Input } from '@angular/core';

import { Subject } from 'rxjs';
import WXPlayer from './WXPlayer';
import Logger from 'src/app/logger';

import { ConfigService } from 'src/app/config.service';
import { TipsService } from 'src/app/tips/tips.service';
import { LockerService } from 'src/app/shared/func-service/lock-service.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { BoolEmployee } from 'src/app/shared/func-service/employee.service';
import { LayoutService } from 'src/app/shared/func-service/layout.service';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { DiyHttpService } from 'src/app/shared/func-service/diy-http.service';
import { StreamURLInterface } from 'src/app/preview/StreamURLInterface';

@Component({
  selector: 'app-player',
  templateUrl: './player.component.html',
  styleUrls: ['./player.component.scss']
})
export class PlayerComponent implements OnInit, AfterViewInit, OnDestroy {

  @Input() option: any;

  @ViewChild('canvasvideo', { static: true })
  canvasVideo: ElementRef<HTMLCanvasElement>;
  @ViewChild('canvasdraw', { static: true })
  canvasDraw: ElementRef<HTMLCanvasElement>;
  @ViewChild('canvasbox', { static: true })
  canvasBox: ElementRef<HTMLCanvasElement>;
  @ViewChild('videoEdge', {static: true})
  videoEdgeChild: ElementRef;

  constructor(
    private Re2: Renderer2,
    private el: ElementRef,
    private cfg: ConfigService,
    private tips: TipsService,
    private pfs: PublicFuncService,
    private resCheck: ResponseErrorService,
    private los: LayoutService,
    private ics: IeCssService,
    private dhs: DiyHttpService,
  ) { }

  private saveSignal: Subject<boolean> = new Subject<boolean>();
  private urlSignal: Subject<number> = new Subject<number>();
  private urlOb: any;
  private urlTaskNumber: number = 0;
  private globalStreamId: number = -1;

  private ctOb: any;
  private resolutionOb: any;
  private employee = new BoolEmployee();
  private start_x: number = 0;
  private start_y: number = 0;
  private x: number = 0;
  private y: number = 0;
  private rect: any;
  private ctx: any;
  private aspect: number = 9 / 16;
  private defaultAspect: number = 9 / 16;
  private wxplayer: WXPlayer;
  tipContent: string;
  private imageMode: boolean = false;
  private playerCtx: any;
  private isIe: boolean = false;

  private isMenuEnabled: boolean = false;
  isPlanEnabled: boolean = false;
  private drawerHeightProportion: number;
  private drawerWidthProportion: number;
  private drawArray: DrawCell[];
  private charLength: number = 16;
  private pointEnabled: boolean = false;
  isDrawing: boolean = false;
  private drawingStatus: string = 'none';
  private pointIndex: any;
  private activeIndex: any = 'none';
  private drawEnabled: any;
  private moveEnabled: boolean = true;
  private normalHeight: number;
  private normalWidth: number;
  private motionMode: boolean = false;
  private motionX: number;
  private motionY: number;
  private motionXCount: number;
  private motionYCount: number;
  private isViewInit: boolean = false;
  private canvasDom: any;
  private playerInterval: any;
  private size = {
    height: 0,
    width: 0,
    videoHeight: 0,
    videoWidth: 0,
    videoX: 0,
    videoY: 0,
    videoCropHeight: 0,
    videoCropWidth: 0,
    displayHeight: 0,
    displayWidth: 0,
    displayX: 0,
    displayY: 0,
    reset: () => {
      this.size.displayHeight = 0;
      this.size.displayWidth = 0;
      this.size.displayX = 0;
      this.size.displayY = 0;
      this.size.videoX = 0;
      this.size.videoY = 0;
      this.size.videoCropHeight = this.size.videoHeight;
      this.size.videoCropWidth = this.size.videoWidth;
    },
  };

  logger = new Logger('playerComponent');
  private locker = new LockerService(this.tips, this.logger);
  private ws: any = null;
  isRtmp: boolean = false;
  isHttpFlv: boolean = false;
  isPlaying: boolean = false;
  isMute: boolean = false;
  isEnlarging: boolean = false;
  private displayUrl: string;
  private videoBigPlayShowStatus: boolean = true;

  isRecording: boolean = false;
  recordTitleBank = {
    true: 'stopRecording',
    false: 'startRecording',
  };

  videoPara =  {
    isReady: false,
    height: 0,
    width: 0
  };

  activePoint = [
    {
      x: 0,
      y: 0,
    },
    {
      x: 0,
      y: 0,
    },
    {
      x: 0,
      y: 0,
    },
    {
      x: 0,
      y: 0,
    },
    {
      x: 0,
      y: 0,
    },
    {
      x: 0,
      y: 0,
    },
    {
      x: 0,
      y: 0,
    },
    {
      x: 0,
      y: 0,
    },
  ];

  epBank = [
    {
      name: 'recording',
      task: ['record', 'plan']
    }
  ];

  expendBtn: any = {
    all: false,
    save: false,
    undraw: false,
  };

  playerUrlList: DisplayUrl[] = [
    {
      id: 0,
      name: 'Main',
      url: '',
      tip: 'mainStream',
      resolution: '',
      format: '',
    },
    {
      id: 1,
      name: 'Sub',
      url: '',
      tip: 'subStream',
      resolution: '',
      format: '',
    },
    {
      id: 2,
      name: 'Third',
      url: '',
      tip: 'thirdStream',
      resolution: '',
      format: '',
    },
  ];

  get SaveSignal() {
    return this.saveSignal.asObservable();
  }

  ngOnInit() {
    if (this.option['speName']) {
      this.logger.modify('playerComponent:' + this.option['speName']);
    }

    this.isIe = this.ics.getIEBool();

    if (this.option['imageMode']) {
      this.imageMode = this.option['imageMode'];
    }

    if (this.option.isReshape) {
      this.aspect = 0.75;
      this.resolutionOb = this.los.PlayerResolution.subscribe(
        (resolution: string) => {
          if (resolution) {
            for (let i = 0; i < resolution.length; i++) {
              if (resolution[i] === '*') {
                this.videoPara.height = Number(resolution.slice(i + 1));
                this.videoPara.width = Number(resolution.slice(0, i));
                this.size.videoHeight = this.videoPara.height;
                this.size.videoWidth = this.videoPara.width;
                this.resize4display();
                this.aspect = Number(resolution.slice(i + 1)) / Number(resolution.slice(0, i));
                this.watiViewInit(5000).then(() => {
                  this.resizeCanvas(this.aspect);
                  if (this.drawArray) {
                    this.drawWhenResize();
                  }
                })
                .catch(
                  (error) => {
                    this.logger.log(error, 'ngOnInit:getVideoEncoderInterface:watiViewInit:error');
                  }
                );
                if (this.resolutionOb) {
                  this.resolutionOb.unsubscribe();
                  this.resolutionOb = null;
                }
                break;
              }
            }
          }
        }
      );
    } else {
      this.resolutionOb = this.los.PlayerResolution.subscribe(
        (resolution: string) => {
          if (resolution) {
            for (let i = 0; i < resolution.length; i++) {
              if (resolution[i] === '*') {
                this.size.videoHeight = Number(resolution.slice(i + 1));
                this.size.videoWidth = Number(resolution.slice(0, i));
                this.resize4display();
                if (this.resolutionOb) {
                  this.resolutionOb.unsubscribe();
                  this.resolutionOb = null;
                }
                break;
              }
            }
          }
        }
      );
    }

    this.cfg.getVideoEncoderInterface('').subscribe(
      (res: Array<any>) => {
        for (const item of res) {
          if (item['sResolution'] && item['sStreamType'] && item['sOutputDataType']) {
            if (item['sStreamType'] === "mainStream") {
              this.playerUrlList[0].resolution = item['sResolution'];
              this.playerUrlList[0].format = item['sOutputDataType'];
            } else if (item['sStreamType'] === "subStream") {
              this.playerUrlList[1].resolution = item['sResolution'];
              this.playerUrlList[1].format = item['sOutputDataType'];
            } else if (item['sStreamType'] === "thirdStream") {
              this.playerUrlList[2].resolution = item['sResolution'];
              this.playerUrlList[2].format = item['sOutputDataType'];
            }
          }
        }
        this.urlTaskNumber = this.urlTaskNumber + 1;
        this.urlSignal.next(this.urlTaskNumber);
      },
      err => {
        this.logger.error(err, 'getVideoEncoderInterface:');
      }
    );

    this.cfg.getStreamURLInterface().subscribe(
      (res: StreamURLInterface[]) => {
        this.resCheck.analyseRes(res, 'getVideoUrlFail');
        for (const item of res) {
          if (item.sStreamProtocol === "HTTP") {
            if (item.sURL.match("mainstream")) {
              this.playerUrlList[0].url = item.sURL;
            } else if (item.sURL.match("substream")) {
              this.playerUrlList[1].url = item.sURL;
            } else if (item.sURL.match("thirdstream")) {
              this.playerUrlList[2].url = item.sURL;
            }
          }
        }
        this.logger.log(this.playerUrlList, "get playerUrlList:");
        this.urlTaskNumber = this.urlTaskNumber + 1;
        this.urlSignal.next(this.urlTaskNumber);
        // let streamId = -1;
        // if (this.displayUrl) {
        //   for (const item of this.playerUrlList) {
        //     if (item.url === this.displayUrl) {
        //       streamId = item.id;
        //       break;
        //     }
        //   }
        // }
        // if (streamId >= 0) {
        //   this.setVideoResolution(streamId);
        // }
      },
      err => {
        this.tips.setRbTip('getVideoUrlFail');
      }
    );

    this.expendBtnSwitch();
  }

  ngAfterViewInit() {
    this.isViewInit = true;
    if (this.option.isReshape) {
      this.canvasDraw.nativeElement.addEventListener('mousedown', e => {
        this.rect = this.canvasDraw.nativeElement.getBoundingClientRect();
        this.start_x = e.clientX - this.rect.left;
        this.start_y = e.clientY - this.rect.top;
        if (this.motionMode  && this.isDrawing) {
          this.drawingStatus = 'motion';
          this.motionX = this.start_x;
          this.motionY = this.start_y;
        } else {
          this.pointIndex = this.clickOnWhichPoint(this.start_x, this.start_y);
          if (this.pointIndex !== 'none' && this.pointEnabled) {
            this.drawingStatus = 'point';
          } else {
            this.activeIndex = this.clickOnWhich(this.start_x, this.start_y);
            if (this.activeIndex !== 'none' && this.moveEnabled) {
              this.setActivePoint(this.activeIndex);
              this.drawingStatus = 'move';
            } else {
              if (this.isDrawing) {
                this.drawEnabled = this.checkDrawEnable();
                if (this.drawEnabled !== 'none') {
                  this.drawingStatus = 'draw';
                  this.activeIndex = this.drawEnabled;
                  this.drawArray[this.drawEnabled].x = this.start_x;
                  this.drawArray[this.drawEnabled].y = this.start_y;
                } else {
                  this.activeIndex = 'none';
                }
              } else {
                this.activeIndex = 'none';
              }
            }
          }
        }
        this.drawPic();
      });

      this.canvasDraw.nativeElement.addEventListener('mousemove', (e: any) => {
        if (this.drawingStatus !== 'none') {
          this.x = e.clientX - this.rect.left;
          this.y = e.clientY - this.rect.top;
          const deltaX = this.x - this.start_x;
          const deltaY = this.y - this.start_y;
          this.start_x = this.x;
          this.start_y = this.y;
          if (this.drawingStatus === 'draw') {
            this.drawArray[this.drawEnabled].width += deltaX;
            this.drawArray[this.drawEnabled].height += deltaY;
            this.setActivePoint(this.activeIndex);
          } else if (this.drawingStatus === 'point') {
            this.pointMove(this.pointIndex, deltaX, deltaY);
            this.pointSetRect(this.activeIndex);
          } else if (this.drawingStatus === 'move') {
            this.RectMove(this.activeIndex, deltaX, deltaY);
            this.setActivePoint(this.activeIndex);
          } else if (this.drawingStatus === 'motion') {
            this.motionCheck();
          }
          if (e.clientX < this.rect.left + 1
            || e.clientX > this.rect.right - 5
            || e.clientY < this.rect.top + 1
            || e.clientY > this.rect.bottom - 3) {
              this.drawingStatus = 'none';
            }
          this.drawPic();
        }
      });

      this.canvasDraw.nativeElement.addEventListener('mouseup', e => {
        this.drawingStatus = 'none';
      });
    }

    this.canvasDraw.nativeElement.oncontextmenu = function(e) {
      if (document.all) {
        window.event.returnValue = false;// for IE
      } else {
        e.preventDefault();
      }
    };
    this.resizeCanvas(this.aspect);
    this.ctx = this.canvasDraw.nativeElement.getContext('2d');
  }

  ngOnDestroy() {
    if (this.ctOb) {
      this.ctOb.unsubscribe();
    }
    if (this.resolutionOb) {
      this.resolutionOb.unsubscribe();
      this.resolutionOb = null;
    }
    if (this.playerInterval) {
      clearInterval(this.playerInterval);
      this.playerInterval = null;
    }
    if (this.wxplayer) {
      this.wxplayer.destroy();
    }
  }

  watiViewInit = (timeoutms: number) => new Promise(
    (resolve, reject) => {
      const isViewInitDone = () => {
        timeoutms -= 100;
        if (this.isViewInit) {
          resolve();
        } else if (timeoutms <= 0) {
          reject();
        } else {
          setTimeout(isViewInitDone, 100);
        }
      };
      isViewInitDone();
    }
  )

  motionCheck() {
    const MaxX = Math.max(this.motionX, this.start_x);
    const MaxY = Math.max(this.motionY, this.start_y);
    const MinX = Math.min(this.motionX, this.start_x);
    const MinY = Math.min(this.motionY, this.start_y);
    for (const item of this.drawArray) {
      if (item.x > MinX && item.x < MaxX && item.y > MinY && item.y < MaxY) {
        item.enabled = true;
      } else if (item.x + item.width > MinX && item.x + item.width < MaxX && item.y + item.height > MinY && item.y + item.height < MaxY) {
          item.enabled = true;
      } else if (item.x + item.width > MinX && item.x + item.width < MaxX && item.y > MinY && item.y < MaxY) {
          item.enabled = true;
      } else if (item.x > MinX && item.x < MaxX && item.y + item.height > MinY && item.y + item.height < MaxY) {
          item.enabled = true;
      } else if (item.x < this.start_x && item.y < this.start_y && item.x + item.width > this.start_x
          && item.y + item.height > this.start_y) {
        item.enabled = true;
      }
    }
  }

  initDrawer(width: number, height: number) {
    this.normalHeight = height;
    this.normalWidth = width;
    this.initPorportion();
    this.clearDrawArray();
  }

  initMotionDrawer(xNum: number, yNum: number, mapString: string) {
    this.clearDrawArray();
    this.normalHeight = this.canvasDraw.nativeElement.height;
    this.normalWidth = this.canvasDraw.nativeElement.width;
    this.initPorportion();
    this.motionXCount = Math.ceil(xNum / 4) * 4;
    this.motionYCount = yNum;
    const motionHeight = this.canvasDraw.nativeElement.height / yNum;
    const motionWidth = this.canvasDraw.nativeElement.width / xNum;
    for (let y = 0; y < this.motionYCount; y++) {
      for (let x = 0; x < this.motionXCount; x++) {
        const cell = {
          enabled: false,
          x: x * motionWidth,
          y: y * motionHeight,
          content: '',
          width: motionWidth,
          height: motionHeight,
          shadow: false
        };
        this.drawArray.push(cell);
      }
    }
    let tst = '';
    let progressNumber = 0;
    for (const char of mapString) {
      const mapPart = this.pfs.hex2DecimalOne(char);
      for (const pchar of mapPart) {
        tst += pchar;
        this.drawArray[progressNumber].enabled = Boolean(Number(pchar));
        progressNumber += 1;
      }
    }
  }

  getMotionString() {
    let rst: string = '';
    let patChar: string = '';
    for (let z = 0; z < this.motionYCount; z++) {
      for (let i = 0; i < this.motionXCount / 4; i++) {
        patChar = '';
        for (let j = 0; j < 4; j++) {
          patChar += String(Number(this.drawArray[i * 4 + z * this.motionXCount + j].enabled));
        }
        rst += this.pfs.decimal2Hex4(patChar);
      }
    }
    return rst;
  }

  initPorportion() {
    this.drawerHeightProportion = this.canvasDraw.nativeElement.height / this.normalHeight;
    this.drawerWidthProportion = this.canvasDraw.nativeElement.width / this.normalWidth;
  }

  clearDrawArray() {
    this.drawArray = [];
  }

  pushDrawArray(
    enabledPara: boolean, xPara: number, yPara: number,
    contentPara: string, widthPara: number, heightPara: number,
    shadowEnable: boolean = false) {
    const cell = {
      enabled: enabledPara,
      // x: this.formatX((xPara - widthPara / 2) * this.drawerWidthProportion, widthPara * this.drawerWidthProportion),
      // y: this.formatY((yPara - heightPara / 2) * this.drawerHeightProportion, heightPara * this.drawerHeightProportion),
      x: this.formatX(xPara * this.drawerWidthProportion, widthPara * this.drawerWidthProportion),
      y: this.formatY(yPara * this.drawerHeightProportion, heightPara * this.drawerHeightProportion),
      content: contentPara,
      width: widthPara * this.drawerWidthProportion,
      height: heightPara * this.drawerHeightProportion,
      shadow: shadowEnable
    };
    this.drawArray.push(cell);
  }

  pushCharArray(enabledPara: boolean, xPara: number, yPara: number, contentPara: string, shadowEnable: boolean = false) {
    const len = contentPara.length;
    const widthPara = this.calContentWidth(contentPara);
    const heightPara = this.calContentHeight();
    const cell = {
      enabled: enabledPara,
      x: this.formatX(xPara * this.drawerWidthProportion, widthPara * this.drawerWidthProportion),
      y: this.formatY(yPara * this.drawerHeightProportion, heightPara * this.drawerHeightProportion),
      content: contentPara,
      width: widthPara * this.drawerWidthProportion,
      height: heightPara * this.drawerHeightProportion,
      shadow: shadowEnable
    };
    this.drawArray.push(cell);
  }

  setCharArray(index: number, newContent: string) {
    let newWidth = this.calContentWidth(newContent) * this.drawerWidthProportion;
    newWidth = newWidth > this.canvasDraw.nativeElement.width ? this.canvasDraw.nativeElement.width : newWidth;
    if (this.drawArray[index]) {
      this.drawArray[index].content = newContent;
      this.drawArray[index].width = newWidth;
      this.drawArray[index].x = this.formatX(this.drawArray[index].x, newWidth);
    } else {
      this.logger.error('setCharArray:' + index + ' not exist in drawArray!');
    }
  }

  setFontSize(sizeNum: number) {
    this.charLength = Math.ceil(sizeNum * this.drawerHeightProportion);
    if (this.drawArray && this.drawArray.length > 0) {
      for (const item of this.drawArray) {
        if (item.content.length > 0) {
          item.height = this.calContentHeight();
          item.width = this.calContentWidth(item.content);
        }
      }
    }
  }

  calContentWidth(content: string): number {
    return content.length * (this.charLength);
  }

  get maxContentLenght(): number {
    return Math.ceil(this.canvasDraw.nativeElement.width / this.drawerWidthProportion / (this.charLength));
  }

  calContentHeight() {
    return this.charLength + 6;
  }

  getDrawArray(index: number) {
    if (this.drawArray && this.drawArray[index]) {
      if (this.drawArray[index].width < 0) {
        this.drawArray[index].x += this.drawArray[index].width;
        this.drawArray[index].width *= -1;
      }
      if (this.drawArray[index].height < 0) {
        this.drawArray[index].y += this.drawArray[index].height;
        this.drawArray[index].height *= -1;
      }
      const cell = {
        width: Math.ceil(this.drawArray[index].width / this.drawerWidthProportion),
        height: Math.ceil(this.drawArray[index].height / this.drawerHeightProportion),
        // x: this.drawArray[index].x / this.drawerWidthProportion + this.drawArray[index].width / 2,
        // y: this.drawArray[index].y / this.drawerHeightProportion + this.drawArray[index].height / 2,
        x: Math.ceil(this.drawArray[index].x / this.drawerWidthProportion),
        y: Math.ceil(this.drawArray[index].y / this.drawerHeightProportion),
      };
      return cell;
    } else {
      return null;
    }
  }

  setWidth(index: number, width: number) {
    this.drawArray[index].width = width * this.drawerWidthProportion;
  }

  drawPic() {
    if (this.pointEnabled && this.activeIndex !== 'null') {
      this.setActivePoint(this.activeIndex);
    }
    this.ctx.clearRect(0, 0, this.rect.width, this.rect.height);
    this.ctx.font = (this.charLength).toString() + 'px bold 宋体';
    this.ctx.fillStyle = '#D71920';
    this.ctx.textAlign = 'left';
    this.ctx.textBaseline = 'top';
    this.ctx.strokeStyle = '#D71920';
    this.ctx.lineWidth = 2;
    if (this.motionMode) {
      for (const item of this.drawArray) {
        if (item.enabled) {
          this.ctx.strokeRect(item.x, item.y, item.width, item.height);
        }
      }
    } else {
      if (!this.drawArray) {
        return;
      }
      for (const item of this.drawArray) {
        if (item.enabled && item.width !== 0 && item.height !== 0) {
          this.ctx.strokeRect(item.x, item.y, item.width, item.height);
          if (item.content) {
            this.ctx.fillText(item.content, item.x + 3, item.y + 3);
          }
          if (item.shadow) {
            this.ctx.fillStyle = 'rgba(0, 0, 0,0.5)';
            this.ctx.fillRect(item.x, item.y, item.width, item.height);
            this.ctx.fillStyle = '#D71920';
          }
        }
      }
      if (this.pointEnabled && this.activeIndex !== 'none') {
        for (const item of this.activePoint) {
          this.ctx.strokeRect(item.x, item.y, 5, 5);
          this.ctx.fillRect(item.x, item.y, 5, 5);
        }
      }
    }
  }

  formatX(num: number, width: number) {
    if (num < 0) {
      return 0;
    } else {
      if (num + width > this.canvasDraw.nativeElement.width) {
        return (this.canvasDraw.nativeElement.width - width > 0) ? (this.canvasDraw.nativeElement.width - width) : 0;
      } else {
        return num;
      }
    }
  }

  formatY(num: number, height: number) {
    if (num < 0) {
      return 0;
    } else {
      if (num + height > this.canvasDraw.nativeElement.height) {
        return this.canvasDraw.nativeElement.height - height;
      } else {
        return num;
      }
    }
  }

  clickOnWhich(drawerX: number, drawerY: number) {
    for (const i in this.drawArray) {
      if (this.drawArray[i].width > 0 && this.drawArray[i].height > 0) {
        if (drawerX >= this.drawArray[i].x &&
          drawerX <= this.drawArray[i].x + this.drawArray[i].width &&
          drawerY >= this.drawArray[i].y &&
          drawerY <= this.drawArray[i].y + this.drawArray[i].height) {
            return i;
          }
      }
    }
    return 'none';
  }

  checkDrawEnable() {
    for (const i in this.drawArray) {
      if (this.drawArray[i].width == 0 && this.drawArray[i].height == 0) {
        return i;
      }
    }
    return 'none';
  }

  clickOnWhichPoint(drawerX: number, drawerY: number) {
    for (const i in this.activePoint) {
      if (drawerX >= this.activePoint[i].x - 1 &&
        drawerX <= this.activePoint[i].x + 6 &&
        drawerY >= this.activePoint[i].y - 1 &&
        drawerY <= this.activePoint[i].y + 6) {
          return i;
        }
    }
    return 'none';
  }

  setZero4ActivePoint() {
    for (const item of this.activePoint) {
      item.x = 0;
      item.y = 0;
    }
  }

  setActivePoint(index: any) {
    if (index === 'none') {
      return null;
    } else {
      const loX = this.drawArray[index].x;
      const loY = this.drawArray[index].y;
      const loW = this.drawArray[index].width;
      const loH = this.drawArray[index].height;
      this.activePoint = [
        {
          x: loX,
          y: loY,
        },
        {
          x: loX + loW / 2,
          y: loY,
        },
        {
          x: loX + loW,
          y: loY,
        },
        {
          x: loX + loW,
          y: loY + loH / 2,
        },
        {
          x: loX + loW,
          y: loY + loH,
        },
        {
          x: loX + loW / 2,
          y: loY + loH,
        },
        {
          x: loX,
          y: loY + loH,
        },
        {
          x: loX,
          y: loY + loH / 2,
        },
      ];
      for (let i = 0; i < 8; i++) {
        this.activePoint[i].x -= 2;
        this.activePoint[i].y -= 2;
      }
    }
  }

  pointSetRect(rectIndex: number) {
    this.drawArray[rectIndex].x = this.activePoint[0].x + 2;
    this.drawArray[rectIndex].y = this.activePoint[0].y + 2;
    this.drawArray[rectIndex].width = this.activePoint[2].x - this.activePoint[0].x;
    this.drawArray[rectIndex].height = this.activePoint[6].y - this.activePoint[0].y;
    this.setActivePoint(rectIndex);
  }

  pointMove(index: number, deltaX: number, deltaY: number) {
    if (index == 0 || index == 4) {
      if (index == 0) {
        this.activePoint[0].x = this.noBigThan(this.activePoint[0].x + deltaX, this.activePoint[4].x);
        this.activePoint[0].y = this.noBigThan(this.activePoint[0].y + deltaY, this.activePoint[4].y);
      } else {
        this.activePoint[4].x = this.noSmallThan(this.activePoint[4].x + deltaX, this.activePoint[0].x);
        this.activePoint[4].y = this.noSmallThan(this.activePoint[4].y + deltaY, this.activePoint[0].y);
      }
      this.activePoint[this.cycleIndex(index, 8, -1)].x = this.activePoint[index].x;
      this.activePoint[this.cycleIndex(index, 8, -1)].y += deltaY / 2;
      this.activePoint[this.cycleIndex(index, 8, -2)].x = this.activePoint[index].x;
      this.activePoint[this.cycleIndex(index, 8, 1)].y = this.activePoint[index].y;
      this.activePoint[this.cycleIndex(index, 8, 1)].x += deltaX / 2;
      this.activePoint[this.cycleIndex(index, 8, 2)].y = this.activePoint[index].y;
    } else {
      if (index == 2 || index == 6) {
        if (index == 2) {
          this.activePoint[2].x = this.noSmallThan(this.activePoint[2].x + deltaX, this.activePoint[0].x);
          this.activePoint[2].y = this.noBigThan(this.activePoint[2].y + deltaY, this.activePoint[4].y);
        } else {
          this.activePoint[6].x = this.noBigThan(this.activePoint[6].x + deltaX, this.activePoint[4].x);
          this.activePoint[6].y = this.noSmallThan(this.activePoint[6].y + deltaY, this.activePoint[0].y);
        }
        this.activePoint[this.cycleIndex(index, 8, -1)].y = this.activePoint[index].y;
        this.activePoint[this.cycleIndex(index, 8, -1)].x += deltaX / 2;
        this.activePoint[this.cycleIndex(index, 8, -2)].y = this.activePoint[index].y;
        this.activePoint[this.cycleIndex(index, 8, 1)].x = this.activePoint[index].x;
        this.activePoint[this.cycleIndex(index, 8, 1)].y += deltaY / 2;
        this.activePoint[this.cycleIndex(index, 8, 2)].x = this.activePoint[index].x;
      } else {
        if (index == 1 || index == 5) {
          if (index == 1) {
            this.activePoint[1].y = this.noBigThan(this.activePoint[1].y + deltaY, this.activePoint[4].y);
          } else {
            this.activePoint[5].y = this.noSmallThan(this.activePoint[5].y + deltaY, this.activePoint[0].y);
          }
          this.activePoint[this.cycleIndex(index, 8, -1)].y = this.activePoint[index].y;
          this.activePoint[this.cycleIndex(index, 8, 1)].y = this.activePoint[index].y;
          this.activePoint[this.cycleIndex(index, 8, 2)].y += deltaY / 2;
          this.activePoint[this.cycleIndex(index, 8, -2)].y += deltaY / 2;
        } else {
          if (index == 3 || index == 7) {
            if (index == 3) {
              this.activePoint[3].x = this.noSmallThan(this.activePoint[3].x + deltaX, this.activePoint[0].x);
            } else {
              this.activePoint[7].x = this.noBigThan(this.activePoint[7].x + deltaX, this.activePoint[4].x);
            }
            this.activePoint[this.cycleIndex(index, 8, 1)].x = this.activePoint[index].x;
            this.activePoint[this.cycleIndex(index, 8, -1)].x = this.activePoint[index].x;
            this.activePoint[this.cycleIndex(index, 8, 2)].x += deltaX / 2;
            this.activePoint[this.cycleIndex(index, 8, -2)].x += deltaX / 2;
          }
        }
      }
    }
  }

  cycleIndex(index: number, cycle: number, change: number) {
    let rst: number = Number(index) + Number(change);
    while (rst < 0) {
      rst += cycle;
    }
    while (rst >= cycle) {
      rst -= cycle;
    }
    return rst;
  }

  noBigThan(num: number, target: number) {
    if (num > target) {
      return target;
    } else {
      return num;
    }
  }

  noSmallThan(num: number, target: number) {
    if (num < target) {
      return target;
    } else {
      return num;
    }
  }

  needBetween(num: number, upTarget: number, downTarget: number) {
    if (num > upTarget) {
      return upTarget;
    } else {
      if (num < downTarget) {
        return downTarget;
      } else {
        return num;
      }
    }
  }

  RectMove(index: number, deltaX: number, deltaY: number) {
    this.drawArray[index].x = this.formatX(this.drawArray[index].x + deltaX, this.drawArray[index].width);
    this.drawArray[index].y = this.formatY(this.drawArray[index].y + deltaY, this.drawArray[index].height);
  }

  drawWhenResize() {
    for (const item of this.drawArray) {
      item.x /= this.drawerWidthProportion;
      item.y /= this.drawerHeightProportion;
      item.width /= this.drawerWidthProportion;
      item.height /= this.drawerHeightProportion;
    }
    this.initPorportion();
    for (const item of this.drawArray) {
      item.x *= this.drawerWidthProportion;
      item.y *= this.drawerHeightProportion;
      item.width *= this.drawerWidthProportion;
      item.height *= this.drawerHeightProportion;
    }
    this.setActivePoint(this.activeIndex);
    this.drawPic();
  }

  resizeCanvas(aspect: number) {
    if (this.isMenuEnabled) {
      this.resizeWithMenu(aspect);
    } else {
      this.resizeCanvasFunc(aspect);
    }
  }

  resize4display() {
    if (!this.size.videoHeight || !this.size.videoWidth || !this.size.width || !this.size.height) {
      return;
    }
    this.size.reset();
    if (this.option.isReshape) {
      this.size.displayWidth = this.size.width;
      this.size.displayHeight = this.size.height;
      this.size.displayX = 0;
      this.size.displayY = 0;
      return;
    }
    // TODO: expand for more display mode
    this.set4displaySize();
  }

  set4displaySize() {
    if (this.size.videoCropHeight != this.size.height) {
      this.size.displayWidth = this.size.videoCropWidth / this.size.videoCropHeight * this.size.height;
      this.size.displayHeight = this.size.height;
    } else {
      this.size.displayWidth = this.size.videoCropWidth;
      this.size.displayHeight = this.size.height;
    }
    if (this.size.displayWidth > this.size.width) {
      this.size.displayHeight = this.size.displayHeight / this.size.displayWidth * this.size.width;
      this.size.displayWidth = this.size.width
    }
    this.size.displayX = Math.ceil((this.size.width - this.size.displayWidth) / 2);
    this.size.displayY = Math.ceil((this.size.height - this.size.displayHeight) / 2);
  }

  resizeWithMenu(aspect: number) {
    if (this.videoPara.height === 0 || this.videoPara.width === 0) {
      return;
    }
    const sideBarHeight = this.los.sideBarHeightValue - 40;
    const boxWidth = this.canvasBox.nativeElement.clientWidth;
    let setHeight = this.videoPara.height;
    let setWidth = this.videoPara.width;
    if (setWidth !== boxWidth) {
      setHeight = Math.ceil(setHeight * boxWidth / setWidth);
      setWidth = boxWidth;
    }
    if (setHeight > sideBarHeight) {
      setWidth = Math.ceil(setWidth * sideBarHeight / setHeight);
      setHeight = sideBarHeight;
    }
    this.Re2.setStyle(this.videoEdgeChild.nativeElement, 'width', setWidth + 'px');
    this.Re2.setStyle(this.videoEdgeChild.nativeElement, 'height', setHeight + 'px');
    this.Re2.setStyle(this.el.nativeElement.querySelector('.player-control-menu'), 'width', setWidth + 'px');
    this.Re2.setStyle(this.canvasDraw.nativeElement, 'left', this.videoEdgeChild.nativeElement.offsetLeft + 'px');
    this.Re2.setStyle(this.canvasDraw.nativeElement, 'top', this.videoEdgeChild.nativeElement.offsetTop + 'px');
    this.rect = this.canvasDraw.nativeElement.getBoundingClientRect();
    this.size.height = setHeight;
    this.size.width = setWidth;
    this.resize4display();
  }

  resizeCanvasFunc(aspect: number): void {
    const sideBarHeight = this.los.sideBarHeightValue;
    let boxWidth = this.canvasBox.nativeElement.clientWidth;
    if (boxWidth * aspect > sideBarHeight && !this.isMenuEnabled) {
      boxWidth = Math.round((sideBarHeight * 0.8) / aspect);
    }
    this.Re2.setStyle(this.el.nativeElement.querySelector('.player-control-menu'), 'width', boxWidth + 'px');
    if (this.isRtmp) {
      this.Re2.setStyle(this.el.nativeElement.querySelector('.canvasvideo'), 'display', 'none');
      this.Re2.setStyle(this.el.nativeElement.querySelector('.videoEdge'), 'display', 'block');
      this.videoEdgeChild.nativeElement.width = boxWidth;
      this.videoEdgeChild.nativeElement.height = boxWidth * aspect;
      // this.videoChild.nativeElement.width = boxWidth;
      // this.videoChild.nativeElement.height = boxWidth * aspect;
      const adjustList = ['.videoEdge', '.big-play-btn-area'];
      for (const className of adjustList) {
        const adjustElement = this.el.nativeElement.querySelector(className);
        this.Re2.setStyle(adjustElement, 'width', boxWidth + 'px');
        this.Re2.setStyle(adjustElement, 'height', boxWidth * aspect + 'px');
      }
    } else {
      this.Re2.setStyle(this.el.nativeElement.querySelector('.canvasvideo'), 'display', 'block');
      this.Re2.setStyle(this.el.nativeElement.querySelector('.videoEdge'), 'display', 'none');
      this.canvasVideo.nativeElement.width = boxWidth;
      this.canvasVideo.nativeElement.height = boxWidth * aspect;
      const btnElement = this.el.nativeElement.querySelector('.big-play-btn-area');
      if (btnElement) {
        this.Re2.setStyle(btnElement, 'width', boxWidth + 'px');
        this.Re2.setStyle(btnElement, 'height', boxWidth * aspect + 'px');
      }
    }
    this.canvasDraw.nativeElement.width = boxWidth;
    this.canvasDraw.nativeElement.height = boxWidth * aspect;
    if (this.isRtmp) {
      this.Re2.setStyle(this.canvasDraw.nativeElement, 'left', this.videoEdgeChild.nativeElement.offsetLeft + 'px');
      this.Re2.setStyle(this.canvasDraw.nativeElement, 'top', this.videoEdgeChild.nativeElement.offsetTop + 'px');
    } else {
      this.Re2.setStyle(this.canvasDraw.nativeElement, 'left', this.canvasVideo.nativeElement.offsetLeft + 'px');
      this.Re2.setStyle(this.canvasDraw.nativeElement, 'top', this.canvasVideo.nativeElement.offsetTop + 'px');
    }
    this.rect = this.canvasDraw.nativeElement.getBoundingClientRect();
    this.size.height = boxWidth * aspect;
    this.size.width = boxWidth;
    this.resize4display();
  }

  reshapeCanvas() {
    this.resizeCanvas(this.aspect);
    this.ctx = this.canvasDraw.nativeElement.getContext('2d');
  }

  @HostListener('window:resize', ['$event'])
  onResize(event): void {
    if (this.isViewInit) {
      this.resizeCanvas(this.aspect);
      if (this.drawerHeightProportion) {
        this.drawWhenResize();
      }
      this.draw4ImageMode();
    }
  }

  setVideoResolution = (streamId: number) => {
    if (this.playerUrlList[streamId].resolution) {
      for (let i = 0; i < this.playerUrlList[streamId].resolution.length; i++) {
        if (this.playerUrlList[streamId].resolution[i] === '*') {
          const oHeight = Number(this.playerUrlList[streamId].resolution.slice(i + 1));;
          const oWidth = Number(this.playerUrlList[streamId].resolution.slice(0, i));
          if (this.size.videoHeight !== oHeight || this.size.videoWidth !== oWidth) {
            this.size.videoHeight = oHeight;
            this.size.videoWidth = oWidth;
            this.resize4display();
          }
          break;
        }
      }
    }
    this.cfg.getVideoEncoderInterface(this.playerUrlList[streamId].tip).subscribe(
      res => {
        const resolution = res['sResolution'];
        if (resolution) {
          this.playerUrlList[streamId].resolution = resolution;
          this.playerUrlList[streamId].format = res['sOutputDataType'];
          for (let i = 0; i < resolution.length; i++) {
            if (resolution[i] === '*') {
              const rHeight = Number(resolution.slice(i + 1));;
              const rWidth = Number(resolution.slice(0, i));
              if (this.size.videoHeight !== rHeight || this.size.videoWidth !== rWidth) {
                this.size.videoHeight = rHeight;
                this.size.videoWidth = rWidth;
                this.resize4display();
              }
              break;
            }
          }
        } else {
          this.logger.error(resolution, 'getVideoEncoderInterface:noneResolution:');
        }
      },
      err => {
        this.logger.error(err, 'getVideoEncoderInterface:');
      }
    );
  }

  setPlayTimeout = () => {
    if (this.urlTaskNumber !== 2) {
      this.tips.setRbTip('getVideoUrlFail');
      this.logger.error("setPlayTimeout:getVideoUrlFail");
    }
  };

  setPlay(url: string, streamId: number = -1) {
    this.globalStreamId = streamId;
    if (this.urlOb) {
      return;
    }
    if (this.urlTaskNumber === 2) {
      this.setPlayFunc(url, streamId);
    } else {
      this.urlOb = this.urlSignal.asObservable().subscribe(
        (change: number) => {
          if (change === 2) {
            this.setPlayFunc(url, this.globalStreamId);
            if (this.urlOb) {
              this.urlOb.unsubscribe();
              this.urlOb = null;
            }
          }
        }
      )
      setTimeout(this.setPlayTimeout, 5000);
    }
  }

  setPlayFunc(url: string, streamId: number) {
    this.logger.log('setPlay url is ' + url, 'stream id is ' + streamId);
    if (streamId === -1) {
      for (const item of this.playerUrlList) {
        if (item.url === url) {
          streamId = item.id;
          break;
        }
      }
    }
    if (streamId >= 0) {
      this.setVideoResolution(streamId);
    }
    this.logger.log('streamId is ' + streamId);
    if (url.startsWith('HTTP') || url.startsWith('http')) {
      this.isRtmp = false;
      this.isHttpFlv = true;
      this.resizeCanvasFunc(this.aspect);
      this.logger.log('play format is ' + this.playerUrlList[streamId].format);
      if (this.playerUrlList[streamId].format == 'H.265') {
        this.httpFlvPlay(url, true);
      } else {
        this.httpFlvPlay(url, false);
      }
    } else {
      this.tips.setRbTip('illegalStreamUrl');
      this.logger.error(url, 'setPlay:illegalStreamUrl:');
    }
  }

  httpFlvPlay(palyUrl: string, isH265: boolean = false) {
    if (!this.wxplayer) {
      this.wxplayer = new WXPlayer();
    } else {
      this.wxplayer.stop();
    }
    if (!this.canvasDom) {
      this.canvasDom = document.getElementById('canvas-really');
    }
    const options: any = {
      asmUrl: "./assets/wxplayer/prod." + (isH265 ? "h265" : "all") + ".asm.combine.js",
      wasmUrl: "./assets/wxplayer/prod." + (isH265 ? "h265" : "all") + ".wasm.combine.js",
      url: palyUrl,
      $container: this.canvasDom,
      hasVideo: true,
      hasAudio: false,
      volume: 1.0,
      muted: false,
      autoplay: true,
      loop: false,
      isLive: true,
      chunkSize: 64 * 1024, // 加载/解析块大小
      preloadTime: 1000, // 预加载时间，与延时无关
      bufferingTime: 100, // 缓冲时间，与延时有关
      cacheSegmentCount: 256, // 内部最大的缓存帧数量，默认256，太小会一直在playing和buffering中切换，导致卡顿
      customLoader: null,
    };
    let sub = this.wxplayer.initPlayer(options, this.httpFlvIntervalFunc);
    sub.subscribe(
      (change) => {
        this.isPlaying = true;
        this.httpFlvIntervalFunc();
      }
    );
  }

  httpFlvIntervalFunc = () => {
    if (!this.playerCtx) {
      this.playerCtx = this.canvasVideo.nativeElement.getContext('2d');
      this.playerCtx.clearRect(0, 0, this.size.width, this.size.height);
    }
    this.resize4display();
    if (this.playerInterval) {
      clearInterval(this.playerInterval);
      this.playerInterval = null;
    }
    this.playerInterval = setInterval(() => {
      this.playerCtx.clearRect(0, 0, this.size.width, this.size.height);
      this.playerCtx.drawImage(this.canvasDom, this.size.videoX, this.size.videoY, this.size.videoCropWidth, this.size.videoCropHeight,
        this.size.displayX, this.size.displayY, this.size.displayWidth, this.size.displayHeight);
    }, 12);
    if (this.locker.isLock('switchStream')) {
      this.locker.releaseLocker('switchStream');
    }
  }

  httpFlvUnsupportIntervalFunc = () => {
    if (!this.playerCtx) {
      this.playerCtx = this.canvasVideo.nativeElement.getContext('2d');
      this.playerCtx.clearRect(0, 0, this.size.width, this.size.height);
    }
    this.playerCtx.fillStyle = '#f8f8f8';
    this.playerCtx.textAlign = 'center';
    this.playerCtx.font = (this.size.width / 10).toString() + 'px bold 宋体';
    this.resize4display();
    if (this.playerInterval) {
      clearInterval(this.playerInterval);
      this.playerInterval = null;
    }
    this.playerInterval = setInterval(() => {
      this.playerCtx.clearRect(0, 0, this.size.width, this.size.height);
      this.playerCtx.fillText('Unsupport H265', this.size.width / 2, this.size.height / 2);
    });
    if (this.locker.isLock('switchStream')) {
      this.locker.releaseLocker('switchStream');
    }
  }

  onAdd(): void {
    this.isDrawing = !this.isDrawing;
  }

  onDel() {
    this.tips.showCTip('isDeleteAllMask');
    this.ctOb = this.tips.ctAction.subscribe(
      change => {
        if (change === 'onYes') {
          this.deleteAllMask();
          this.tips.setCTPara('close');
        } else if (change === 'onNo') {
          this.ctOb.unsubscribe();
          this.ctOb = null;
        }
      }
    );
  }

  deleteAllMask() {
    if (this.motionMode) {
      for (const item of this.drawArray) {
        item.enabled = false;
      }
    } else {
      for (const item of this.drawArray) {
        item.x = 0;
        item.y = 0;
        item.width = 0;
        item.height = 0;
      }
      this.setZero4ActivePoint();
      this.activeIndex = 'none';
    }
    this.drawPic();
  }

  changeSoundStatus() {
    this.isMute = !this.isMute;
  }

  usingDIYMenu(judge: boolean = true) {
    if (judge) {
      const diyMenu = this.el.nativeElement.querySelector('.player-control-menu');
      if (diyMenu) {
        this.isMenuEnabled = true;
        this.Re2.setStyle(diyMenu, 'display', 'block');
      }
    } else {
      const vjMenu = this.el.nativeElement.querySelector('.vjs-control-bar');
      if (vjMenu) {
        this.Re2.setStyle(vjMenu, 'display', 'none');
      }
    }
  }

  diyPlay() {
    this.isPlaying = true;
    if (this.wxplayer) {
      this.wxplayer.play();
    }
  }

  // reserved
  diyPause() {
    return;
    this.isPlaying = false;
    if (this.wxplayer) {
      this.wxplayer.pause();
    }
  }

  diyStop() {
    this.isPlaying = false;
    if (this.wxplayer) {
      this.wxplayer.stop();
    }
  }

  bigBtnPlay() {
    if (this.isPlaying) {
      return;
    }
    this.logger.log('bigBtnPlay');
    this.hideBigPlayBtn();
    if (this.imageMode) {
      this.isRtmp = false;
      this.resizeCanvas(this.aspect);
      this.draw4ImageMode();
    } else if (this.displayUrl) {
      this.setPlay(this.displayUrl);
    } else if (this.playerUrlList[1].url !== '') {
      this.displayUrl = this.playerUrlList[1].url;
      this.setPlay(this.displayUrl);
    }
    // necessary for all
    if (this.drawArray) {
      this.drawPic();
    }
  }

  draw4ImageMode() {
    if (this.imageMode && this.displayUrl) {
      this.playerCtx = this.canvasVideo.nativeElement.getContext('2d');
      const img = new Image();
      img.src = this.displayUrl;
      const that = this;
      img.onload = () => {
        that.playerCtx.drawImage(
          img, 0, 0,
          that.canvasVideo.nativeElement.clientWidth, that.canvasVideo.nativeElement.clientHeight
          );
      };
    }
  }

  enlargeCtxClear() {
    if (!this.ctx) {
      this.ctx = this.canvasDraw.nativeElement.getContext('2d');
    }
    this.ctx.clearRect(0, 0, this.rect.width, this.rect.height);
  }

  onEnlarge() {
    this.enlargeCtxClear();
    this.isEnlarging = !this.isEnlarging;
  }

  draw4Enlarge() {
    this.enlargeCtxClear();
    this.ctx.fillStyle = '#D71920';
    this.ctx.strokeStyle = '#D71920';
    this.ctx.lineWidth = 2;
    this.ctx.strokeRect(this.start_x, this.start_y, this.x - this.start_x, this.y - this.start_y);
  }

  enlargeFunc() {
    this.enlargeCtxClear();
    this.start_x = this.start_x < this.size.displayX ? this.size.displayX : this.start_x;
    this.start_y = this.start_y < this.size.displayY ? this.size.displayY : this.start_y;
    this.x = this.x < this.size.displayX ? this.size.displayX : this.x;
    this.y = this.y < this.size.displayY ? this.size.displayY : this.y;
    const rightEdge = this.size.displayX + this.size.displayWidth;
    const bottomEdge = this.size.displayY + this.size.displayHeight;
    this.start_x = this.start_x > rightEdge ? rightEdge : this.start_x;
    this.start_y = this.start_y > bottomEdge ? bottomEdge : this.start_y;
    this.x = this.x > rightEdge ? rightEdge : this.x;
    this.y = this.y > bottomEdge ? bottomEdge : this.y;
    if (this.start_x !== this.x && this.start_y !== this.y) {
      let e_x = 0;
      let e_y = 0;
      let e_w = 0;
      let e_h = 0;
      if (this.start_x > this.x) {
        e_x = this.x;
        e_w = this.start_x - this.x;
      } else {
        e_x = this.start_x;
        e_w = this.x - this.start_x;
      }
      if (this.start_y > this.y) {
        e_y = this.y;
        e_h = this.start_y - this.y;
      } else {
        e_y = this.start_y;
        e_h = this.y - this.start_y;
      }
      this.size.videoX += ((e_x - this.size.displayX) * this.size.videoCropWidth / this.size.displayWidth);
      this.size.videoY += ((e_y - this.size.displayY) * this.size.videoCropHeight / this.size.displayHeight);
      this.size.videoCropWidth = e_w * this.size.videoCropWidth / this.size.displayWidth;
      this.size.videoCropHeight = e_h * this.size.videoCropHeight / this.size.displayHeight;
      this.set4displaySize();
    }
    this.start_x = 0;
    this.start_y = 0;
    this.x = 0;
    this.y = 0;
  }

  set4Preview() {
    this.usingDIYMenu(true);
    const playerButton = this.el.nativeElement.querySelectorAll('.blue-btn');
    for (const btn of playerButton) {
      this.Re2.setAttribute(btn, 'hidden', 'true');
    }
    this.employee.hire(this.epBank[0]);
    this.cfg.getRecordStatus().subscribe(
      res => {
        this.resCheck.analyseRes(res, 'getRecordStatusFail');
        if (Number(res) < 0) {
          this.tips.setRbTip('recordStatusError');
          this.isRecording = false;
        } else {
          this.isRecording = Boolean(Number(res));
        }
        this.employee.numTask(this.epBank, 0, 0, 1, 0);
      },
      err => {
        this.logger.error(err, 'set4Preview:getRecordStatus:');
        this.employee.numTask(this.epBank, 0, 0, 0, 0);
      }
    );
    this.cfg.getPlanAdvancePara(0).subscribe(
      res => {
        this.isPlanEnabled = Boolean(res['iEnabled']);
        this.employee.numTask(this.epBank, 0, 1, 1, 0);
      },
      err => {
        this.logger.error(err, 'set4Preview:getPlanAdvancePara(0):');
        this.employee.numTask(this.epBank, 0, 1, 0, 0);
      }
    );
    this.employee.observeTask(this.epBank[0].name, 5000)
      .then(
        () => {
          this.checkPlanEnabled();
        }
      )
      .catch(
        () => {
          this.logger.error('set4Preview:observeTask');
          this.tips.showInitFail();
        }
    );
    this.resizeCanvas(this.defaultAspect);
    this.watiViewInit(5000).then(() => {
      this.canvasDraw.nativeElement.addEventListener('mousedown', e => {
        if (this.isEnlarging && e.button != 2) {
          this.rect = this.canvasDraw.nativeElement.getBoundingClientRect();
          this.start_x = e.clientX - this.rect.left;
          this.start_y = e.clientY - this.rect.top;
        }
      });
      this.canvasDraw.nativeElement.addEventListener('mousemove', (e: any) => {
        if (this.isEnlarging && this.start_x !== 0 && this.start_y !== 0) {
          this.x = e.clientX - this.rect.left;
          this.y = e.clientY - this.rect.top;
          if (e.clientX < this.rect.left + 1
            || e.clientX > this.rect.right - 5
            || e.clientY < this.rect.top + 1
            || e.clientY > this.rect.bottom - 3) {
              this.enlargeFunc();
          } else {
            this.draw4Enlarge();
          }
        }
      });
      this.canvasDraw.nativeElement.addEventListener('mouseup', e => {
        if (this.isEnlarging) {
          if (e.button == 2) {
            this.resize4display();
          } else {
            this.enlargeFunc();
          }
        }
      });
    })
    .catch(
      (error) => {
        this.logger.log(error, 'set4Preview:watiViewInit:error');
      }
    );
  }

  hideDrawer() {
    const drawer = this.el.nativeElement.querySelector('.canvasdraw');
    if (drawer) {
      this.Re2.setStyle(drawer, 'display', 'none');
    }
  }

  postSnapSignal() {
    if (!this.displayUrl.match("mainstream")) {
      this.tips.setRbTip('onlySupportMain');
      return;
    }
    this.cfg.postTakePhotoSignal().subscribe(
      res => {
        this.tips.setRbTip('snapSuccess');
      },
      err => {
        this.tips.setRbTip('snapFail');
      }
    );
  }

  hideBigPlayBtn() {
    this.Re2.setStyle(this.el.nativeElement.querySelector('.big-play-btn-area'), 'display', 'none');
  }

  // reserved
  destroyWhenSwitch() {
    return;
  }

  alertRecordingStatus() {
    if (!this.displayUrl.match("mainstream")) {
      this.tips.setRbTip('onlySupportMain');
      return;
    }
    if (this.locker.getLocker('alertRecordingStatus', '')) {
      return;
    }
    this.isRecording = !this.isRecording;
    this.cfg.sendRecordSignal(this.isRecording).subscribe(
      res => {
        this.tips.setRbTip(this.getRecordingTitle(!this.isRecording));
        this.locker.releaseLocker('alertRecordingStatus');
        this.checkPlanEnabled();
      },
      err => {
        this.logger.error(err, 'alertRecordingStatus:');
        this.tips.setRbTip('recordingFail');
        this.isRecording = !this.isRecording;
        this.locker.releaseLocker('alertRecordingStatus');
      }
    );
  }

  checkPlanEnabled() {
    if (this.isPlanEnabled && this.isRecording) {
      this.tips.setRbTip('videoPlanEnabledRecordUseless');
    }
  }

  getRecordingTitle(key: boolean) {
    return this.recordTitleBank[key.toString()];
  }

  reshapeByResolution(height: number, width: number) {
    this.aspect = height / width;
    this.resizeCanvas(this.aspect);
  }

  expendBtnSwitch() {
    if (this.option['expendBtn']) {
      if (this.option['expendBtn'] instanceof Array) {
        for (const item of this.option['expendBtn']) {
          this.expendBtn[item.toString()] = true;
        }
      }
    }
  }

  onSave() {
    this.saveSignal.next(false);
  }

  drawWholeMask() {
    if (this.drawArray.length >= 1) {
      this.drawArray[0].x = 0;
      this.drawArray[0].y = 0;
      this.drawArray[0].width = this.canvasDraw.nativeElement.width;
      this.drawArray[0].height = this.canvasDraw.nativeElement.height;
      this.drawPic();
    }
  }

  isSelectedStream(urlString: string) {
    return this.displayUrl === urlString;
  }

  switchStream(urlString: string, id: number) {
    if (this.displayUrl === urlString) {
      return;
    }
    if (this.locker.getLocker('switchStream', 'switchingStream', true)) {
      return;
    }
    this.displayUrl = urlString;
    if (this.playerInterval) {
      clearInterval(this.playerInterval);
      this.playerInterval = null;
    }
    if (this.isPlaying && this.wxplayer) {
      this.wxplayer.stop();
      if (!this.canvasDom) {
        this.canvasDom = document.getElementById('canvas-really');
      }
      // clear canvas
      this.canvasDom.height = this.canvasDom.height - 1;
    }
    this.setPlay(this.displayUrl, id);
  }

  screenshotFunc() {
    if (!this.canvasDom) {
      this.logger.error("no this.canvasDom!");
    }
    const dataURL = this.canvasDom.toDataURL('image/jpeg', 1.0);
    let fileName = '';
    for (const item of this.playerUrlList) {
      if (this.displayUrl == item.url) {
        fileName = item.tip + (new Date()).getTime() + '.jpeg';
        break;
      }
    }
    if (fileName === '') {
      fileName = 'screenshot.jpeg';
    }
    if (this.isIe) {
      this.dhs.downloadPic4IEFunc(fileName, dataURL);
    } else {
      this.dhs.aDownloadFunc(fileName, dataURL);
    }
  }
}

export interface DrawCell {
  enabled: boolean;
  x: number;
  y: number;
  content: string;
  width: number;
  height: number;
  shadow: boolean;
}

export interface DisplayUrl {
  id: number;
  name: string;
  url: string;
  tip: string;
  resolution: string;
  format: string;
}
