import { Component, OnInit, Renderer2, ElementRef, OnDestroy} from '@angular/core';
import { ActivatedRoute } from '@angular/router';

import { ConfigService } from '../config.service';
import { MenuGroup } from './MenuGroup';
import { LayoutService } from 'src/app/shared/func-service/layout.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';

@Component({
  selector: 'app-config',
  templateUrl: './config.component.html',
  styleUrls: ['./config.component.scss']
})
export class ConfigComponent implements OnInit, OnDestroy {

  public menuGroups: Array<MenuGroup> = [];
  public selectedGroup: string = '';
  public selectedMenuItem: string = '';
  private heightChange: any;
  private losOb: any;
  private isInit: boolean = false;
  private onceParamOb: any;
  private urlOb: any;

  constructor(
    private cfgService: ConfigService,
    private Re2: Renderer2,
    private el: ElementRef,
    private los: LayoutService,
    private route: ActivatedRoute,
    private pfs: PublicFuncService,
  ) {}

  private path2Click: any = {
    System: '.system.for-click',
    Network: '.network.for-click',
    Video: '.video.for-click',
    Audio: '.audio.for-click',
    Image: '.image.for-click',
    Event: '.event.for-click',
    Storage: '.storage.for-click',
    Intel: '.intel.for-click',
    Peripherals: '.peripherals.for-click',
  };

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

  layoutFunc = () => {
    this.isInit = true;
    this.menuGroups = this.los.getSecondLt('config', true);
    this.urlOb = this.route.children[0].url.subscribe(
      url => {
        let path = url[0].path;
        const groupArray = this.los.getSecondLt('config');
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
          const menuArray = this.los.getThirdLt('config', path);
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
  }

  onMenuGroupClicked(selectedGroup: MenuGroup) {
    if (this.isInit && this.selectedGroup !== selectedGroup.name) {
      const menuArray = this.los.getThirdLt('config', selectedGroup.name);
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
