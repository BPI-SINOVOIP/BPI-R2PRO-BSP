import { Component, OnInit, ViewChild, OnDestroy, AfterViewInit, ElementRef} from '@angular/core';

import { ConfigService } from 'src/app/config.service';
import { ActivatedRoute } from '@angular/router';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { TipsService } from 'src/app/tips/tips.service';
import { LayoutService } from 'src/app/shared/func-service/layout.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { DiyHttpService } from 'src/app/shared/func-service/diy-http.service';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-config-image',
  templateUrl: './config-image.component.html',
  styleUrls: ['./config-image.component.scss']
})
export class ConfigImageComponent implements OnInit, AfterViewInit, OnDestroy {

  @ViewChild('osd', {static: false}) osdCon: any;
  @ViewChild('isp', {static: false}) ispCon: any;
  @ViewChild('mask', {static: false}) MaskCon: any;
  @ViewChild('pic', {static: false}) PicCon: any;

  constructor(
    private cfgService: ConfigService,
    private el: ElementRef,
    private route: ActivatedRoute,
    private resError: ResponseErrorService,
    private tips: TipsService,
    private pfs: PublicFuncService,
    private los: LayoutService,
    private dhs: DiyHttpService,
  ) { }

  private logger: Logger = new Logger('config-image');
  afterInit: boolean = false;
  selectedMenuItem: string;
  subMenuItems: Array<string>;
  subMenuItemsBasic: Array<string> = [
    'isp',
    'osd',
    'mask',
    'pictureMask'
  ];
  private lastActiveTab: string = 'isp';
  private src = '';
  private isViewInit = false;
  selectedTab = 'isp';
  option = {};
  private routeObserver: any;
  private losOb: any;
  private layoutGroup: Array<string>;
  private paramOb: any;
  private urlOb: any;

  ngOnInit() {
    this.losOb = this.los.FinishSignal.subscribe(
      (si: boolean) => {
        if (si) {
          this.layoutGroup = this.los.getThirdLt('config', 'Image');
          this.paramOb = this.routeObserver = this.route.queryParamMap.subscribe(params => {
            let menu = params.get('menu');
            if (!this.pfs.isInArrayString(this.layoutGroup, menu) && this.layoutGroup.length > 0) {
              menu = this.layoutGroup[0];
            }
            if (menu === 'DisplaySettings') {
              this.selectedMenuItem = 'isp';
            } else if (menu === 'OSDSettings') {
              this.selectedMenuItem = 'osd';
            } else if (menu === 'PrivacyCover') {
              this.selectedMenuItem = 'mask';
            } else if (menu === 'PictureMask') {
              this.selectedMenuItem = 'pictureMask';
            } else {
              this.selectedMenuItem = 'isp';
            }
          });
          if (this.losOb) {
            this.losOb.unsubscribe();
            this.losOb = null;
          }
        }
      }
    );

    this.getDisplayUrl();
  }

  ngAfterViewInit() {
  }

  ngOnDestroy() {
    if (this.routeObserver) {
      this.routeObserver.unsubscribe();
    }
    if (this.losOb) {
      this.losOb.unsubscribe();
    }
    if (this.paramOb) {
      this.paramOb.unsubscribe();
    }
    if (this.urlOb) {
      this.urlOb.unsubscribe();
    }
  }

  getDisplayUrl() {
    this.urlOb = this.dhs.getStreamUrl().subscribe(
      (msg: string) => {
        if (msg) {
          this.src = msg;
          this.option['src'] = this.src;
        } else {
          this.tips.setRbTip('getVideoUrlFail');
        }
        this.urlOb.unsubscribe();
        this.urlOb = null;
      }
    );
  }
}
