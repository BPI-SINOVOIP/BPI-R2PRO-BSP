import { Component, OnInit, Input, ElementRef, OnDestroy } from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import { LastNavService } from 'src/app/shared/func-service/last-nav.service';
import { LayoutService } from 'src/app/shared/func-service/layout.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';

@Component({
  selector: 'app-config-network',
  templateUrl: './config-network.component.html',
  styleUrls: ['./config-network.component.scss']
})
export class ConfigNetworkComponent implements OnInit, OnDestroy {

  constructor(
    private _route: ActivatedRoute,
    private lns: LastNavService,
    private el: ElementRef,
    private los: LayoutService,
    private pfs: PublicFuncService,
  ) { }

  private losOb: any;
  private rtOb: any;
  private rtObOnce: any;
  private navGroup: string = 'network';
  private layoutGroup: Array<string>;
  selectedMenuItem: string;
  subMenuItems: Array<string>;
  subMenuItemsBasic: Array<string> = [
    'TCPIP',
    'DDNS',
    'PPPoE',
    'Port',
    'uPnP'
  ];

  subMenuItemsAdvanced: Array<string> = [
    'Wi-Fi',
    'SMTP',
    'FTP',
    'eMail',
    'Cloud',
    'Protocol',
    'QoS',
    'Https'
  ];

  ngOnInit() {

    this.losOb = this.los.FinishSignal.subscribe(
      (si: boolean) => {
        if (si) {
          this.layoutGroup = this.los.getThirdLt('config', 'Network');
          for (const thirdLy of this.layoutGroup) {
            switch (thirdLy) {
              case 'Basic':
                this.subMenuItemsBasic = this.los.getfourthLt('config', 'Network', 'Basic');
                break;
              case 'Advanced':
                this.subMenuItemsAdvanced = this.los.getfourthLt('config', 'Network', 'Advanced');
                break;
            }
          }
          this.rtOb = this._route.queryParamMap.subscribe(params => {
            const menu = params.get('menu');
            if (this.pfs.isInArrayString(this.layoutGroup, menu)) {
              if (params.get('menu') === 'Basic') {
                this.setMenu('Basic');
                this.subMenuItems = this.subMenuItemsBasic;
              } else if (params.get('menu') === 'Advanced') {
                this.setMenu('Advanced');
                this.subMenuItems = this.subMenuItemsAdvanced;
              } else {
                this.setMenu('Basic');
                this.subMenuItems = this.subMenuItemsBasic;
              }
            } else {
              this.setMenu('Basic');
              this.subMenuItems = this.subMenuItemsBasic;
            }
          });
          if (this.losOb) {
            this.losOb.unsubscribe();
            this.losOb = null;
          }
        }
      }
    );
  }

  ngOnDestroy() {
    if (this.losOb) {
      this.losOb.unsubscribe();
    }
    if (this.rtOb) {
      this.rtOb.unsubscribe();
    }
    if (this.rtObOnce) {
      this.rtObOnce.unsubscribe();
    }
  }

  setMenu(itemName: string) {
    if (itemName !== this.selectedMenuItem) {
      this.selectedMenuItem = itemName;
      this.lns.getLastNav(this.navGroup + itemName, this.el);
    }
  }

  freshMenu() {
    this.rtObOnce = this._route.queryParamMap.subscribe(params => {
      if (params.get('menu') === 'Basic') {
        this.setMenu('Basic');
        this.subMenuItems = this.subMenuItemsBasic;
      } else if (params.get('menu') === 'Advanced') {
        this.setMenu('Advanced');
        this.subMenuItems = this.subMenuItemsAdvanced;
      }
      if (this.rtObOnce) {
        this.rtObOnce.unsubscribe();
        this.rtObOnce = null;
      }
    });
  }

  setActiveTab(tab: string): void {
    this.lns.setLastNav(this.navGroup + this.selectedMenuItem, tab + 'Tab');
  }
}
