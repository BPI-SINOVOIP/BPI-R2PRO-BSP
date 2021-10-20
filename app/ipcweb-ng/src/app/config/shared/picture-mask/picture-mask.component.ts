import { Component, OnInit, ViewChild, Input, ElementRef, AfterViewInit, OnDestroy, Renderer2 } from '@angular/core';
import { FormBuilder, FormGroup, FormArray } from '@angular/forms';

import { ConfigService } from 'src/app/config.service';
import Logger from 'src/app/logger';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { TipsService } from 'src/app/tips/tips.service';
import { LockService } from 'src/app/shared/func-service/lock-service.service';

@Component({
  selector: 'app-picture-mask',
  templateUrl: './picture-mask.component.html',
  styleUrls: ['./picture-mask.component.scss']
})
export class PictureMaskComponent implements OnInit, AfterViewInit, OnDestroy {

  @Input() option: any;
  @ViewChild('player', {static: true}) playerChild: any;
  @ViewChild('player', {read: ElementRef}) playerDom: ElementRef;
  @ViewChild('input', {static: true}) inputChild: any;

  constructor(
    private fb: FormBuilder,
    private cfgService: ConfigService,
    private Re2: Renderer2,
    private el: ElementRef,
    private ieCss: IeCssService,
    private resError: ResponseErrorService,
    private pfs: PublicFuncService,
    private tips: TipsService,
  ) { }

  logger = new Logger('pictureMask');
  isChrome: boolean = false;
  // private lock = new LockService(this.tips);
  isIe: boolean = true;

  private iNormalizedScreenHeight: number;
  private iNormalizedScreenWidth: number;
  private viewInit: boolean = false;
  private isDrawerInit: boolean = false;
  private limitSize: number = 1024 * 1024;
  fileLimitSize = '';
  private fileTypeLimit: Array<string> = ['bmp', 'BMP'];
  typeLimit: string = '';

  private uploadFile: any;
  fileName: string = 'noFileSelected';

  playerOption = {
    isReshape: true,
    speName: 'picture-mask',
  };

  PicForm = this.fb.group({
    imageOverlay: this.fb.group({
      iImageHeight: [''],
      iImageWidth: [''],
      iPositionX: [''],
      iPositionY: [''],
      iImageOverlayEnabled: [''],
      iTransparentColorEnabled: [''],
    }),
    normalizedScreenSize: this.fb.group({
      iNormalizedScreenHeight: [''],
      iNormalizedScreenWidth: [''],
    }),
  });

  get imageOverlay(): FormGroup {
    return this.PicForm.get('imageOverlay') as FormGroup;
  }

  get normalizedScreenSize(): FormGroup {
    return this.PicForm.get('normalizedScreenHeight') as FormGroup;
  }

  timer: any = null;

  ngOnInit(): void {
    this.getUrl(5000)
      .then(
        () => {
          this.wait2Init(5000)
            .then(
              () => {
                this.playEntry(this.option['src']);
              }
            )
            .catch(
              () => {
                this.tips.setRbTip('initPlayerFailFreshPlease');
              }
            );
        }
      )
      .catch(
        () => {
          this.tips.setRbTip('getVideoUrlFail');
        }
      );
    this.isIe = this.ieCss.getIEBool();
    if (this.isIe) {
      this.fileName = '';
    }
    this.isChrome = this.ieCss.getChromeBool();
    this.typeLimit = this.fileTypeLimit.join(',');
    this.dataInit();
    this.timer = setInterval(() => {
      if (this.isDrawerInit) {
        const cell = this.playerChild.getDrawArray(0);
        if (cell) {
          this.PicForm.get('imageOverlay.iPositionX').setValue(Math.ceil(cell.x));
          this.PicForm.get('imageOverlay.iPositionY').setValue(Math.ceil(cell.y));
        }
      }
    }, 100);
  }

  ngAfterViewInit(): void {
    this.playerChild.hideBigPlayBtn();
    const playerButton = this.playerDom.nativeElement.querySelectorAll('.blue-btn');
    for (const btn of playerButton) {
      this.Re2.setAttribute(btn, 'hidden', 'true');
    }
    this.viewInit = true;
    this.initDrawArray();

  }

  ngOnDestroy() {
    if (this.timer) {
      clearInterval(this.timer);
      this.timer = null;
    }
    this.stopPlayer();
  }

  initDrawArray() {
    if (this.iNormalizedScreenHeight && this.iNormalizedScreenWidth) {
      this.playerChild.pointEnabled = false;
      this.playerChild.isDrawing = false;
      this.playerChild.initDrawer(this.iNormalizedScreenWidth, this.iNormalizedScreenHeight);
      this.playerChild.pushDrawArray(
        true, this.PicForm.get('imageOverlay').value.iPositionX, this.PicForm.get('imageOverlay').value.iPositionY,
        '', this.PicForm.get('imageOverlay').value.iImageWidth,
        this.PicForm.get('imageOverlay').value.iImageHeight, true
      );
      this.playerChild.drawPic();
    }
    this.isDrawerInit = true;
  }

  onFileChange(file: File[]) {
    if (file[0].size >= this.limitSize) {
      this.fileLimitSize = this.transformLimitSize();
      this.fileName = 'fileSizeShouldBeSmallerThan';
      this.setRedFileTip();
    } else if (!this.checkFileType(file[0].name)) {
      this.fileLimitSize = this.typeLimit;
      this.fileName = 'fileTypeShouldBe';
      this.setRedFileTip();
    } else {
      const ob = this.pfs.checkBmp24(file[0]).subscribe(
        e => {
          if (e !== 'ok') {
            this.fileLimitSize = '';
            this.fileName = e;
            this.setRedFileTip();
            ob.unsubscribe();
          } else {
            this.uploadFile = file[0];
            if (!this.isIe) {
              this.fileName = file[0].name;
              this.setBlackFileTip();
            } else {
              this.fileName = '';
            }
            this.fileLimitSize = '';
            ob.unsubscribe();
          }
        }
      );
    }
  }

  onSave() {
    const cell = this.playerChild.getDrawArray(0);
    if (!cell) {
      this.tips.showSaveFail();
      return;
    }
    this.PicForm.value.imageOverlay.iPositionX = cell.x;
    this.PicForm.value.imageOverlay.iPositionY = cell.y;
    this.pfs.formatInt(this.PicForm.value);
    this.cfgService.setPictureMaskInterface(this.PicForm.value).subscribe(
      res => {
        this.resError.analyseRes(res);
        this.PicForm.patchValue(res);
        this.tips.showSaveSuccess();
        // this.initDrawArray();
      },
      err => {
        this.logger.error(err, 'onSave:setPictureMaskInterface:');
        this.tips.showSaveFail();
      }
    );
  }

  changeIntoBoolean() {
    this.PicForm.value.imageOverlay.iImageOverlayEnabled = Number(this.PicForm.value.imageOverlay.iImageOverlayEnabled);
  }

  dataInit() {
    this.cfgService.getPictureMaskInterface().subscribe(
      res => {
        this.resError.analyseRes(res);
        this.PicForm.patchValue(res);
        this.iNormalizedScreenWidth = res.normalizedScreenSize.iNormalizedScreenWidth;
        this.iNormalizedScreenHeight = res.normalizedScreenSize.iNormalizedScreenHeight;
        if (this.viewInit) {
          this.initDrawArray();
        }
      },
      err => {
        this.tips.setRbTip('getParaFailFreshPlease');
      }
    );
  }

  onUpdate() {
    if (!this.uploadFile) {
      this.tips.setRbTip('selectFileFirst');
      return;
    }
    this.cfgService.postImage(this.uploadFile).subscribe(
      res => {
        this.resError.analyseRes(res);
        this.dataInit();
        this.initDrawArray();
      },
      err => {
        this.logger.error(err, 'onUpdate:postImage:');
      }
    );
  }

  setPicCanvas() {
    this.playerChild.reshapeCanvas();
    this.initDrawArray();
    this.isDrawerInit = true;
  }

  onFileClick(id: string) {
    const event = new MouseEvent('click');
    const fileInputBtn = document.getElementById(id);
    fileInputBtn.dispatchEvent(event);
  }

  transformLimitSize() {
    if (this.limitSize < 1024) {
      return this.limitSize + 'KB';
    } else {
      const kb = this.limitSize / 1024;
      const mb = Math.ceil(kb / 1024 * 100) / 100;
      return mb + 'MB';
    }
  }

  checkFileType(name: string) {
    let type = '';
    for (let i = name.length - 1; i >= 0; i--) {
      if (name[i] === '.') {
        type = name.slice(i + 1);
      }
    }
    for (const item of this.fileTypeLimit) {
      if (item === type) {
        return true;
      }
    }
    return false;
  }

  playEntry(src: string) {
    if (this.viewInit && src) {
      this.playerChild.displayUrl = src;
      this.playerChild.bigBtnPlay();
    }
  }

  pausePlayer() {
    if (this.viewInit && this.playerChild) {
    this.playerChild.diyPause();
    this.playerChild.destroyWhenSwitch();
    }
  }

  stopPlayer() {
    if (this.viewInit && this.playerChild && this.playerChild.isPlaying) {
      this.playerChild.diyStop();
    }
  }

  getUrl = (timeoutms: number) => new Promise(
    (resolve, reject) => {
      const waitFunc = () => {
        timeoutms -= 100;
        if (this.option['src']) {
          resolve();
        } else if (timeoutms <= 0) {
          reject();
        } else {
          setTimeout(waitFunc, 100);
        }
      };
      waitFunc();
    }
  )

  wait2Init = (timeoutms: number) => new Promise(
    (resolve, reject) => {
      const waitFunc = () => {
        timeoutms -= 100;
        if (this.viewInit) {
          resolve();
        } else if (timeoutms <= 0) {
          reject();
        } else {
          setTimeout(waitFunc, 100);
        }
      };
      waitFunc();
    }
  )

  setRedFileTip() {
    this.Re2.setStyle(this.el.nativeElement.querySelector('.input-file'), 'color', 'red');
  }
  setBlackFileTip() {
    this.Re2.setStyle(this.el.nativeElement.querySelector('.input-file'), 'color', 'black');
  }
}
