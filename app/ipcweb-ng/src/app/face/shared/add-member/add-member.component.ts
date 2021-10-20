import { Component, OnInit, ViewChild, ElementRef, Renderer2,
  OnDestroy, AfterViewInit, ViewChildren, ChangeDetectorRef } from '@angular/core';
import { FormBuilder, Validators, FormControl } from '@angular/forms';

import { ConfigService } from 'src/app/config.service';
import { MemberListInterface } from '../list-management/ListMangementInterface';
import { AddMemberInterface } from './AddMemberInterface';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { TipsService } from 'src/app/tips/tips.service';
import { isNumberJudge } from 'src/app/shared/validators/benumber.directive';
import { ImgWorkshopService } from 'src/app/shared/func-service/img-workshop.service';
import { Subject } from 'rxjs';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-add-member',
  templateUrl: './add-member.component.html',
  styleUrls: ['./add-member.component.scss']
})
export class AddMemberComponent implements OnInit, AfterViewInit, OnDestroy {

  @ViewChild('pic', {static: true}) picChild: ElementRef<HTMLCanvasElement>;
  @ViewChild('img', {read: ElementRef}) imgView: ElementRef;

  constructor(
    private fb: FormBuilder,
    private cfg: ConfigService,
    private el: ElementRef,
    private Re2: Renderer2,
    private resError: ResponseErrorService,
    private pfs: PublicFuncService,
    private ieCss: IeCssService,
    private tips: TipsService,
    private iws: ImgWorkshopService,
    private cdr: ChangeDetectorRef,
  ) { }

  conditionType: Array<string> = [
    'whiteList',
    'blackList',
  ];

  genderType: Array<string> = [
    'male',
    'female',
  ];

  nationList: Array<string> = ['汉族', '满族', '蒙古族', '回族', '藏族', '维吾尔族', '苗族', '彝族', '壮族', '布依族', '侗族', '瑶族', '白族',
    '土家族', '哈尼族', '哈萨克族', '傣族', '黎族', '傈僳族', '佤族', '畲族', '高山族', '拉祜族', '水族', '东乡族', '纳西族', '景颇族', '柯尔克孜族',
    '土族', '达斡尔族', '仫佬族', '羌族', '布朗族', '撒拉族', '毛南族', '仡佬族', '锡伯族', '阿昌族', '普米族', '朝鲜族', '塔吉克族', '怒族',
    '乌孜别克族', '俄罗斯族', '鄂温克族', '德昂族', '保安族', '裕固族', '京族', '塔塔尔族', '独龙族', '鄂伦春族', '赫哲族', '门巴族', '珞巴族', '基诺族'];

  certiType: Array<string> = [
    'identityCard'
  ];

  listType: Array<string> = [
    'permanent',
  ];

  private logger: Logger = new Logger('addMember');

  title: string = 'test';
  private ctx: any;
  picFile: any;
  private idNum: string = '';
  private isListen: boolean = true;
  private isPutAddress: boolean = false;
  private snapPath: string;
  needInit: boolean = false;
  private afterSaveFunc: () => {};
  private isRefeed: boolean = false;
  private refeedObj: any;
  private defaultFileName  = 'noFileSelectedWithLimit';
  fileName: string = 'noFileSelectedWithLimit';
  private limitSize: number = 1024 * 1024;
  private fileTypeLimit: Array<string> = ['jpg', 'JPG'];
  private defaultTypeLimitTip: string = 'fileTypeShouldBe';
  typeLimit: string = '';
  beforeTip: string = '';
  isIe: boolean = true;
  isImgError: boolean = false;

  AddForm = this.fb.group({
    iAccessCardNumber: [0, isNumberJudge],
    sTelephoneNumber: [''],
    sAddress: [''],
    sBirthday: ['1970-01-01', Validators.required],
    sCertificateNumber: [''],
    sCertificateType: ['identityCard'],
    sGender: ['male'],
    sHometown: [''],
    sListType: ['permanent'],
    sName: [''],
    sNation: ['汉族'],
    sNote: [''],
    sType: ['whiteList'],
  });

  fakeData: AddMemberInterface = {
    iAccessCardNumber: 0,
    sTelephoneNumber: '',
    sAddress: '',
    sBirthday: '1970-01-01',
    sCertificateNumber: '',
    sCertificateType: 'identityCard',
    sGender: 'male',
    sHometown: '',
    sListType: 'permanent',
    sName: '',
    sNation: '汉族',
    sNote: 'undone',
    sType: 'whiteList',
  };

  get iAccessCardNumber(): FormControl {
    return this.AddForm.get('iAccessCardNumber') as FormControl;
  }

  ngOnInit(): void {
    this.isIe = this.ieCss.getIEBool();
    this.Re2.setAttribute(this.el.nativeElement.querySelector('.content.disabled-item'), 'disabled', 'true');
    this.typeLimit = this.fileTypeLimit.join(',');
  }

  ngAfterViewInit() {
    this.ctx = this.picChild.nativeElement.getContext('2d');
    this.picChild.nativeElement.addEventListener('mousedown', (e: any) => {
      if (this.isListen) {
        const event = new MouseEvent('click');
        const inputBtn = document.getElementById('upload-avator');
        inputBtn.dispatchEvent(event);
      }
    });
  }

  ngOnDestroy() {
  }

  onSubmit() {
    this.showWaitTip('waitForUploading');
    if (!this.needInit) {
      this.idNum = '';
    }
    const that = this;
    this.pfs.formatInt(this.AddForm.value);
    // tslint:disable-next-line: forin
    for (const key in this.AddForm.value) {
      this.fakeData[key] = this.AddForm.value[key];
    }
    if (this.picFile) {
      this.fakeData['iFaceDBId'] = -2;
    } else if (this.fakeData['iFaceDBId']) {
      delete this.fakeData.iFaceDBId;
    }
    this.fakeData.sNote = 'undone';
    this.cfg.putAddMemberInterface(this.fakeData, this.idNum).subscribe(
      res => {
        this.resError.analyseRes(res);
        that.idNum = res.id.toString();
        if (this.isPutAddress && res['sPicturePath'] && !this.picFile) {
          this.putImageUrl(res['sPicturePath']);
        } else {
          if (this.picFile && res['sPicturePath']) {
            this.putImage(res['sPicturePath'].toString());
          } else {
            this.cfg.putAddMemberInterface(this.AddForm.value, this.idNum).subscribe(
              dat => {
                if (this.isRefeed) {
                  // tslint:disable-next-line: forin
                  for (const key in dat) {
                    this.refeedObj[key] = dat[key];
                  }
                }
                if (this.afterSaveFunc) {
                  this.afterSaveFunc();
                }
                this.endWaitTip('modifySuccessful');
              },
              err => {
                this.logger.error(err, 'onSubmit:putAddMemberInterface:putAddMemberInterface:');
                this.whenUploadFail();
              }
            );
          }
        }
      },
      err => {
        this.whenUploadFail();
        this.logger.error(err, 'onSubmit:putAddMemberInterface:');
      }
    );
    this.onNo();
  }

  putImage(path: string) {
    path = encodeURI(path);
    this.cfg.putAddMemberImage(this.picFile, path).subscribe(
      res => {
        this.cfg.putAddMemberInterface(this.AddForm.value, this.idNum).subscribe(
          dat => {
            if (this.isRefeed) {
              // tslint:disable-next-line: forin
              for (const key in dat) {
                this.refeedObj[key] = dat[key];
              }
            }
            if (this.afterSaveFunc) {
              this.afterSaveFunc();
            }
            this.endWaitTip('uploadSuccess');
          },
          err => {
            this.logger.error(err, 'putImage:putAddMemberImage:putAddMemberInterface:');
            this.whenUploadFail();
          }
        );
      },
      err => {
        this.whenUploadFail();
        this.logger.error(err, 'putImage:putAddMemberImage:');
      }
    );
  }

  putImageUrl(path: string) {
    const pat = /\/\/.*?\/(.*?)\/(.*?)$/;
    const pathList = pat.exec(this.snapPath);
    path = encodeURI(path);
    this.cfg.putSnapImg(path, {path: encodeURI('/' + pathList[1] + '/' + pathList[2])}).subscribe(
      res => {
        this.cfg.putAddMemberInterface(this.AddForm.value, this.idNum).subscribe(
          dat => {
            if (this.isRefeed) {
              // tslint:disable-next-line: forin
              for (const key in dat) {
                this.refeedObj[key] = dat[key];
              }
            }
            if (this.afterSaveFunc) {
              this.afterSaveFunc();
            }
            this.endWaitTip('modifySuccessful');
          },
          err => {
            this.logger.error(err, 'putImageUrl:putSnapImg:putAddMemberInterface:');
            this.whenUploadFail();
          }
        );
      },
      err => {
        this.whenUploadFail();
        this.logger.error(err, 'putImageUrl:putSnapImg:');
      }
    );
  }

  onShow(needInit: boolean = false, id: any = '', data?: MemberListInterface) {
    this.needInit = needInit;
    this.clear();
    this.isImgError = false;
    if (needInit) {
      this.idNum = id.toString();
      this.AddForm.patchValue(data);
      const img = new Image();
      const that = this;
      img.onerror = (err) => {
        that.isImgError = true;
      };
      img.onabort = (ab) => {
        that.isImgError = true;
      };
      this.snapPath = data.sPicturePath;
      img.src = data.sPicturePath;
      this.ctx.clearRect(0, 0, 200, 200);
      const height = img.height;
      const width = img.width;
      let drawHeight = 160;
      let drawWidth = 160;
      if (height > width) {
        drawWidth = Math.ceil(width * drawHeight / height);
      } else {
        drawHeight = Math.ceil(height * drawWidth / width);
      }
      this.ctx.drawImage(img, 10, 10, drawWidth, drawHeight);
      if (id === '') {
        this.Re2.setStyle(this.el.nativeElement.querySelector('.img-li'), 'display', 'none');
      } else {
        this.Re2.setStyle(this.el.nativeElement.querySelector('.img-li'), 'display', 'block');
      }
    }
    this.Re2.setStyle(this.el.nativeElement.querySelector('.modal'), 'display', 'block');
  }

  onNo() {
    this.ctx.clearRect(0, 0, 200, 200);
    this.Re2.setStyle(this.el.nativeElement.querySelector('.modal'), 'display', 'none');
    this.idNum = '';
    this.imgView.nativeElement.querySelector('.put-img').value = '';
    this.fileName = this.defaultFileName;
    this.beforeTip = '';
  }

  onFileChange(file: File[]) {
    if (file.length <= 0 || !file[0].type || !file[0].type.match('image')) {
      this.fileName = 'typeErrorImg';
      this.Re2.setStyle(this.el.nativeElement.querySelector('.input-file-alter'), 'color', 'red');
      return;
    }
    let fob = new Subject<number>();
    const fList = [];
    const fail = [];
    fob.subscribe(
      (change) => {
        if (fail.length === 0) {
          if (fList.length === 0) {
            this.logger.error('onFileChange: shape fail!');
          } else {
            this.canvasPreview(fList[0]);
          }
          if (fob) {
            fob.unsubscribe();
            fob = null;
          }
        } else {
          if (fail[0]['reason']) {
            this.fileName = fail[0]['reason'];
            this.Re2.setStyle(this.el.nativeElement.querySelector('.input-file-alter'), 'color', 'red');
          } else {
            this.fileName = 'undefined';
            this.Re2.setStyle(this.el.nativeElement.querySelector('.input-file-alter'), 'color', 'red');
          }
        }
      }
    );
    this.iws.shapeJpeg(file[0], 0, fob, fList, fail);
  }

  canvasPreview = (file: any) => {
    this.ctx.clearRect(0, 0, 200, 200);
    this.picFile = null;
    this.beforeTip = '';
    if (file.size >= this.limitSize) {
      this.fileName = 'oversize1M';
      this.Re2.setStyle(this.el.nativeElement.querySelector('.input-file-alter'), 'color', 'red');
      return;
    } else if (!this.pfs.checkFileType(file.name, this.fileTypeLimit)) {
      this.beforeTip = this.defaultTypeLimitTip;
      this.fileName = this.typeLimit;
      this.Re2.setStyle(this.el.nativeElement.querySelector('.input-file-alter'), 'color', 'red');
      return;
    } else {
      this.Re2.setStyle(this.el.nativeElement.querySelector('.input-file-alter'), 'color', 'black');
    }
    const that = this;
    this.picFile = file;
    if (this.isIe) {
      this.fileName = '';
    } else {
      this.fileName = file.name;
    }
    this.AddForm.get('sName').setValue(this.pfs.getFileName(file.name));
    const reader = new FileReader();
    const image = new Image();
    reader.onload = (e) => {
      image.src = e.target.result.toString();
      image.onload = () => {
        const height = image.height;
        const width = image.width;
        if (height > 3000) {
          that.fileName = 'overResolutionHeight';
          that.Re2.setStyle(that.el.nativeElement.querySelector('.input-file-alter'), 'color', 'red');
          this.picFile = null;
        } else if (width > 2000) {
          that.fileName = 'overResolutionWidth';
          that.Re2.setStyle(that.el.nativeElement.querySelector('.input-file-alter'), 'color', 'red');
          this.picFile = null;
        } else {
          let drawHeight = 160;
          let drawWidth = 160;
          if (height > width) {
            drawWidth = Math.ceil(width * drawHeight / height);
          } else {
            drawHeight = Math.ceil(height * drawWidth / width);
          }
          this.ctx.clearRect(0, 0, 200, 200);
          that.ctx.drawImage(image, 10, 10, drawWidth, drawHeight);
        }
      };
    };
    reader.readAsDataURL(this.picFile);
    this.cdr.markForCheck();
    this.cdr.detectChanges();
  }

  clear() {
    const initForm = {
      iAccessCardNumber: 0,
      sTelephoneNumber: '',
      sAddress: '',
      sBirthday: '1970-01-01',
      sCertificateNumber: '',
      sCertificateType: 'identityCard',
      sGender: 'male',
      sHometown: '',
      sListType: 'permanent',
      sName: '',
      sNation: '汉族',
      sNote: '',
      sType: 'whiteList',
    };
    this.AddForm.patchValue(initForm);
    this.picFile = null;
  }

  onFileClick(id: string) {
    const event = new MouseEvent('click');
    const fileInputBtn = document.getElementById(id);
    fileInputBtn.dispatchEvent(event);
  }

  whenUploadFail() {
    this.endWaitTip('uploadFail');
  }

  showWaitTip(cTipContent: string) {
    this.tips.showCTip(cTipContent);
    this.tips.setCTPara('hideAll');
    this.tips.setCTPara('quit');
  }

  endWaitTip(tip: string) {
    this.tips.setCTip(tip);
    this.tips.setCTPara('showNo');
  }

}
