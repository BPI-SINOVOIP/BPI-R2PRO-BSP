import { Injectable } from '@angular/core';
import { Subject } from 'rxjs';

@Injectable({
  providedIn: 'root'
})
export class TipsService {

  defaultTips = {
    init: 'initFailFreshPlease',
    saveSuccess: 'saveSuccess',
    saveFail: 'saveFail',
  };

  // cTipAction = ['onYes', 'onNo', 'hideAll', 'showAll'];

  constructor() { }

  private rbTip: Subject<string> = new Subject<string>();
  private cAction: Subject<string> = new Subject<string>();
  private cTip: Subject<string> = new Subject<string>();
  private cTipPara: Subject<string> = new Subject<string>();

  get rbContent() {
    return this.rbTip.asObservable();
  }

  /*
  waitTime = NaN means setting waitTime default
  set waitTime 0 means not close
  set waitTime <0 means close tip
  */
  setRbTip(tip: string, waitTime: number = NaN) {
    if (isNaN(waitTime)) {
      this.rbTip.next(tip);
    } else {
      const newTip = '{\"' + tip + '\":' + waitTime + '}';
      this.rbTip.next(newTip);
    }
  }

  showInitFail() {
    this.setRbTip(this.defaultTips.init);
  }

  showSaveSuccess() {
    this.setRbTip(this.defaultTips.saveSuccess);
  }

  showSaveFail() {
    this.setRbTip(this.defaultTips.saveFail);
  }

  get cContent() {
    return this.cTip.asObservable();
  }

  get ctAction() {
    return this.cAction.asObservable();
  }

  get ctPara() {
    return this.cTipPara.asObservable();
  }

  showCTip(tip: string) {
    this.cTip.next(tip);
    this.cTipPara.next('init');
  }

  setCTip(tip: string) {
    this.cTip.next(tip);
  }

  setCTPara(para: string) {
    this.cTipPara.next(para);
  }

  setCTAction(ac: string) {
    this.cAction.next(ac);
  }

  setCTTable(tableObj: Array<object>) {
    const jsObj = {
      table: tableObj
    };
    const jsString = JSON.stringify(jsObj);
    this.setCTPara(jsString);
  }

  setCTMoreTip(tipsList: Array<string>) {
    const jsObj = {
      tips: tipsList
    };
    const jsString = JSON.stringify(jsObj);
    this.setCTPara(jsString);
  }

  setCTProgress(ttlNumber: number) {
    const newTip = '{\"ttlProcess\":' + ttlNumber + '}';
    this.setCTPara(newTip);
  }

  updateCTProgress(progress: number) {
    const newTip = '{\"nowProcess\":' + progress + '}';
    this.setCTPara(newTip);
  }

  setCTClickTable(tableObj: Array<object>) {
    const jsObj = {
      clickTable: tableObj
    };
    const jsString = JSON.stringify(jsObj);
    this.setCTPara(jsString);
  }

}

export interface InitCenterTip {
  onYes: boolean;
  onNo: boolean;
}
