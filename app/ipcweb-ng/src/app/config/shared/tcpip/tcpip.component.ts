import { Component, OnInit, Input, AfterViewInit, OnDestroy, ViewChild } from '@angular/core';
import { FormBuilder, FormControl } from '@angular/forms';

import { ConfigService } from 'src/app/config.service';
import { NetworkInterface } from '../../NetworkInterface';
import { isIpv4 } from 'src/app/shared/validators/is-ipv4.directive';
import { isIp } from 'src/app/shared/validators/is-ip.directive';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { TipsService } from 'src/app/tips/tips.service';
import { LockService } from 'src/app/shared/func-service/lock-service.service';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-tcpip',
  templateUrl: './tcpip.component.html',
  styleUrls: ['./tcpip.component.scss']
})
export class TcpipComponent implements OnInit, AfterViewInit, OnDestroy {

  constructor(
    private cfgService: ConfigService,
    private fb: FormBuilder,
    private ieCss: IeCssService,
    private resError: ResponseErrorService,
    private tips: TipsService,
  ) { }

  // private lock = new LockService(this.tips);
  private logger: Logger = new Logger('tcpip');

  isChrome: boolean = false;
  network: NetworkInterface;
  _iface: string ;
  private isAutoChange: any;

  nicSpeeds: Array<string> = [
    'auto',
    '10MHD',
    '10MD',
    '100MHD',
    '100MD',
    '1000MD'
  ];

  networkInterfaceForm = this.fb.group({
    link: this.fb.group ({
      sAddress: [''],
      sInterface: [''],
      sNicSpeed: [''],
      sDNS1: ['', isIp],
      sDNS2: ['', isIp],
    }),
    ipv4: this.fb.group ({
      sV4Address: ['', isIp],
      sV4Gateway: ['', isIp],
      sV4Method: [''],
      sV4Netmask: ['', isIpv4],
    }),
  });

  get sDNS1(): FormControl {
    return this.networkInterfaceForm.get('link.sDNS1') as FormControl;
  }

  get sDNS2(): FormControl {
    return this.networkInterfaceForm.get('link.sDNS2') as FormControl;
  }

  get sV4Address(): FormControl {
    return this.networkInterfaceForm.get('ipv4.sV4Address') as FormControl;
  }

  get sV4Gateway(): FormControl {
    return this.networkInterfaceForm.get('ipv4.sV4Gateway') as FormControl;
  }

  get sV4Netmask(): FormControl {
    return this.networkInterfaceForm.get('ipv4.sV4Netmask') as FormControl;
  }

  @Input()
  set iface(iface: string) {
    this._iface = (iface && iface.trim()) || '';
  }

  get iface(): string {
    return this._iface;
  }

  ngOnInit() {
    this.isChrome = this.ieCss.getChromeBool();
    this._iface = (this.iface && this.iface.trim()) || 'lan';
    if (this._iface.match('lan') && !(this._iface.match('wlan'))) {
      this.cfgService.getLanInterface().subscribe(
        (res: NetworkInterface) => {
          this.resError.analyseRes(res, 'noLanInfo');
          this.networkInterfaceForm.patchValue(res);
          this.nicSpeeds = res.link.sNicSpeedSupport.split(' ');
          if (this.nicSpeeds[this.nicSpeeds.length - 1] === '') {
            this.nicSpeeds.splice(this.nicSpeeds.length - 1, 1);
          }
          if (res.ipv4) {
            this.networkInterfaceForm.get('ipv4.sV4Method').setValue(res.ipv4.sV4Method.match('dhcp'));
          }
          this.network = res;
        },
        err => {
          this.logger.error(err, 'ngOnInit:getLanInterface:');
          this.tips.setRbTip('noLanInfo');
        }
      );
    } else if (this._iface.match('wlan')) {
      this.cfgService.getWLanInterface().subscribe(
        (res: NetworkInterface) => {
          this.resError.analyseRes(res, 'noWlanInfo', 'base-wlan');
          this.networkInterfaceForm.patchValue(res);
          if (res.ipv4) {
            this.networkInterfaceForm.get('ipv4.sV4Method').setValue(res.ipv4.sV4Method.match('dhcp'));
          }
          this.network = res;
        },
        err => {
          this.logger.error(err, 'ngOnInit:getWLanInterface:');
          this.tips.setRbTip('noWlanInfo');
        }
      );
    }
  }

  ngAfterViewInit() {
    this.isAutoChange = this.networkInterfaceForm.get('ipv4.sV4Method').valueChanges
    .subscribe(cheked => {
      if (cheked) {
        this.networkInterfaceForm.get('ipv4.sV4Address').disable();
        this.networkInterfaceForm.get('ipv4.sV4Netmask').disable();
        this.networkInterfaceForm.get('ipv4.sV4Gateway').disable();
        this.networkInterfaceForm.get('link.sDNS1').disable();
        this.networkInterfaceForm.get('link.sDNS2').disable();
      } else {
        this.networkInterfaceForm.get('ipv4.sV4Address').enable();
        this.networkInterfaceForm.get('ipv4.sV4Netmask').enable();
        this.networkInterfaceForm.get('ipv4.sV4Gateway').enable();
        this.networkInterfaceForm.get('link.sDNS1').enable();
        this.networkInterfaceForm.get('link.sDNS2').enable();
      }
    });
  }

  ngOnDestroy() {
    this.isAutoChange.unsubscribe();
  }

  onSubmit() {
    this.network.link = this.networkInterfaceForm.get('link').value;
    this.network.ipv4 = this.networkInterfaceForm.get('ipv4').value;
    this.network.ipv4.sV4Method = this.networkInterfaceForm.get('ipv4.sV4Method').value ? 'dhcp' : 'manual';
    if (this._iface.match('lan') && !(this._iface.match('wlan'))) {
      this.cfgService.setLanInterface(this.network)
        .subscribe(res => {
          this.resError.analyseRes(res);
          this.tips.showSaveSuccess();
        },
        err => {
          this.logger.error(err, 'onSubmit:setLanInterface:');
          this.tips.showSaveFail();
        });
    } else if (this._iface.match('wlan')) {
      this.cfgService.setWLanInterface(this.networkInterfaceForm.value)
        .subscribe(res => {
          this.resError.analyseRes(res);
          this.tips.showSaveSuccess();
        },
        err => {
          this.logger.error(err, 'onSubmit:setWLanInterface:');
          this.tips.showSaveFail();
        });
      }
    }
}
