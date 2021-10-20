import { Component, OnInit, OnDestroy, ElementRef, Renderer2, ViewChild } from '@angular/core';
import { FormBuilder, Validators, FormControl } from '@angular/forms';

import { ConfigService } from 'src/app/config.service';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import { TipsService } from 'src/app/tips/tips.service';
import { LayoutService } from 'src/app/shared/func-service/layout.service';
import { confirmPasswordValidator } from 'src/app/shared/validators/confirm-password.directive';
import { isIp } from 'src/app/shared/validators/is-ip.directive';
import { isNumberJudge } from 'src/app/shared/validators/benumber.directive';
import { isPString32 } from 'src/app/shared/validators/pstring32.directive';

import Logger from 'src/app/logger';

@Component({
  selector: 'app-ftp',
  templateUrl: './ftp.component.html',
  styleUrls: ['./ftp.component.scss', ]
})
export class FtpComponent implements OnInit, OnDestroy {

  constructor(
    private cfg: ConfigService,
    private fb: FormBuilder,
    private Re2: Renderer2,
    private el: ElementRef,
    private ieCss: IeCssService,
    private resError: ResponseErrorService,
    private tips: TipsService,
    private pfs: PublicFuncService,
    private los: LayoutService,
  ) { }

  private logger: Logger = new Logger('ftp');

  private topChange: any;
  private subChange: any;
  private anonyChange: any;
  private profixChange: any;
  private depthChange: any;
  isIe: boolean = true;

  relation = {
    sTopDirNameRule: 'sTopDirName',
    sSubDirNameRule: 'sSubDirName',
    sPicNameRuleType: 'sPicNamePrefix'
  };

  passwdBank = {
    sPassword: '',
    sPasswordConfirm: ''
  };

  status = {
    pwTip : true,
  };

  FtpForm = this.fb.group({
    iEnabled: [''],
    iPicArchivingInterval: [''],
    iPortNo: ['', isNumberJudge],
    id: [''],
    sAddressType: [''],
    sIpAddress: ['', isIp],
    sPicNameRuleType: [''],
    sPicNamePrefix: ['', isPString32],
    sUserName: ['', Validators.required],
    sPassword: ['', Validators.required],
    sPasswordConfirm: ['', Validators.required],
    iAnonymous: 0,
    sSubDirName: ['', isPString32],
    sSubDirNameRule: [''],
    sTopDirName: ['', isPString32],
    sTopDirNameRule: [''],
    iPathDepth: [''],
  }, { validators: [confirmPasswordValidator('sPassword', 'sPasswordConfirm'), ]});

  get iPortNo(): FormControl {
    return this.FtpForm.get('iPortNo') as FormControl;
  }
  get sIpAddress(): FormControl {
    return this.FtpForm.get('sIpAddress') as FormControl;
  }
  get sPicNamePrefix(): FormControl {
    return this.FtpForm.get('sPicNamePrefix') as FormControl;
  }
  get sUserName(): FormControl {
    return this.FtpForm.get('sUserName') as FormControl;
  }
  get sPassword(): FormControl {
    return this.FtpForm.get('sPassword') as FormControl;
  }
  get sPasswordConfirm(): FormControl {
    return this.FtpForm.get('sPasswordConfirm') as FormControl;
  }
  get sSubDirName(): FormControl {
    return this.FtpForm.get('sSubDirName') as FormControl;
  }
  get sTopDirName(): FormControl {
    return this.FtpForm.get('sTopDirName') as FormControl;
  }
  pathDepthOption = ['saveRoot', 'saveTop', 'saveSub'];
  topDirRuleOption = ['customize', 'deviceName', 'deviceIP', 'telecontrolID'];
  subDirRuleOption = ['customize', 'streamType'];
  picIntervalOption = ['close', 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
    18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30];
  picNameRuleOption = ['default', 'customize'];

  ngOnInit() {
    this.isIe = this.ieCss.getIEBool();
    this.cfg.getFtpConfig().subscribe(
      res => {
        this.resError.analyseRes(res);
        this.FtpForm.patchValue(res);
        if (this.FtpForm.value.sIpAddress !== '0.0.0.0') {
          this.FtpForm.get('sPassword').setValue('········');
          this.FtpForm.get('sPasswordConfirm').setValue('········');
          this.passwdBank.sPassword = '········';
          this.passwdBank.sPasswordConfirm = '········';
        }
      },
      err => {
        this.logger.error(err, 'ngOnInit:getFtpConfig:');
        this.tips.showInitFail();
      }
    );

    this.topChange = this.FtpForm.get('sTopDirNameRule').valueChanges.subscribe(
      (change: string) => {
        if (this.FtpForm.get('sTopDirNameRule').enabled) {
          this.checkCustomize('sTopDirNameRule', change);
        }
      }
    );

    this.subChange = this.FtpForm.get('sSubDirNameRule').valueChanges.subscribe(
      (change: string) => {
        if (this.FtpForm.get('sSubDirNameRule').enabled) {
          this.checkCustomize('sSubDirNameRule', change);
        }
      }
    );

    this.anonyChange = this.FtpForm.get('iAnonymous').valueChanges.subscribe(
      (change: string | number) => {
        const anony = Number(change);
        if (anony) {
          this.FtpForm.get('sUserName').disable();
          this.FtpForm.get('sPassword').disable();
          this.FtpForm.get('sPasswordConfirm').disable();
          this.passwdBank.sPassword = this.FtpForm.get('sPassword').value;
          this.passwdBank.sPasswordConfirm = this.FtpForm.get('sPasswordConfirm').value;
          this.FtpForm.get('sPassword').setValue('');
          this.FtpForm.get('sPasswordConfirm').setValue('');
        } else {
          this.FtpForm.get('sUserName').enable();
          this.FtpForm.get('sPassword').enable();
          this.FtpForm.get('sPasswordConfirm').enable();
          this.FtpForm.get('sPassword').setValue(this.passwdBank.sPassword);
          this.FtpForm.get('sPasswordConfirm').setValue(this.passwdBank.sPasswordConfirm);
        }
      }
    );

    this.profixChange = this.FtpForm.get('sPicNameRuleType').valueChanges.subscribe(
      (change: string) => {
        this.checkCustomize('sPicNameRuleType', change);
      }
    );

    this.depthChange = this.FtpForm.get('iPathDepth').valueChanges.subscribe(
      (change: string | number) => {
        const dep = Number(change);
        if (dep === 0) {
          this.FtpForm.get('sTopDirNameRule').disable();
          this.FtpForm.get('sTopDirName').disable();
          this.FtpForm.get('sSubDirNameRule').disable();
          this.FtpForm.get('sSubDirName').disable();
        } else if (dep === 1) {
          this.FtpForm.get('sTopDirNameRule').enable();
          this.FtpForm.get('sTopDirName').enable();
          this.FtpForm.get('sSubDirNameRule').disable();
          this.FtpForm.get('sSubDirName').disable();
          this.checkCustomize('sTopDirNameRule', this.FtpForm.get('sTopDirNameRule').value);
        } else if (dep === 2) {
          this.FtpForm.get('sTopDirNameRule').enable();
          this.FtpForm.get('sTopDirName').enable();
          this.FtpForm.get('sSubDirNameRule').enable();
          this.FtpForm.get('sSubDirName').enable();
          this.checkCustomize('sTopDirNameRule', this.FtpForm.get('sTopDirNameRule').value);
          this.checkCustomize('sSubDirNameRule', this.FtpForm.get('sSubDirNameRule').value);
        } else {
          this.logger.error(dep, 'Unexcept Path Depth:');
        }
      }
    );
  }

  ngOnDestroy() {
    if (this.topChange) {
      this.topChange.unsubscribe();
    }

    if (this.subChange) {
      this.subChange.unsubscribe();
    }

    if (this.anonyChange) {
      this.anonyChange.unsubscribe();
    }

    if (this.depthChange) {
      this.depthChange.unsubscribe();
    }

    if (this.profixChange) {
      this.profixChange.unsubscribe();
    }
  }

  onSubmit() {
    this.FtpForm.get('sPasswordConfirm').disable();
    if (this.FtpForm.get('sPassword').value === '········' || this.FtpForm.get('sPassword').value === '') {
      this.FtpForm.get('sPassword').disable();
    }
    this.pfs.formatInt(this.FtpForm.value);
    this.cfg.setFtpCofig(this.FtpForm.value).subscribe(
      res => {
        this.FtpForm.get('sPasswordConfirm').enable();
        this.FtpForm.get('sPassword').enable();
        this.resError.analyseRes(res);
        this.FtpForm.patchValue(res);
        if (this.FtpForm.value.sIpAddress !== '0.0.0.0') {
          this.FtpForm.get('sPassword').setValue('········');
          this.FtpForm.get('sPasswordConfirm').setValue('········');
          this.passwdBank.sPassword = '········';
          this.passwdBank.sPasswordConfirm = '········';
        }
        this.tips.showSaveSuccess();
      },
      err => {
        this.FtpForm.get('sPasswordConfirm').enable();
        this.FtpForm.get('sPassword').enable();
        this.logger.error(err, 'onSubmit:setFtpCofig:');
        this.tips.showSaveFail();
      }
    );
  }

  checkCustomize(rule: string, change: string) {
    const subKey = this.relation[rule];
    if (subKey) {
      if (change === 'customize') {
        this.FtpForm.get(subKey).enable();
      } else {
        this.FtpForm.get(subKey).disable();
      }
    }
  }

  onPW(key: string) {
    this.status.pwTip = false;
    if (this.FtpForm.get(key).value === '········') {
      this.FtpForm.get(key).patchValue('');
    }
  }

  onPWBlur() {
    this.status.pwTip = true;
  }

  onTest() {
    const tipBank = {
      '-1': 'infoLack',
      0: 'testSuccess',
      1: 'invalidUserOrPassword',
      2: 'noFtpAuth',
      3: 'testFail',
    };
    this.tips.showCTip('ftpTest');
    this.tips.setCTPara('hideAll');
    this.pfs.formatInt(this.FtpForm.value);
    this.cfg.testFtp(this.FtpForm.value).subscribe(
      res => {
        let msg = this.resError.isErrorCode(res);
        if (msg) {
          this.logger.error(msg, 'onTest:');
          const tipList = {
            tips: ['responseError', msg]
          };
          const tipString = JSON.stringify(tipList);
          this.tips.setCTPara(tipString);
        } else {
          if (res['rst'] !== undefined) {
            const resTip = tipBank[res['rst'].toString()];
            if (resTip) {
              this.tips.setCTip(resTip);
            } else {
              this.tips.setCTip('undefinedResoponse');
              this.logger.error(res['rst'], 'onTest:undefinedResoponse:');
            }
          } else {
            this.tips.setCTip('undefinedResoponse');
            this.logger.error(res['rst'], 'onTest:undefinedResoponse:');
          }
        }
        this.tips.setCTPara('showNo');
        this.tips.setCTPara('quit');
        msg = null;
      },
      err => {
        this.logger.error(err, 'onSubmit:onTest:');
      }
    );
  }
}
