import { Injectable } from '@angular/core';
import { TranslateService } from '@ngx-translate/core';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';

@Injectable({
  providedIn: 'root'
})
export class Json2xmlService {

  constructor(
    public translate: TranslateService,
    private pfs: PublicFuncService,
  ) { }

  jsonBank = [];
  translateBank = {};
  currentLang: string;
  valBank = {};

  employee = {
    waitDoneTranslate: 0,
    jsonDone: 0,
    reset: () => {
      const itemList = ['waitDoneTranslate', 'jsonDone'];
      for (const item of itemList) {
        if (this.employee[item] < 0 ) {
          this.employee[item] = 0;
        }
      }
    },
  };

  resetService() {
    this.jsonBank = [];
    this.employee.reset();
  }

  initService() {
    this.jsonBank = [];
    this.currentLang = localStorage.getItem('currentLanguage');
    if (!this.translateBank[this.currentLang]) {
      this.translateBank[this.currentLang] = {};
    }
    this.translateTask('info');
  }

  preAddTranslate(info: object) {
    this.employee.reset();
    this.currentLang = localStorage.getItem('currentLanguage');
    if (!this.translateBank[this.currentLang]) {
      this.translateBank[this.currentLang] = {};
    }
    this.add4TranslateBank(info);
  }

  addObjectVal(keyName: string, relativeForm: object) {
    if (relativeForm instanceof Object) {
      this.valBank[keyName] = relativeForm;
      for (const key of this.pfs.objectKeys(relativeForm)) {
        this.translateTask(relativeForm[key]);
      }
    }
  }

  addArrayVal(relativeForm: Array<string>) {
    if (relativeForm instanceof Array) {
      for (const item of relativeForm) {
        this.translateTask(item);
      }
    }
  }

  push(info: Array<object>|object) {
    this.employee.jsonDone += 1;
    this.waitEmployeeDone('waitDoneTranslate')
      .then(
        () => {
          this.pushFunc(info);
          this.employee.jsonDone -= 1;
        }
      )
      .catch(
        () => {
          console.log('err');
          this.pushFunc(info);
          this.employee.jsonDone -= 1;
        }
      );
  }

  pushFunc(info: Array<object>|object) {
    if (info instanceof Array) {
      for (const pat of info) {
        this.jsonBank.push(this.pushInfo(pat));
      }
    } else {
      this.jsonBank.push(this.pushInfo(info));
    }
  }

  pushInfo(info: any) {
    if (info instanceof Object) {
      const rst = {};
      for (const key of this.pfs.objectKeys(info)) {
        if (this.valBank[key]) {
          this.valBank[key][info[key]] ? info[key] = this.valBank[key][info[key]] : null;
        }
        if (this.translateBank[this.currentLang][key]) {
          rst[this.translateBank[this.currentLang][key]] = this.pushInfo(info[key]);
        } else {
          rst[key] = this.pushInfo(info[key]);
        }
      }
      return rst;
    } else if (typeof(info) === 'string') {
      if (this.translateBank[this.currentLang][info]) {
        return this.translateBank[this.currentLang][info];
      } else {
        return info;
      }
    } else {
      return info;
    }
  }

  add4TranslateBank(info: any) {
    if (info instanceof Object) {
      for (const key of this.pfs.objectKeys(info)) {
        this.translateTask(key);
        this.add4TranslateBank(info[key]);
      }
    }
  }

  translateTask(key: string) {
    if (!this.translateBank[this.currentLang][key]) {
      this.employee.waitDoneTranslate += 1;
      this.addTranslate(key);
    }
  }

  // translateKey(pat: any) {
  //   if (pat instanceof Object) {
  //     const rst = {};
  //     for (const key of this.pfs.objectKeys(pat)) {
  //       if (this.translateBank[this.currentLang][key]) {
  //         rst[this.translateBank[this.currentLang][key]] = this.translateKey(pat[key]);
  //       } else {
  //         this.addTranslate(String(key));
  //         this.waitTranslate(5000)
  //           .then(() => {
  //             rst[this.translateBank[this.currentLang][key]] = this.translateKey(pat[key]);
  //           })
  //           .catch(() => {
  //             rst[key] = this.translateKey(pat[key]);
  //           });
  //       }
  //     }
  //     return rst;
  //   } else {
  //     return pat;
  //   }
  // }

  addTranslate(key: string) {
    this.translate.get(key).subscribe(
      rst => {
        this.translateBank[this.currentLang][key] = rst;
        this.employee.waitDoneTranslate -= 1;
      }
    );
  }

  waitEmployeeDone = (watiName: string) => new Promise(
    (resolve, reject) => {
      const waitDone = () => {
        if (this.employee[watiName] <= 0) {
          resolve();
        } else {
          setTimeout(waitDone, 100);
        }
      };
      waitDone();
    }
  )

  // waitTranslate = (timeoutms: number) => new Promise(
  //   (resolve, reject) => {
  //     const waitDone = () => {
  //       timeoutms -= 100;
  //       if (this.translateDone) {
  //         resolve();
  //       } else if (timeoutms <= 0) {
  //         reject();
  //       } else {
  //         setTimeout(waitDone, 100);
  //       }
  //     };
  //     waitDone();
  //   }
  // )

  json2xml(fileName: string) {
    this.waitEmployeeDone('jsonDone')
      .then(() => {
        const rstJson = {};
        if (this.translateBank[this.currentLang]['info']) {
          rstJson[this.translateBank[this.currentLang]['info']] = this.jsonBank;
        } else {
          rstJson['info'] = this.jsonBank;
        }
        const builder = require('xmlbuilder');
        const xmlOut = builder.create(rstJson).end({ pretty: true});
        const rst = xmlOut.toString();
        this.funDownload(rst, fileName);
      });
  }

  funDownload(content: string, filename: string) {
    const eleLink = document.createElement('a');
    eleLink.download = filename;
    eleLink.style.display = 'none';
    const blob = new Blob([content]);
    eleLink.href = URL.createObjectURL(blob);
    document.body.appendChild(eleLink);
    eleLink.click();
    document.body.removeChild(eleLink);
    this.jsonBank = [];
  }

}
