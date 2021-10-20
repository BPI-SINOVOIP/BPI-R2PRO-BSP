import { Component, OnInit, AfterViewInit, ElementRef, Renderer2, ViewChild } from '@angular/core';
import { FormBuilder, Validators, FormControl } from '@angular/forms';

import { ConfigService } from 'src/app/config.service';
import { AuthService } from 'src/app/auth/auth.service';
import { UserCell } from './UserInterface';
import { confirmPasswordValidator } from 'src/app/shared/validators/confirm-password.directive';
import { IeCssService } from 'src/app/shared/func-service/ie-css.service';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { TipsService } from 'src/app/tips/tips.service';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-user-manage',
  templateUrl: './user-manage.component.html',
  styleUrls: ['./user-manage.component.scss']
})
export class UserManageComponent implements OnInit, AfterViewInit {

  constructor(
    private auth: AuthService,
    private fb: FormBuilder,
    private el: ElementRef,
    private Re2: Renderer2,
    private cfg: ConfigService,
    private ieCss: IeCssService,
    private resError: ResponseErrorService,
    private tips: TipsService,
  ) { }

  isChrome: boolean = false;
  private logger: Logger = new Logger('user-manage');

  titleManage = {
    title: 'addOne',
    pwTitle: 'currentUserName',
    set: (newTitle: string) => {
      const toPw = {
        addOne: true,
        modify: true,
        delete: true,
      };
      if (toPw[newTitle]) {
        this.titleManage.title = newTitle;
        this.titleManage.pwTitle = toPw[newTitle];
      }
    }
  };

  tableManage = {
    selectUserName: '',
  };

  tipManage = {
    admin: {
      status: false,
      tip: 'managerPasswordWrong'
    },
    modify: {
      status: false,
      tip: 'userNotExist'
    },
    add: {
      status: false,
      tip: 'userExist'
    },
    delete: {
      status: false,
      tip: 'userNotExist'
    },
    reset: () => {
      this.tipManage.admin.status = false;
      this.tipManage.modify.status = false;
      this.tipManage.add.status = false;
      this.tipManage.delete.status = false;
    },
    showTip: (name: string) => {
      this.tipManage[name]['status'] = true;
    },
  };

  UserList: UserCell[] = [];

  // number2Auth = {
  //   0: 'user',
  //   1: 'manager'
  // };

  number2Auth = {
    0: 'Administrator',
    1: 'Operator',
    2: 'User',
    3: 'Anonymous',
  };

  registerAuth = [
    1,
    2
  ];

  UserForm = this.fb.group({
    sUserName: 'admin',
    sPassword: ['', Validators.required],
    newUserName: ['', Validators.required],
    newPassword: ['', Validators.required],
    secondNewPw: ['', Validators.required],
    iUserLevel: [1, Validators.required],
  }, { validators: [confirmPasswordValidator('newPassword', 'secondNewPw'), ]});

  get newPassword(): FormControl {
    return this.UserForm.get('newPassword') as FormControl;
  }

  get secondNewPw(): FormControl {
    return this.UserForm.get('secondNewPw') as FormControl;
  }

  ngOnInit(): void {
    // this.isChrome = this.ieCss.getChromeBool();
    this.getUser();
  }

  ngAfterViewInit() {
    this.Re2.setAttribute(document.getElementById('d-admin'), 'disabled', 'true');
  }

  getUser() {
    this.cfg.getUserList().subscribe(
      res => {
        this.resError.analyseRes(res);
        this.UserList = res;
      },
      err => {
        this.logger.error(err, 'getUser:');
        this.tips.showInitFail();
      }
    );
  }

  onNo() {
    this.Re2.setStyle(this.el.nativeElement.querySelector('.modal'), 'display', 'none');
  }

  onShow(titleName: string) {
    // init
    this.clearModal();
    this.tipManage.reset();
    // setUserName
    if (titleName !== 'addOne') {
      this.UserForm.get('newUserName').setValue(this.tableManage.selectUserName);
      this.Re2.setAttribute(document.getElementById('d-username'), 'disabled', 'true');
    } else {
      this.Re2.removeAttribute(document.getElementById('d-username'), 'disabled');
    }
    // disabled new password
    if (titleName === 'delete') {
      this.UserForm.get('newPassword').disable();
      this.UserForm.get('secondNewPw').disable();
    } else {
      this.UserForm.get('newPassword').enable();
      this.UserForm.get('secondNewPw').enable();
    }
    this.titleManage.set(titleName);
    this.Re2.setStyle(this.el.nativeElement.querySelector('.modal'), 'display', 'block');
  }

  clearModal() {
    const clearForm = {
      sUserName: 'admin',
      sPassword: '',
      newUserName: '',
      newPassword: '',
      secondNewPw: '',
    };
    this.UserForm.patchValue(clearForm);
  }

  selectOne(user: any) {
    this.tableManage.selectUserName = user['sUserName'];
  }

  isSelect(user: any) {
    return this.tableManage.selectUserName === user['sUserName'];
  }

  isGray(user: any, id: number) {
    if (id % 2 === 1) {
      return false;
    }
    return this.tableManage.selectUserName !== user['sUserName'];
  }

  onSubmit() {
    this.tipManage.reset();
    this.UserForm.value.sPassword = btoa(this.UserForm.value.sPassword);
    this.UserForm.value.newPassword = btoa(this.UserForm.value.newPassword);
    this.UserForm.value.secondNewPw = btoa(this.UserForm.value.secondNewPw);
    this.UserForm.value.iUserLevel = Number(this.UserForm.value.iUserLevel);
    switch (this.titleManage.title) {
      case 'addOne':
        this.cfg.addUser(this.UserForm.value).subscribe(
          res => {
            this.resError.analyseRes(res);
            switch (res.status) {
              case -2:
                this.tips.setRbTip('addSuccess');
                this.onNo();
                this.getUser();
                break;
              case -1:
                this.tipManage.showTip('admin');
                break;
              default:
                this.tipManage.showTip('add');
                break;
            }
          },
          err => {
            this.tips.setRbTip('addFail');
            this.logger.error(err, 'onSubmit:addOne:');
          }
        );
        break;
      case 'modify':
        this.cfg.modifyUser(this.UserForm.value).subscribe(
          res => {
            switch (res.status) {
              case -2:
                this.tipManage.showTip('modify');
                break;
              case -1:
                this.tipManage.showTip('admin');
                break;
              default:
                this.tips.setRbTip('modifySuccess');
                this.getUser();
                this.onNo();
                this.getUser();
                break;
            }
          },
          err => {
            this.tips.setRbTip('modifyFail');
            this.logger.error(err, 'onSubmit:modify:');
          }
        );
        break;
      case 'delete':
        this.cfg.deleteUser(this.UserForm.value).subscribe(
          res => {
            switch (res.status) {
              case -2:
                this.tipManage.showTip('delete');
                break;
              case -1:
                this.tipManage.showTip('admin');
                break;
              default:
                this.tips.setRbTip('deleteSuccess');
                this.onNo();
                this.getUser();
                break;
            }
          },
          err => {
            this.tips.setRbTip('deleteFail');
            this.logger.error(err, 'onSubmit:delete:');
          }
        );
    }
  }
}
