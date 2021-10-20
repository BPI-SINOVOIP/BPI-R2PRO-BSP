import { Injectable, ElementRef } from '@angular/core';
import { Subject } from 'rxjs';
import { observeOn } from 'rxjs/operators';

@Injectable({
  providedIn: 'root'
})
export class PublicFuncService {

  constructor() { }

  checkFileType(name: string, types: Array<string>) {
    let type = '';
    for (let i = name.length - 1; i >= 0; i--) {
      if (name[i] === '.') {
        type = name.slice(i + 1);
        break;
      }
    }
    for (const item of types) {
      if (item === type) {
        return true;
      }
    }
    return false;
  }

  getFileType(name: string) {
    let type = '';
    for (let i = name.length - 1; i >= 0; i--) {
      if (name[i] === '.') {
        type = name.slice(i + 1);
        return type;
      }
    }
  }

  checkBmp24(picFile: any) {
    const rst: Subject<string> = new Subject<string>();
    const reader = new FileReader();
    const that = this;
    reader.onloadend = (e) => {
      const buf = e.target.result;
      if (buf instanceof ArrayBuffer) {
        const binBuf = new DataView(buf);
        if (binBuf.getInt8(28) === 24) {
          that.checkImgResolution(picFile, rst, 16, 16);
        } else {
          rst.next('not24BMP');
        }
      } else {
        rst.next('fileError');
      }
    };
    reader.readAsArrayBuffer(picFile);
    return rst;
  }

  checkImgResolution(picFile: any, rst: Subject<string>, h: number, w: number) {
    const reader = new FileReader();
    const image = new Image();
    reader.onloadend = (e) => {
      image.src = e.target.result.toString();
      image.onload = () => {
        if (image.height % h === 0 && image.width % w === 0) {
          rst.next('ok');
        } else {
          if (h === w) {
            rst.next('resolutionRatioWrong' + h);
          } else {
            rst.next('resolutionRatioWrong');
          }
        }
      };
    };
    reader.readAsDataURL(picFile);
  }

  translateFormItem(rawWord: string) {
    const start = rawWord.slice(1, 2);
    const end = rawWord.slice(2);
    return start.toLowerCase() + end;
  }

  formatInt(formatInfo: any) {
    const pat = /^i.*?/;
    if (formatInfo instanceof Array) {
      // tslint:disable-next-line: forin
      for (const index in formatInfo) {
        formatInfo[index] = this.formatInt(formatInfo[index]);
      }
    } else if (formatInfo instanceof Object) {
      for (let part of this.objectKeys(formatInfo)) {
        if (formatInfo[part] instanceof Object) {
          formatInfo[part] = this.formatInt(formatInfo[part]);
        } else if (pat.test(part)) {
          formatInfo[part] = this.formatInt(formatInfo[part]);
        }
      }
    } else {
      formatInfo = Number(formatInfo);
    }
    return formatInfo;
  }

  waitNavActive = (timeoutms: number, navId: string, gap: number = 100, className = 'nav-link active') => new Promise(
    (resolve, reject) => {
      const waitFunc = () => {
        timeoutms -= gap;
        let navList =  document.getElementsByClassName(className);
        if (navList) {
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
          setTimeout(waitFunc, gap);
        }
      };
      waitFunc();
    }
  )

  checkNacActive(navId: string) {
    let navList =  document.getElementsByClassName('nav-link active');
    // tslint:disable-next-line: prefer-for-of
    for (let i = 0; i < navList.length; i++) {
      if (navList[i].id === navId) {
        navList = null;
        return true;
      }
    }
    navList = null;
    return false;
  }

  hex2DecimalOne(ch: string | number) {
    const hexDict = {
      0: '0000',
      1: '0001',
      2: '0010',
      3: '0011',
      4: '0100',
      5: '0101',
      6: '0110',
      7: '0111',
      8: '1000',
      9: '1001',
      a: '1010',
      b: '1011',
      c: '1100',
      d: '1101',
      e: '1110',
      f: '1111',
    };
    return hexDict[ch];
  }

  decimal2Hex4(ch: string | number) {
    const toHexDict = {
      '0000': '0',
      '0001': '1',
      '0010': '2',
      '0011': '3',
      '0100': '4',
      '0101': '5',
      '0110': '6',
      '0111': '7',
      '1000': '8',
      '1001': '9',
      '1010': 'a',
      '1011': 'b',
      '1100': 'c',
      '1101': 'd',
      '1110': 'e',
      '1111': 'f',
      '': ''
    };
    return toHexDict[ch];
  }

  formatTime(dateTime: string) {
    const today = new Date(dateTime);
    const year = today.getFullYear().toString();
    let month = '';
    let day = '';
    let hour = '';
    let minute = '';
    if (today.getMonth() <= 8) {
      month =  '0' + (today.getMonth() + 1);
    } else {
      month = (today.getMonth() + 1).toString();
    }
    if (today.getDate() <= 9) {
      day = '0' + (today.getDate()).toString();
    } else {
      day = today.getDate().toString();
    }
    if (today.getHours() <= 9) {
      hour = '0' + today.getHours();
    } else {
      hour = today.getHours().toString();
    }
    if (today.getMinutes() <= 9) {
      minute = '0' + today.getMinutes();
    } else {
      minute = today.getMinutes().toString();
    }
    return year + '-' + month + '-' + day + 'T' + hour + ':' + minute + ':00';
  }

  string2Number(rawData: string | number): number {
    if (typeof(rawData) === 'string') {
      const val = rawData;
      if (val.match('/')) {
        const dataArray = val.split('/');
        if (dataArray.length === 2) {
          if (this.isPureNumber(dataArray[0]) && this.isPureNumber(dataArray[1])) {
            const num0 = Number(dataArray[0]);
            const num1 = Number(dataArray[1]);
            if (num1 !== 0) {
              return num0 / num1;
            }
          }
        }
      }
      return Number(val);
    } else {
      return rawData;
    }
  }

  isPureNumber(rawData: string): boolean {
    const val = rawData;
    const rst = Number(val);
    return !isNaN(rst);
  }

  getBase64(url: string, fileType: string, callback: Function) {
    const Img = new Image();
    let dataURL = '';
    Img.src = url;
    Img.setAttribute('crossOrigin', 'Anonymous');
    Img.onload = () => {
        const canvas = document.createElement('canvas');
        canvas.width = Img.width,
        canvas.height = Img.height;
        canvas.getContext('2d').drawImage(Img, 0, 0, Img.width, Img.height);
        dataURL = canvas.toDataURL('image/' + fileType, 1.0);
        callback ? callback(dataURL) : null;
    };
  }

  canvasSnapVediojs(video: HTMLVideoElement, size: any) {
    const canvas = document.createElement('canvas');
    canvas.width = size.width,
    canvas.height = size.height;
    console.log(video.width, video.height);
    canvas.getContext('2d').drawImage(video, 0, 0, size.width, size.height);
    // canvas.getContext('2d').drawImage(video, 0, 0, size.width, size.height, 0, 0, size.width, size.height);
    const dataURL = canvas.toDataURL('image/jpeg', 1.0);
    return dataURL;
  }

  // for dbtool SystemPara
  formName2translateName(formName: string): string {
    let translateName = '';
    const pat = /^[i,s]+/;
    if (pat.test(formName)) {
      translateName = formName.substr(1);
      const pat2 = /^[A-Z]{2}/;
      if (pat2.test(translateName)) {
        return translateName;
      } else {
        translateName = translateName.replace(translateName[0], translateName[0].toLowerCase());
        return translateName;
      }
    } else {
      return formName;
    }
  }

  checkMenuType(item: any, checkType: string): boolean {
    if (!item || !item['type']) {
      return false;
    }
    const type = item['type'];
    if (type === 'range') {
      if (item['range']['step']) {
        return type === checkType;
      } else {
        return checkType === 'number';
      }
    }
    return type === checkType;
  }

  getRange(item: any, key: string) {
    return item['range'][key];
  }

  isInArrayString(list: Array<string>, key: string): boolean {
    for (const item of list) {
      if (item === key) {
        return true;
      }
    }
    return false;
  }

  cycleGet(data: any, refer: Array<any>, start: number) {
    const len = refer.length;
    if (start >= len) {
      return data;
    } else {
      data = data[refer[start]];
      start += 1;
    }
    return this.cycleGet(data, refer, start);
  }
  // for dbtool SystemPara end

  usingDbJson(res: any) {
    if (typeof(res) === 'string') {
      return res;
    } else if (res instanceof Object) {
      if (res['para']) {
        return res['para']
      }
    }
    return res;
  }

  getNameFromPath(path: string) {
    if (path.match('/')) {
      for (let i = path.length - 1; i >= 0; i--) {
        if (path[i] === '/') {
          return path.slice(i + 1);
        }
      }
    }
    return path;
  }

  waitAInit = (timeoutms: number, className: string, el: ElementRef) => new Promise(
    (resolve, reject) => {
      const waitFunc = () => {
        timeoutms -= 100;
        let targetItem =  el.nativeElement.querySelector(className);
        if (targetItem) {
          resolve();
        } else if (timeoutms < 0) {
          reject();
        } else {
          setTimeout(waitFunc, 100);
        }
      };
      waitFunc();
    }
  )

  getFileName(rawName: string) {
    for (let i = rawName.length - 1; i >= 0; i--) {
      if (rawName[i] === '.') {
        return rawName.slice(0, i);
      }
    }
  }

  objectKeys = (data: {}) => {
    if (Object.keys) {
      return Object.keys(data);
    } else {
      if (data !== Object(data)) {
        throw new TypeError('objectKeys called on a non-object');
      }
      const rst = [];
      for (const p in data) {
        if (Object.prototype.hasOwnProperty.call(data, p)) {
          rst.push(p);
        }
      }
      return rst;
    }
  }

}
