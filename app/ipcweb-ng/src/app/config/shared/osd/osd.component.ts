import { Component, OnInit, ViewChild, ElementRef, Input, AfterViewInit, OnDestroy, Renderer2 } from '@angular/core';
import { FormBuilder, FormGroup, FormArray } from '@angular/forms';

import { ConfigService } from 'src/app/config.service';
import { OsdOverplaysInterface } from './OsdOverplaysInterface';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { TipsService } from 'src/app/tips/tips.service';
import { LockService } from 'src/app/shared/func-service/lock-service.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-osd',
  templateUrl: './osd.component.html',
  styleUrls: ['./osd.component.scss']
})

// disabled char overlay in html
export class OsdComponent implements OnInit, AfterViewInit, OnDestroy {

  @Input() option: any;
  @ViewChild('player', {static: true}) playerChild: any;
  @ViewChild('player', {read: ElementRef}) playerDom: ElementRef;

  constructor(
    private fb: FormBuilder,
    private cfgService: ConfigService,
    private Re2: Renderer2,
    private el: ElementRef,
    private ieCss: IeCssService,
    private resError: ResponseErrorService,
    private tips: TipsService,
    private pfs: PublicFuncService,
  ) { }

  private logger: Logger = new Logger('osd');
  // private lock = new LockService(this.tips);
  isChrome: boolean = false;
  isIe: boolean = true;

  playerOption = {
    isReshape: true,
    speName: 'osd',
  };

  osdAttributes: Array<string> = [
    'opaque/not-flashing',
  ];

  osdAttributesBank: Array<string> = [
    'transparent/flashing',
    'opaque/flashing',
    'transparent/not-flashing',
    'opaque/not-flashing',
  ];

  osdFonts: Array<string> = [
    '16*16',
    '32*32',
    '48*48',
    '64*64',
    // 'adaptive',
  ];

  osdColors: Array<string> = [
    'auto',
    'customize',
  ];

  osdAligments: Array<string> = [
    'customize',
    'alignRight',
    'alignLeft',
    'GB',
  ];

  minEdgeSizes: Array<string> = [
    'no characters',
    'one character',
    'two characters',
  ];

  timeFormats: Array<string> = [
    '12hour',
    '24hour',
  ];

  dateFormats: Array<string> = [
    'YYYY-MM-DD',
    'MM-DD-YYYY',
    'DD-MM-YYYY',
    'CHR-YYYY-MM-DD',
    'CHR-MM-DD-YYYY',
    'CHR-DD-MM-YYYY',
    'CHR-YYYY/MM/DD',
    'CHR-MM/DD/YYYY',
    'CHR-DD/MM/YYYY',
  ];

  boundaryChangeDict = {
    'no characters': 0,
    'one character': 1,
    'two characters': 2,
  };

  aligmentChekcDict = {
    customize: false,
    alignRight: false,
    alignLeft: false,
    GB: true,
  };

  weekContent: any = {
    iDisplayWeekEnabled: {
      true: 'WeekDay',
      false: ''
    },
    sTimeStyle: {
      '12hour': 'hh:mm:ss AM/PM',
      '24hour': 'hh:mm:ss'
    }
  };

  maxCharLen: number = 10;
  isUniversalAligment: boolean = false;
  isCustomizeColor: boolean = false;
  private iNormalizedScreenHeight: number;
  private iNormalizedScreenWidth: number;
  private viewInit: boolean = false;
  private drawArrayInit: boolean = false;
  private charLength: number = 14;
  private alignChange: any;
  private colorChange: any;
  private channelChange: any;
  private dateChange: any;
  private charChange: any;

  OsdForm = this.fb.group ({
    attribute: this.fb.group({
      iBoundary: [''],
      sAlignment: [''],
      sOSDAttribute: [''],
      sOSDFontSize: [''],
      sOSDFrontColor: [''],
      sOSDFrontColorMode: [''],
    }),
    channelNameOverlay: this.fb.group({
      iPositionX: [''],
      iPositionY: [''],
      sChannelName: [''],
      iChannelNameOverlayEnabled: [''],
    }),
    characterOverlay: this.fb.array([
      this.fb.group({
        iPositionX: [''],
        iPositionY: [''],
        id: 0,
        sDisplayText: [''],
        sIsPersistentText: [''],
        iTextOverlayEnabled: [''],
      }),
      this.fb.group({
        iPositionX: [''],
        iPositionY: [''],
        id: 1,
        sDisplayText: [''],
        sIsPersistentText: [''],
        iTextOverlayEnabled: [''],
      }),
      this.fb.group({
        iPositionX: [''],
        iPositionY: [''],
        id: 2,
        sDisplayText: [''],
        sIsPersistentText: [''],
        iTextOverlayEnabled: [''],
      }),
      this.fb.group({
        iPositionX: [''],
        iPositionY: [''],
        id: 3,
        sDisplayText: [''],
        sIsPersistentText: [''],
        iTextOverlayEnabled: [''],
      }),
      this.fb.group({
        iPositionX: [''],
        iPositionY: [''],
        id: 4,
        sDisplayText: [''],
        sIsPersistentText: [''],
        iTextOverlayEnabled: [''],
      }),
      this.fb.group({
        iPositionX: [''],
        iPositionY: [''],
        id: 5,
        sDisplayText: [''],
        sIsPersistentText: [''],
        iTextOverlayEnabled: [''],
      }),
      this.fb.group({
        iPositionX: [''],
        iPositionY: [''],
        id: 6,
        sDisplayText: [''],
        sIsPersistentText: [''],
        iTextOverlayEnabled: [''],
      }),
      this.fb.group({
        iPositionX: [''],
        iPositionY: [''],
        id: 7,
        sDisplayText: [''],
        sIsPersistentText: [''],
        iTextOverlayEnabled: [''],
      }),
    ]),
    dateTimeOverlay: this.fb.group({
      iPositionX: [''],
      iPositionY: [''],
      sDateStyle: [''],
      iDateTimeOverlayEnabled: [''],
      iDisplayWeekEnabled: [''],
      sTimeStyle: [''],
    }),
    normalizedScreenSize: this.fb.group({
      iNormalizedScreenHeight: [''],
      iNormalizedScreenWidth: [''],
    })
  });

  get attribute(): FormGroup {
    return this.OsdForm.get('attribute') as FormGroup;
  }

  get channelNameOverlay(): FormGroup {
    return this.OsdForm.get('channelNameOverlay') as FormGroup;
  }

  get characterOverlay(): FormArray {
    return this.OsdForm.get('characterOverlay') as FormArray;
  }

  get dateTimeOverlay(): FormGroup {
    return this.OsdForm.get('dateTimeOverlay') as FormGroup;
  }

  get normalizedScreenSize(): FormGroup {
    return this.OsdForm.get('normalizedScreenSize') as FormGroup;
  }

  ngOnInit() {
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

    this.isChrome = this.ieCss.getChromeBool();
    this.isIe = this.ieCss.getIEBool();
    this.cfgService.getOsdOverplaysInterface().subscribe(
      (res: OsdOverplaysInterface) => {
        this.resError.analyseRes(res);
        if (res.attribute.sOSDFrontColor && !res.attribute.sOSDFrontColor.match('#')) {
          res.attribute.sOSDFrontColor = '#' + res.attribute.sOSDFrontColor;
        }
        this.OsdForm.patchValue(res);
        // forbid osd attribute for no function forTest
        this.OsdForm.get('attribute.sOSDAttribute').setValue('opaque/not-flashing');
        this.isUniversalAligment = this.aligmentChekcDict[this.OsdForm.get('attribute').value.sAlignment];
        this.iNormalizedScreenHeight = res.normalizedScreenSize.iNormalizedScreenHeight;
        this.iNormalizedScreenWidth = res.normalizedScreenSize.iNormalizedScreenWidth;
        if (this.viewInit) {
          this.initDrawArray();
        }
      },
      err => {
        this.tips.setRbTip('getParaFailFreshPlease');
      }
    );

  }

  ngAfterViewInit(): void {
    // forbid osd attribute for no function forTest
    this.Re2.setAttribute(this.el.nativeElement.querySelector('.custom-select.attribute-disabled'), 'disabled', 'true');

    this.playerChild.hideBigPlayBtn();
    this.viewInit = true;
    this.initDrawArray();
    const playerButton = this.playerDom.nativeElement.querySelectorAll('.blue-btn');
    for (const btn of playerButton) {
      this.Re2.setAttribute(btn, 'hidden', 'true');
    }

    this.alignChange = this.OsdForm.get('attribute').get('sAlignment').valueChanges
      .subscribe((value: string) => {
        if (value === 'GB') {
          this.isUniversalAligment = true;
        } else {
          this.isUniversalAligment = false;
        }
      });

    this.colorChange = this.OsdForm.get('attribute').get('sOSDFrontColorMode').valueChanges
      .subscribe((value: string) => {
        if (value === 'customize') {
          this.isCustomizeColor = true;
        } else {
          this.isCustomizeColor = false;
        }
      });

    this.channelChange = this.OsdForm.get('channelNameOverlay').valueChanges
    .subscribe(change => {
      if (this.OsdForm.get('channelNameOverlay').value.iChannelNameOverlayEnabled) {
        this.Re2.removeAttribute(this.el.nativeElement.querySelector('.form-control.channel-name'), 'disabled');
      } else {
        this.Re2.setAttribute(this.el.nativeElement.querySelector('.form-control.channel-name'), 'disabled', 'true');
      }
      if (this.drawArrayInit) {
        this.playerChild.drawArray[1].enabled = change.iChannelNameOverlayEnabled;
        this.playerChild.setCharArray(1, change.sChannelName);
        this.playerChild.drawPic();
      }
    });

    this.dateChange = this.OsdForm.get('dateTimeOverlay').valueChanges
    .subscribe(change => {
      if (this.OsdForm.get('dateTimeOverlay').value.iDateTimeOverlayEnabled) {
        this.Re2.removeAttribute(this.el.nativeElement.querySelector('input.displayweek'), 'disabled');
      } else {
        this.Re2.setAttribute(this.el.nativeElement.querySelector('input.displayweek'), 'disabled', 'true');
      }
      if (this.drawArrayInit) {
        this.playerChild.drawArray[0].enabled = change.iDateTimeOverlayEnabled;
        const newContent = ''.concat(
          change.sDateStyle,
          ' ',
          this.weekContent.iDisplayWeekEnabled[String(Boolean(change.iDisplayWeekEnabled))],
          ' ',
          this.weekContent.sTimeStyle[change.sTimeStyle]
          );
        this.playerChild.setCharArray(0, newContent);
        this.playerChild.drawPic();
      }
    });

    this.charChange = this.OsdForm.get('characterOverlay').valueChanges.subscribe(change => {
      if (this.drawArrayInit) {
        for (let i = 2; i <= 9; i++) {
          this.playerChild.drawArray[i].enabled = change[i - 2].iTextOverlayEnabled;
          const newContent = change[i - 2].sDisplayText;
          this.playerChild.setCharArray(i, newContent);
        }
        this.playerChild.drawPic();
      }
    });
  }

  ngOnDestroy() {
    this.alignChange.unsubscribe();
    this.colorChange.unsubscribe();
    this.dateChange.unsubscribe();
    this.channelChange.unsubscribe();
    this.charChange.unsubscribe();
    this.stopPlayer();
  }

  getFontSizeNumber(fontString: string) {
    const pat = /(\d{1,2})\*\d{1,2}/;
    const rst = pat.exec(fontString);
    if (rst) {
      return Number(rst[1]);
    } else {
      this.logger.error('set font fail, set 16 default!');
      return 16;
    }
  }

  initDrawArray() {
    if (this.iNormalizedScreenHeight && this.iNormalizedScreenWidth) {
      this.drawArrayInit = true;
      this.playerChild.pointEnabled = false;
      this.playerChild.isDrawing = false;
      this.playerChild.initDrawer(this.iNormalizedScreenWidth, this.iNormalizedScreenHeight);
      this.maxCharLen = this.playerChild.maxContentLenght;
      // this.playerChild.setFontSize(this.getFontSizeNumber(this.OsdForm.value.attribute.sOSDFontSize));
      // order: date, channel name, characteroverlay
      const timeContent = ''.concat(
        this.OsdForm.get('dateTimeOverlay').value.sDateStyle,
        ' ',
        this.weekContent.iDisplayWeekEnabled[String(Boolean(this.OsdForm.get('dateTimeOverlay').value.iDisplayWeekEnabled))],
        ' ',
        this.weekContent.sTimeStyle[this.OsdForm.get('dateTimeOverlay').value.sTimeStyle]
        );
      this.playerChild.pushCharArray(
        this.OsdForm.get('dateTimeOverlay').value.iDateTimeOverlayEnabled, this.OsdForm.get('dateTimeOverlay').value.iPositionX,
        this.OsdForm.get('dateTimeOverlay').value.iPositionY, timeContent, false);
      this.playerChild.pushCharArray(
        this.OsdForm.get('channelNameOverlay').value.iChannelNameOverlayEnabled, this.OsdForm.get('channelNameOverlay').value.iPositionX,
        this.OsdForm.get('channelNameOverlay').value.iPositionY, this.OsdForm.get('channelNameOverlay').value.sChannelName, false);
      for (let i = 0; i <= 7; i++) {
        this.playerChild.pushCharArray(
          this.OsdForm.get('characterOverlay').value[i].iTextOverlayEnabled, this.OsdForm.get('characterOverlay').value[i].iPositionX,
          this.OsdForm.get('characterOverlay').value[i].iPositionY, this.OsdForm.get('characterOverlay').value[i].sDisplayText, false);
      }
      this.playerChild.drawPic();
    }
  }

  onSubmit() {
    this.transformGroup();
    this.cfgService.setOsdOverplaysInterface(this.OsdForm.value).subscribe(
      res => {
        this.resError.analyseRes(res);
        if (res.attribute.sOSDFrontColor && !res.attribute.sOSDFrontColor.match('#')) {
          res.attribute.sOSDFrontColor = '#' + res.attribute.sOSDFrontColor;
        }
        this.OsdForm.patchValue(res);
        // this.initDrawArray();
        this.tips.showSaveSuccess();
      },
      err => {
        this.logger.error(err, 'onSubmit:setOsdOverplaysInterface:');
        this.tips.showSaveFail();
      }
    );
  }

  transformGroup() {
    const cell = [];
    for (let i = 0; i <= 9; i++) {
      cell.push(this.playerChild.getDrawArray(i));
    }
    this.OsdForm.get('dateTimeOverlay').value.iPositionX = cell[0].x;
    this.OsdForm.get('dateTimeOverlay').value.iPositionY = cell[0].y;
    this.OsdForm.get('channelNameOverlay').value.iPositionX = cell[1].x;
    this.OsdForm.get('channelNameOverlay').value.iPositionY = cell[1].y;
    for (let i = 2; i <= 9; i++) {
      this.OsdForm.get('characterOverlay').value[i - 2].iPositionX = cell[i].x;
      this.OsdForm.get('characterOverlay').value[i - 2].iPositionY = cell[i].y;
    }
    this.pfs.formatInt(this.OsdForm.value);
    if (this.OsdForm.value.attribute.sOSDFrontColor.match('#')) {
      this.OsdForm.value.attribute.sOSDFrontColor = this.OsdForm.value.attribute.sOSDFrontColor.slice(1);
    }
  }

  setOsdCanvas(): void {
    this.playerChild.reshapeCanvas();
    this.initDrawArray();
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
      this.playerChild.destroyWhenSwitch();
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
}
