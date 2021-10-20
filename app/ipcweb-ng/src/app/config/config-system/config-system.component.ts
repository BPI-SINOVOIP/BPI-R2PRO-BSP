import { Component, OnInit, Input, ElementRef, OnDestroy } from '@angular/core';
import { ActivatedRoute, Router } from '@angular/router';
import { LastNavService } from 'src/app/shared/func-service/last-nav.service';
import { LayoutService } from 'src/app/shared/func-service/layout.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';

@Component({
  selector: 'app-config-system',
  templateUrl: './config-system.component.html',
  styles: []
})
export class ConfigSystemComponent implements OnInit, OnDestroy {

  constructor(
    private _route: ActivatedRoute,
    private router: Router,
    private lns: LastNavService,
    private el: ElementRef,
    private los: LayoutService,
    private pfs: PublicFuncService,
  ) { }

  private navGroup: string = 'system';
  selectedMenuItem: string;
  activeTab: string = '';
  private losOb: any;
  private rtOb: any;
  private paramOb: any;
  private layoutGroup: Array<string>;

  subMenuItems: Array<string>;
  subMenuItemsSettings: Array<string> = [
    'basic',
    'time',
  ];

  subMenuItemsMaintain: Array<string> = [
    'upgrade',
    'log',
  ];

  subMenuItemsUser: Array<string> = [];

  subMenuItemsSecurity: Array<string> = [
    'authentication',
    'ipAddrFilter',
    'securityService',
  ];

  ngOnInit() {
    this.losOb = this.los.FinishSignal.subscribe(
      (change: boolean) => {
        if (change) {
          this.layoutFunc();
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
    if (this.paramOb) {
      this.paramOb.unsubscribe();
    }
  }

  layoutFunc = () => {
    if (this.paramOb) {
      this.paramOb.unsubscribe();
      this.paramOb = null;
    }
    this.layoutGroup = this.los.getThirdLt('config', 'System');
    if (this.layoutGroup.length === 0) {
      this.router.navigate(['/']);
    }
    for (const thirdLy of this.layoutGroup) {
      switch (thirdLy) {
        case 'Settings':
          this.subMenuItemsSettings = this.los.getfourthLt('config', 'System', 'Settings');
          break;
        case 'Maintain':
          this.subMenuItemsMaintain = this.los.getfourthLt('config', 'System', 'Maintain');
          break;
        case 'Security':
          this.subMenuItemsSecurity = this.los.getfourthLt('config', 'System', 'Security');
          break;
      }
    }
    this.paramOb = this._route.queryParamMap.subscribe(params => {
      const menu = params.get('menu');
      if (this.pfs.isInArrayString(this.layoutGroup, menu)) {
        if (menu === 'Settings') {
          this.setMenu('Settings');
          this.subMenuItems = this.subMenuItemsSettings;
        } else if (menu === 'Maintain') {
          this.setMenu('Maintain');
          this.subMenuItems = this.subMenuItemsMaintain;
        } else if (menu === 'User') {
          this.setMenu('User');
          this.subMenuItems = this.subMenuItemsUser;
        } else if (menu === 'Security') {
          this.setMenu('Security');
          this.subMenuItems = this.subMenuItemsSecurity;
        } else {
          this.setMenu('Settings');
          this.subMenuItems = this.subMenuItemsSettings;
        }
      } else {
        this.setMenu('Settings');
        this.subMenuItems = this.subMenuItemsSettings;
      }
    });
  }

  setMenu(itemName: string) {
    if (itemName !== this.selectedMenuItem) {
      this.selectedMenuItem = itemName;
      this.lns.getLastNav(this.navGroup + itemName, this.el);
    }
  }

  setActiveTab(tab: string): void {
    this.activeTab = tab;
    this.lns.setLastNav(this.navGroup + this.selectedMenuItem, tab + 'Tab');
  }

  isActiveTab(tab: string): boolean {
    return (tab === this.activeTab);
  }

}
