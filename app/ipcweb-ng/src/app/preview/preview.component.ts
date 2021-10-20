import { Component, OnInit, AfterViewInit, OnDestroy, ViewChild, ElementRef } from '@angular/core';
import { ViewEncapsulation } from '@angular/core';
import { FormBuilder } from '@angular/forms';
import { ConfigService } from '../config.service';
import Logger from '../logger';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { TipsService } from 'src/app/tips/tips.service';
import { DiyHttpService } from 'src/app/shared/func-service/diy-http.service';

@Component({
  selector: 'app-preview',
  templateUrl: './preview.component.html',
  styleUrls: ['./preview.component.scss'],
  encapsulation: ViewEncapsulation.None
})

export class PreviewComponent implements OnInit, AfterViewInit, OnDestroy {

  @ViewChild('player', {static: true}) playerChild: any;

  constructor(
    private cfgService: ConfigService,
    private fb: FormBuilder,
    private resError: ResponseErrorService,
    private tips: TipsService,
    private dhs: DiyHttpService,
  ) { }

  private logger: Logger = new Logger('config');
  private src: string;
  private isViewInit: boolean = false;
  isPTZEnabled: boolean = true;
  private urlOb: any;
  playerOption = {
    isReshape: false,
    speName: 'preview',
  };

  testForm = this.fb.group({
    url: 'ws://172.16.21.151:8081',
  });

  directions = [
    'left-up', 'up', 'right-up',
    'left', 'auto', 'right',
    'left-down', 'down', 'right-down'
  ];

  operations = [
    'zoom',
    'focus',
    'iris'
  ];

  ngOnInit() {
    this.urlOb = this.dhs.getStreamUrl().subscribe(
      (msg: string) => {
        if (msg) {
          this.src = msg;
          this.playEntry();
        } else {
          this.tips.setRbTip('getVideoUrlFail');
        }
        this.urlOb.unsubscribe();
        this.urlOb = null;
      }
    );
  }

  ngAfterViewInit() {
    this.isViewInit = true;
    this.playerChild.set4Preview();
    this.playEntry();
  }

  ngOnDestroy() {
    try {
      this.playerChild.diyStop();
    } catch {}
  }

  playEntry() {
    if (this.isViewInit && this.src) {
      this.playerChild.displayUrl = this.src;
      this.playerChild.bigBtnPlay();
    }
  }

  onConnect() {
    if (this.src) {
      this.playerChild.diyStop();
    }
    if (this.urlOb) {
      this.urlOb.unsubscribe();
    }
    this.src = this.testForm.value.url;
    this.playEntry();
  }
}
