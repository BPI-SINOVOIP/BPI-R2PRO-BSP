import { Injectable } from '@angular/core';

@Injectable({
  providedIn: 'root'
})
export class IeCssService {

  constructor() { }

  addBeforeAfter(dom: any) {
    // console.log(this.getExplorer(), dom);
    // if (this.getExplorer() !== 'ie') {
    //   return;
    // }
    // console.log(document.querySelector, dom, dom.nodeType);
    if (!dom && dom.nodeType !== 1) {
      return;
    }
    const content = dom.getAttribute('data-content') || '';
    const before = document.createElement("before");
    const after = document.createElement("after");
    before.innerHTML = content;
    after.innerHTML = content;
    dom.insertBefore(before, dom.firstChild);
    dom.appendChild(after);
    console.log(dom);
  }

  /* eslint-disable */
  getExplorer = () => {
    const explorer = window.navigator.userAgent;
    if ('ActiveXObject' in window || explorer.indexOf('MSIE') >= 0) {
      return 'ie';
    } else if (explorer.indexOf('Edge') >= 0) {
      return 'Edge';
    } else if (explorer.indexOf('Firefox') >= 0) {
      return 'Firefox';
    } else if (explorer.indexOf('Chrome') >= 0) {
      return 'Chrome';
    } else if (explorer.indexOf('Opera') >= 0) {
      return 'Opera';
    } else if (explorer.indexOf('Safari') >= 0) {
      return 'Safari';
    }
    // unknow set ie default
    return 'ie';
  }

  // only brower is really chrome return true
  getChromeBool() {
    const explorer = window.navigator.userAgent;
    if ('ActiveXObject' in window || explorer.indexOf('MSIE') >= 0) {
      return false;
    } else if (explorer.indexOf('Edge') >= 0) {
      return false;
    } else if (explorer.indexOf('Firefox') >= 0) {
      return false;
    } else if (explorer.indexOf('Opera') >= 0) {
      return false;
    } else if (explorer.indexOf('Chrome') >= 0) {
      return true;
    }
    // unknow set ie default
    return false;
  }

  // return ture when ie
  getIEBool() {
    if (this.getExplorer() === 'ie') {
      return true;
    }
    return false;
  }

}
