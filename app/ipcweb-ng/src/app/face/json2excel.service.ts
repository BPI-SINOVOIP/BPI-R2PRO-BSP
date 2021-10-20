import { Injectable, OnDestroy } from '@angular/core';
import { TranslateService } from '@ngx-translate/core';
import { PublicFuncService  } from 'src/app/shared/func-service/public-func.service';
import { IeCssService  } from 'src/app/shared/func-service/ie-css.service';
import * as XLSX from 'xlsx';
import * as FileSaver from 'file-saver';

/// <reference types="windows-script-host" />
/// <reference types="activex-msforms" />
/// <reference types="activex-scripting" />

declare var ActiveXObject: (type: string) => void;

@Injectable({
  providedIn: 'root'
})
export class Json2excelService implements OnDestroy {

  constructor(
    public translate: TranslateService,
    private pfs: PublicFuncService,
    private ics: IeCssService,
  ) { }

  private jsonBank = [];
  private translateBank = {};
  private currentLang: string;
  private valBank = {};

  private colBank = {};
  private titleList = [];
  private translatedTitle = [];

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

  ngOnDestroy() {
    this.jsonBank = [];
    this.titleList = [];
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

  set4Column(colName: string, data: any) {
    this.titleList = [];
    this.currentLang = localStorage.getItem('currentLanguage');
    if (!this.translateBank[this.currentLang]) {
      this.translateBank[this.currentLang] = {};
    }
    this.colBank[colName] = data;
    for (const item of this.colBank[colName]) {
      this.titleList.push(item['title']);
    }
    this.addArrayVal(this.titleList);
    this.translatedTitle = [];
  }

  push(colName: string, data: Array<object>) {
    this.employee.jsonDone += 1;
    if (this.employee['waitDoneTranslate'] <= 0) {
      if (this.translatedTitle.length === 0) {
        for (const item of this.titleList) {
          this.translatedTitle.push(this.translateFunc(item));
          if (this.jsonBank.length === 0) {
            this.jsonBank.push(this.translatedTitle);
          }
        }
      }
      this.pushData(colName, data);
    } else {
      this.waitEmployeeDone('waitDoneTranslate')
      .then(
        () => {
          if (this.translatedTitle.length === 0) {
            for (const item of this.titleList) {
              this.translatedTitle.push(this.translateFunc(item));
              if (this.jsonBank.length === 0) {
                this.jsonBank.push(this.translatedTitle);
              }
            }
          }
          this.pushData(colName, data);
        }
      )
      .catch(
        () => {
          this.pushData(colName, data);
        }
      );
    }
  }

  pushData(colName: string, data: Array<object>) {
    // translate title
    for (const item of this.colBank[colName]) {
      if (this.translateBank[this.currentLang][item.title]) {
        item.title = this.translateBank[this.currentLang][item.title];
      }
    }
    // push data in titleList
    for (const item of data) {
      const pat = [];
      for (const title of this.titleList) {
        pat.push(this.translateFunc(this.relativeTransform(title, item[title])));
      }
      this.jsonBank.push(pat);
    }
    this.employee.jsonDone -= 1;
  }

  pushData4Dict(colName: string, data: Array<object>) {
    // translate title
    for (const item of this.colBank[colName]) {
      if (this.translateBank[this.currentLang][item.title]) {
        item.title = this.translateBank[this.currentLang][item.title];
      }
    }
    // push data in titleList
    for (const item of data) {
      const pat = {};
      for (const title of this.titleList) {
        pat[title] = this.translateFunc(this.relativeTransform(title, item[title]));
      }
      this.jsonBank.push(pat);
    }
    this.employee.jsonDone -= 1;
  }

  relativeTransform(title: string, raw: string): string {
    if (this.valBank[title]) {
      return this.valBank[title][raw] ? this.valBank[title][raw] : raw;
    } else {
      return raw;
    }
  }

  translateFunc(raw: string) {
    return this.translateBank[this.currentLang][raw] ? this.translateBank[this.currentLang][raw] : raw;
  }

  getExcel(fileName: string) {
    console.log('getExcel');
    this.waitEmployeeDone('jsonDone')
      .then(
        () => {
          this.getExcelFunc(fileName);
        }
      )
      .catch(
        () => {
          this.getExcelFunc(fileName);
        }
      );
  }

  getExcelFunc(fileName: string) {
    this.exportTable(fileName);
    this.jsonBank = [];
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

  addTranslate(key: string) {
    this.translate.get(key).subscribe(
      rst => {
        this.translateBank[this.currentLang][key] = rst;
        this.employee.waitDoneTranslate -= 1;
      }
    );
  }

  exportTable(fileName: string) {
    const exportItem = this.jsonBank;

    const noHeader = {
      skipHeader: true
    };
    const worksheet: XLSX.WorkSheet = XLSX.utils.json_to_sheet(exportItem, noHeader);
    const workbook: XLSX.WorkBook = { Sheets: { 'data': worksheet }, SheetNames: ['data'] };
    const excelBuffer: any = XLSX.write(workbook, { bookType: 'xlsx', type: 'array' });
    this.saveAsExcelFile(excelBuffer, fileName);
  }

  private saveAsExcelFile(buffer: any, fileName: string) {
    const data: Blob = new Blob([buffer], {
      type: 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet;charset=UTF-8'
    });
    FileSaver.saveAs(data, fileName + '.xlsx');
  }
}
