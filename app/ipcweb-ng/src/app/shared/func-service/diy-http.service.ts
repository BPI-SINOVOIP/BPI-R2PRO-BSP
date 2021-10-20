import { Injectable } from '@angular/core';
import { Subject } from 'rxjs';
import { ConfigService } from 'src/app/config.service';
import { PublicFuncService } from './public-func.service';
import { StreamURLInterface } from 'src/app/preview/StreamURLInterface';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import Logger from 'src/app/logger';

@Injectable({
  providedIn: 'root'
})
export class DiyHttpService {

  constructor(
    private cfg: ConfigService,
    private pfs: PublicFuncService,
    private resError: ResponseErrorService,
  ) { }
  private logger: Logger = new Logger('diy-http');

  diyHttp(httpInfo: HttpInfo) {
    const progressBar: Subject<number> = new Subject<number>();
    this.sliceAndPut(httpInfo, progressBar);
    return progressBar;
  }

  sliceAndPut(httpInfo: HttpInfo, progressBar: Subject<number>) {
    if (httpInfo.cursorSize < httpInfo.targetSize) {
      if (httpInfo.cursorSize + httpInfo.perSize >= httpInfo.targetSize) {
        httpInfo.blobFile = httpInfo.file.slice(
          httpInfo.cursorSize, httpInfo.targetSize, httpInfo.type);
        httpInfo.upgradeMsg = 'bytes ' + httpInfo.cursorSize + '-' + (httpInfo.targetSize - 1);
        this.cfg[httpInfo.httpFunc](httpInfo.upgradeId, httpInfo.blobFile, httpInfo.upgradeMsg).subscribe(
          (res: any) => {
            this.updateUpgradeCursor(res, httpInfo, progressBar);
            this.sliceAndPut(httpInfo, progressBar);
          },
          (err: any) => {
            this.logger.error(err, 'diyHTTP-ssliceAndPut:');
            this.getUpgradeProcessing(httpInfo, progressBar);
          }
        );
      } else {
        httpInfo.blobFile = httpInfo.file.slice(
          httpInfo.cursorSize, httpInfo.cursorSize + httpInfo.perSize, httpInfo.type);
        httpInfo.upgradeMsg = 'bytes ' + httpInfo.cursorSize + '-' +
          (httpInfo.cursorSize + httpInfo.perSize - 1);
        this.cfg[httpInfo.httpFunc](httpInfo.upgradeId, httpInfo.blobFile, httpInfo.upgradeMsg).subscribe(
          (res: any) => {
            this.updateUpgradeCursor(res, httpInfo, progressBar);
            this.sliceAndPut(httpInfo, progressBar);
          },
          (err: any) => {
            this.logger.error(err, 'diyHTTP-ssliceAndPut:');
            this.getUpgradeProcessing(httpInfo, progressBar);
          }
        );
      }
    } else {
      httpInfo.file = null;
      progressBar.next(101);
    }
  }

  updateUpgradeCursor(res: any, httpInfo: any, progressBar: Subject<number>) {
    httpInfo.cursorSize = Number(httpInfo.processPat.exec(res.range)[1]) + 1;
    progressBar.next(Math.ceil(httpInfo.cursorSize / httpInfo.targetSize * 100));
  }

  getUpgradeProcessing(httpInfo: any, progressBar: Subject<number>) {
    this.cfg[httpInfo.processFunc](httpInfo.upgradeId, null, '').subscribe(
      (res: any) => {
        this.updateUpgradeCursor(res, httpInfo, progressBar);
        this.sliceAndPut(httpInfo, progressBar);
      },
      (err: any) => {
        this.logger.error(err, 'diyHTTP-getUpgradeProcessing:');
        progressBar.next(-1);
      }
    );
  }

  downloadFunc(url: string, name: string) {
    if (!(name)) {
      const len = url.length;
      for (let i = len - 1; i >= 0; i--) {
        if (url[i] === '/') {
          name = url.slice(i + 1, len);
          break;
        }
      }
    }
    const a = document.createElement('a');
    const event = new MouseEvent('click');
    a.download = name;
    a.href = url;
    a.dispatchEvent(event);
  }

  downloadVideojsSnap(video: any, size: any, isIe: boolean) {
    const url = this.pfs.canvasSnapVediojs(video, size);
    const fileName = 'snapshot' + new Date().getTime() + '.jpeg';
    if (isIe) {
      this.downloadPic4IEFunc(fileName, url);
    } else {
      this.aDownloadFunc(fileName, url);
    }
  }

  downloadPic4Chrome(fileName: string, url: string) {
    const fileType = this.pfs.getFileType(fileName);
    const that = this;
    this.pfs.getBase64(url, fileType, (_baseUrl) => {
      that.aDownloadFunc(fileName, _baseUrl);
    });
  }

  aDownloadFunc(fileName: string, url: string) {
    const a = document.createElement('a');
    const event = new MouseEvent('click');
    a.download = fileName;
    a.href = url;
    a.style.display = 'none';
    document.body.appendChild(a);
    a.dispatchEvent(event);
    document.body.removeChild(a);
  }

  downloadPic4IE(url: string, fileName: string) {
    const fileType = this.pfs.getFileType(fileName);
    const that = this;
    this.pfs.getBase64(url, fileType, (_baseUrl) => {
      that.downloadPic4IEFunc(fileName, _baseUrl);
    });
  }

  downloadPic4IEFunc = (fileName: string, url: string) => {
    if (window.navigator.msSaveOrOpenBlob) {
      const bstr = atob(url.split(',')[1]);
      let n = bstr.length;
      const u8arr = new Uint8Array(n);
      while (n--) {
       u8arr[n] = bstr.charCodeAt(n);
      }
      const blob = new Blob([u8arr]);
      window.navigator.msSaveOrOpenBlob(blob, fileName);
    }
  }

  getReferParaOption(refer: any) {
    const optionBar: Subject<string> = new Subject<string>();
    if (refer['refer']) {
      const referList = refer['refer'];
      this.cfg.getDefaultPara(refer['refer'][0]).subscribe(
        res => {
          const js = JSON.parse(res.toString());
          const data = this.pfs.cycleGet(js, referList, 2);
          if (data['type'] === 'options') {
            const dataString = JSON.stringify(data['options']);
            optionBar.next(dataString);
          }
        },
        error => {
          this.logger.error(error, 'getReferParaOption:');
        }
      );
    }
    return optionBar;
  }

  /* to use getStreamUrl()
  @code
  this.urlOb = this.dhs.getStreamUrl().subscribe(
    (msg: string) => {
      if (msg) {
        this.src = msg;
        this.playEntry();
      } else {
        this.tips.setRbTip('getVideoUrlFail');
      }
      this.urlOb.unsubscribe();
      this.urlOb = null;
    }
  );
  @codeend
  */
  getStreamUrl(streamName: string = 'substream') {
    const urlWaiter: Subject<string> = new Subject<string>();
    this.cfg.getStreamURLInterface().subscribe(
      (res: StreamURLInterface[]) => {
        this.resError.analyseRes(res);
        let url = ''
        for (const item of res) {
          if (item.sStreamProtocol === 'HTTP' && item.sURL.match(streamName)) {
            url = item.sURL;
            break;
          }
        }
        urlWaiter.next(url);
      },
      err => {
        urlWaiter.next('');
      }
    );
    return urlWaiter.asObservable();
  }
}

export interface HttpInfo {
  targetSize: number;
  cursorSize: number;
  perSize: number;
  file: any;
  type: any;
  blobFile: any;
  upgradeId: string;
  processPat: any;
  upgradeMsg: string;
  httpFunc: string;
  processFunc: string;
}
