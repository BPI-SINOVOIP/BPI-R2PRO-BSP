import { Injectable, ElementRef } from '@angular/core';
import { PublicFuncService } from './public-func.service';

@Injectable({
  providedIn: 'root'
})
export class LastNavService {

  private navRootName: string = 'nav';
  constructor(
    private pfs: PublicFuncService,
  ) { }

  setLastNav(groupName: string, navId: string) {
    let items = {};
    if (localStorage.getItem(this.navRootName)) {
      const itemsString = localStorage.getItem(this.navRootName);
      items = JSON.parse(itemsString);
    }
    items[groupName] = navId;
    const itemStorage: string = JSON.stringify(items);
    localStorage.setItem(this.navRootName, itemStorage);
  }

  getLastNav(groupName: string, el: ElementRef) {
    if (localStorage.getItem(this.navRootName)) {
      const itemsString = localStorage.getItem(this.navRootName);
      const items = JSON.parse(itemsString);
      if (items && items[groupName]) {
        const nav = items[groupName];
        this.waitElInit(5000, nav, el, '.nav-link')
          .then(
            () => {
              document.getElementById(nav).click();
            }
          )
          .catch(
            () => {}
          );
        return nav;
      } else {
        return null;
      }
    } else {
      return null;
    }
  }

  waitElInit = (timeoutms: number, navId: string, el: ElementRef, className = '.nav-link') => new Promise(
    (resolve, reject) => {
      const waitFunc = () => {
        timeoutms -= 100;
        let navList =  el.nativeElement.querySelectorAll(className);
        if (navList && navList.length) {
          // tslint:disable-next-line: prefer-for-of
          for (let i = 0; i < navList.length; i++) {
            if (navList[i].id === navId) {
              navList = null;
              resolve();
              return;
            }
          }
        }
        if (timeoutms < 0) {
          navList = null;
          reject();
        } else {
          navList = null;
          setTimeout(waitFunc, 100);
        }
      };
      waitFunc();
    }
  )

  getLastNavName(groupName: string) {
    if (localStorage.getItem(this.navRootName)) {
      const itemsString = localStorage.getItem(this.navRootName);
      const items = JSON.parse(itemsString);
      if (items && items[groupName]) {
        return items[groupName];
      }
    }
    return null;
  }
}
