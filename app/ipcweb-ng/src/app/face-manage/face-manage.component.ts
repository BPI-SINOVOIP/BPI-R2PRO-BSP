import { Component, OnInit, Renderer2, ElementRef, OnDestroy } from '@angular/core';
import { ActivatedRoute } from '@angular/router';

import { ConfigService } from '../config.service';
import { LayoutService } from 'src/app/shared/func-service/layout.service';
import { MenuGroup } from '../shared/MenuGroup';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';

@Component({
  selector: 'app-face-manage',
  templateUrl: './face-manage.component.html',
  styleUrls: ['./face-manage.component.scss']
})
export class FaceManageComponent implements OnInit, OnDestroy {

  constructor(
    private cfg: ConfigService,
    private Re2: Renderer2,
    private el: ElementRef,
    private los: LayoutService,
    private route: ActivatedRoute,
    private pfs: PublicFuncService,
  ) { }

  private heightChange: any;
  public menuGroups: Array<MenuGroup> = [];
  public selectedGroup: string = 'Manage';
  public selectedMenuItem: string = 'MemberList';
  private losOb: any;
  private isInit: boolean = false;
  private onceParamOb: any;

  ngOnInit(): void {
    this.losOb = this.los.FinishSignal.subscribe(
      (change: boolean) => {
        if (change) {
          this.isInit = true;
          this.menuGroups = this.los.getSecondLt('face-manage', true);
          if (this.losOb) {
            this.losOb.unsubscribe();
            this.losOb = null;
          }
          this.pfs.waitAInit(5000, '.manage.for-click', this.el)
            .then(
              () => {
                const oldItem = this.selectedMenuItem;
                this.el.nativeElement.querySelector('.manage.for-click').click();
                if (oldItem !== this.selectedMenuItem) {
                  this.selectedMenuItem = oldItem;
                }
              }
            )
            .catch();
          const menuArray = this.los.getThirdLt('face-manage', 'Manage');
          this.onceParamOb = this.route.queryParamMap.subscribe(params => {
            let menu = params.get('menu');
            if (menuArray.length > 0 && !this.pfs.isInArrayString(menuArray, menu)) {
              menu = menuArray[0];
            }
            if (menu === 'MemberList') {
              this.selectedMenuItem = 'MemberList';
            } else if (menu === 'AddOne') {
              this.selectedMenuItem = 'AddOne';
            } else if (menu === 'BatchInput') {
              this.selectedMenuItem = 'BatchInput';
            } else if (menu === 'SnapShot') {
              this.selectedMenuItem = 'SnapShot';
            } else if (menu === 'Control') {
              this.selectedMenuItem = 'Control';
            } else {
              this.selectedMenuItem = 'MemberList';
            }
            if (this.onceParamOb) {
              this.onceParamOb.unsubscribe();
              this.onceParamOb = null;
            }
          });
        }
      }
    );

    this.heightChange = this.los.sideBarHeight.subscribe((change: number) => {
      if (!isNaN(change) && change !== 0) {
        this.Re2.setStyle(this.el.nativeElement.querySelector('.col-md-2.col-xl-2.sidebar.left'), 'height', change + 'px');
      }
    });

  }

  ngOnDestroy() {
    if (this.heightChange) {
      this.heightChange.unsubscribe();
    }
    if (this.losOb) {
      this.losOb.unsubscribe();
    }
    if (this.onceParamOb) {
      this.onceParamOb.unsubscribe();
    }
  }

  onMenuGroupClicked(selectedGroup: MenuGroup) {
    if (this.isInit && this.selectedGroup !== selectedGroup.name) {
      const menuArray = this.los.getThirdLt('face-manage', selectedGroup.name);
      if (menuArray.length > 0) {
        this.selectedMenuItem = menuArray[0];
      }
    }
    this.selectedGroup = selectedGroup.name;
  }

  onMenuItemClicked(menuItem: string) {
    this.selectedMenuItem = menuItem;
  }

  isSubMenuActive(menuItem: string) {
    if (this.selectedMenuItem === menuItem) {
      return true;
    }
    return false;
  }

}
