import { Component, OnInit, Renderer2, ElementRef, OnDestroy } from '@angular/core';
import { ActivatedRoute } from '@angular/router';

import { ConfigService } from '../config.service';
import { MenuGroup } from '../shared/MenuGroup';
import { LayoutService } from 'src/app/shared/func-service/layout.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';

@Component({
  selector: 'app-face',
  templateUrl: './face.component.html',
  styleUrls: ['./face.component.scss']
})
export class FaceComponent implements OnInit, OnDestroy {

  public menuGroups: Array<MenuGroup> = [];
  public selectedGroup: string = '';
  public selectedMenuItem: string = '';
  private setHeight: number;
  private bodyHeight: number = 0;
  private heightChange: any;
  private losOb: any;
  private isInit: boolean = false;
  private onceParamOb: any;
  private urlOb: any;

  constructor(
    private cfg: ConfigService,
    private Re2: Renderer2,
    private el: ElementRef,
    private los: LayoutService,
    private route: ActivatedRoute,
    private pfs: PublicFuncService,
  ) { }

  private path2Click: any = {
    MemberList: '.memberlist.for-click',
    SnapShot: '.snapshot.for-click',
    Control: '.control.for-click',
    Config: '.config.for-click'
  };

  ngOnInit(): void {

    this.losOb = this.los.FinishSignal.subscribe(
      (change: boolean) => {
        if (change) {
          this.isInit = true;
          this.menuGroups = this.los.getSecondLt('face', true);
          this.urlOb = this.route.children[0].url.subscribe(
            url => {
              let path = url[0].path;
              const groupArray = this.los.getSecondLt('face');
              if (!this.pfs.isInArrayString(groupArray, path) && groupArray.length > 0) {
                path = groupArray[0];
              }
              const clickClass = this.path2Click[path];
              if (clickClass) {
                this.pfs.waitAInit(5000, clickClass, this.el)
                  .then(
                    () => {
                      const oldItem = this.selectedMenuItem;
                      this.el.nativeElement.querySelector(clickClass).click();
                      if (oldItem !== this.selectedMenuItem) {
                        this.selectedMenuItem = oldItem;
                      }
                    }
                  )
                  .catch();
              }
              this.onceParamOb = this.route.children[0].queryParamMap.subscribe(params => {
                let menu = params.get('menu');
                const menuArray = this.los.getThirdLt('face', path);
                if (!this.pfs.isInArrayString(menuArray, menu) && menuArray.length > 0) {
                  menu = menuArray[0];
                }
                this.selectedMenuItem = menu;
                if (this.onceParamOb) {
                  this.onceParamOb.unsubscribe();
                  this.onceParamOb = null;
                }
              });
              if (this.urlOb) {
                this.urlOb.unsubscribe();
                this.urlOb = null;
              }
            }
          );
          if (this.losOb) {
            this.losOb.unsubscribe();
            this.losOb = null;
          }
        }
      }
    );

    // this.resizeSidebar();
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
      this.onceParamOb = null;
    }
    if (this.urlOb) {
      this.urlOb.unsubscribe();
      this.urlOb = null;
    }
  }

  resizeSidebar() {
    const headHeight = document.getElementById('head-nav').clientHeight;
    const footHeight = document.getElementById('footer-component').offsetHeight;
    const bodyHeight = window.innerHeight;
    if (bodyHeight > this.bodyHeight) {
      this.bodyHeight = bodyHeight;
    }
    this.setHeight = this.bodyHeight - headHeight - footHeight - 50;
    this.Re2.setStyle(this.el.nativeElement.querySelector('.col-md-2.col-xl-2.sidebar.left'), 'height', this.setHeight + 'px');
  }

  onMenuGroupClicked(selectedGroup: MenuGroup) {
    if (this.isInit && this.selectedGroup !== selectedGroup.name) {
      const menuArray = this.los.getThirdLt('face', selectedGroup.name);
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
