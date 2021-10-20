import { Component, OnInit, Renderer2, ElementRef, OnDestroy } from '@angular/core';
import { ActivatedRoute } from '@angular/router';

import { ConfigService } from '../config.service';
import { LayoutService } from 'src/app/shared/func-service/layout.service';
import { MenuGroup } from '../shared/MenuGroup';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';

@Component({
  selector: 'app-face-para',
  templateUrl: './face-para.component.html',
  styleUrls: ['./face-para.component.scss']
})
export class FaceParaComponent implements OnInit, OnDestroy {

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
  public selectedGroup: string = 'Config';
  public selectedMenuItem: string = '';
  private losOb: any;
  private isInit: boolean = false;
  private onceParamOb: any;

  ngOnInit(): void {
    this.losOb = this.los.FinishSignal.subscribe(
      (change: boolean) => {
        if (change) {
          this.isInit = true;
          this.menuGroups = this.los.getSecondLt('face-para', true);
          if (this.losOb) {
            this.losOb.unsubscribe();
            this.losOb = null;
          }
          this.pfs.waitAInit(5000, '.config.for-click', this.el)
            .then(
              () => {
                const oldItem = this.selectedMenuItem;
                this.el.nativeElement.querySelector('.config.for-click').click();
                if (oldItem !== this.selectedMenuItem) {
                  this.selectedMenuItem = oldItem;
                }
              }
            )
            .catch();
          const menuArray = this.los.getThirdLt('face-para', 'Config');
          this.onceParamOb = this.route.queryParamMap.subscribe(params => {
            let menu = params.get('menu');
            if (menuArray.length > 0 && !this.pfs.isInArrayString(menuArray, menu)) {
              menu = menuArray[0];
            }
            if (menu === 'FacePara') {
              this.selectedMenuItem = 'FacePara';
            } else if (menu === 'ROI') {
              this.selectedMenuItem = 'ROI';
            } else {
              this.selectedMenuItem = 'FacePara';
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
      const menuArray = this.los.getThirdLt('face-para', selectedGroup.name);
      if (menuArray.length > 0) {
        this.selectedMenuItem = menuArray[0];
      }
    }
    this.selectedGroup = selectedGroup.name;
    // this.selectedMenuItem = selectedGroup.items[0];
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
