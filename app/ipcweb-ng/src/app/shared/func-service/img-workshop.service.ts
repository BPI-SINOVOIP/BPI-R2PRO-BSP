import { Injectable } from '@angular/core';
import { Subject } from 'rxjs';

import toBlob from 'blueimp-canvas-to-blob';
import Logger from 'src/app/logger';

@Injectable({
  providedIn: 'root'
})
export class ImgWorkshopService {

  constructor() { }

  private logger: Logger = new Logger('img-workshop');

  shapeJpeg = (picFile: any, order: number, sig: Subject<number>, quality?: Array<any>, fail?: Array<any>) => {
    this.logger.debug('input:' + picFile.name);
    const that = this;
    let reader = new FileReader();
    let image = new Image();
    reader.onload = (e) => {
      image.src = e.target.result.toString();
      image.onload = () => {
        let canvas = document.createElement('canvas');
        const setCell = this.sizeFormat(image.width, image.height);
        canvas.width = setCell.width,
        canvas.height = setCell.height;
        canvas.getContext('2d').drawImage(image, 0, 0, setCell.width, setCell.height);
        if (canvas.toBlob) {
          canvas.toBlob((rst) => {
            rst['name'] = this.getFileName(picFile.name) + '.jpg';
            if (quality) {
              quality.push(rst);
            }
            that.logger.debug(rst);
            sig.next(order + 1);
            image = null;
            reader = null;
            canvas = null;
          }, 'image/jpeg', 0.95);
        } else {
          const rst = toBlob(canvas.toDataURL('image/jpeg', 0.95));
          rst['name'] = this.getFileName(picFile.name) + '.jpeg';
          that.logger.debug(rst);
          if (quality) {
            quality.push(rst);
          }
          sig.next(order + 1);
          image = null;
          reader = null;
          canvas = null;
        }
      };
      image.onerror = (err) => {
        that.logger.debug(err, 'onerror:' + picFile.name + ':');
        if (fail) {
          fail.push({
            name: picFile.name,
            reason: 'fileError',
          });
        }
        image = null;
        reader = null;
        sig.next(order + 1);
      };
      image.onabort = (ab) => {
        that.logger.debug(ab, 'onabort:' + picFile.name + ':');
        if (fail) {
          fail.push({
            name: picFile.name,
            reason: 'fileAbort',
          });
        }
        image = null;
        reader = null;
        sig.next(order + 1);
      };
    };
    reader.readAsDataURL(picFile);
  }

  sizeFormat(w: number, h: number): SizeCell {
    const setCell: SizeCell = {
      height: h,
      width: w,
    };
    let isFormat: boolean = true;
    if (setCell.width > 480) {
      setCell.height = Math.ceil((setCell.height / setCell.width) * 240) * 2;
      setCell.width = 480;
      isFormat = false;
    }
    if (setCell.height > 640) {
      setCell.width = Math.ceil((setCell.width / setCell.height) * 320) * 2;
      setCell.height = 640;
      isFormat = false;
    }
    if (isFormat) {
      setCell.width = Math.ceil(w / 2) * 2;
      setCell.height = Math.ceil(h / 2) * 2;
    }
    return setCell;
  }

  getFileName(rawName: string) {
    for (let i = rawName.length - 1; i >= 0; i--) {
      if (rawName[i] === '.') {
        return rawName.slice(0, i);
      }
    }
  }

  getHW4IE = (picFile: any, order: number, imageReader: any, fileList: Array<any>, imageSub: Subject<number>) => {
    this.logger.debug('iein:' + picFile.name);
    const that = this;
    let reader = new FileReader();
    let image = new Image();
    reader.onload = (e) => {
      image.src = e.target.result.toString();
      image.onload = () => {
        this.logger.debug('ieonload:' + picFile.name);
        if (image.height > imageReader.height) {
          imageReader.failList.push({
            name: picFile.name,
            reason: imageReader.reason.height,
          });
        } else if (image.width > imageReader.width) {
          imageReader.failList.push({
            name: picFile.name,
            reason: imageReader.reason.width,
          });
        } else {
          fileList.push(picFile);
        }
        image = null;
        reader = null;
        imageSub.next(order + 1);
      };
      image.onerror = (err) => {
        this.logger.debug(err, 'ieonerror:' + picFile.name + ':');
        imageReader.failList.push({
          name: picFile.name,
          reason: imageReader.reason.error,
        });
        image = null;
        reader = null;
        imageSub.next(order + 1);
      };
      image.onabort = (ab) => {
        this.logger.debug(ab, 'ieonabort:' + picFile.name + ':');
        imageReader.failList.push({
          name: picFile.name,
          reason: imageReader.reason.abort,
        });
        image = null;
        reader = null;
        imageSub.next(order + 1);
      };
    };
    reader.readAsDataURL(picFile);
  }

  getHW = (picFile: any, imageReader: any, fileList: Array<any>, ES: any) => {
    this.logger.debug('in:' + picFile.name);
    ES.addTaskItem(imageReader.name);
    const that = this;
    let reader = new FileReader();
    let image = new Image();
    reader.onload = (e) => {
      image.src = e.target.result.toString();
      image.onload = () => {
        this.logger.debug('onload:' + picFile.name);
        if (image.height > imageReader.height) {
          imageReader.failList.push({
            name: picFile.name,
            reason: imageReader.reason.height,
          });
        } else if (image.width > imageReader.width) {
          imageReader.failList.push({
            name: picFile.name,
            reason: imageReader.reason.width,
          });
        } else {
          fileList.push(picFile);
        }
        ES.taskDone(imageReader.name);
        image = null;
        reader = null;
      };
      image.onerror = (err) => {
        this.logger.debug(err, 'onerror:' + picFile.name + ':');
        imageReader.failList.push({
          name: picFile.name,
          reason: imageReader.reason.error,
        });
        ES.taskDone(imageReader.name);
        image = null;
        reader = null;
      };
      image.onabort = (ab) => {
        this.logger.debug(ab, 'onabort:' + picFile.name + ':');
        imageReader.failList.push({
          name: picFile.name,
          reason: imageReader.reason.abort,
        });
        ES.taskDone(imageReader.name);
        image = null;
        reader = null;
      };
    };
    reader.readAsDataURL(picFile);
  }
}

export interface SizeCell {
  height: number;
  width: number;
}
