import { Component, OnInit, ViewChild, OnDestroy } from '@angular/core';
import { FormBuilder, FormGroup, FormArray, FormControl, Validators } from '@angular/forms';
import { ConfigService } from 'src/app/config.service';
import { NetworkPortInterface } from './NetworkPortInterface';
import { isNumberJudge } from 'src/app/shared/validators/benumber.directive';
import { ResponseErrorService } from 'src/app/shared/func-service/response-error.service';
import { TipsService } from 'src/app/tips/tips.service';
import { LockService } from 'src/app/shared/func-service/lock-service.service';
import { BoolEmployee, EmployeeItem } from 'src/app/shared/func-service/employee.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';
import Logger from 'src/app/logger';

@Component({
  selector: 'app-port',
  templateUrl: './port.component.html',
  styleUrls: ['./port.component.scss', ]
})
export class PortComponent implements OnInit, OnDestroy {

  constructor(
    private cfgService: ConfigService,
    private fb: FormBuilder,
    private resError: ResponseErrorService,
    private tips: TipsService,
    private pfs: PublicFuncService,
  ) { }

  // private lock = new LockService(this.tips);

  private logger: Logger = new Logger('port');
  private employee = new BoolEmployee();
  epBank: EmployeeItem[] = [
    {
      name: 'save',
      task: []
    }
  ];

  private ctObserver: any;
  oldPortList = [];
  portsForm: FormGroup = this.fb.group({
    ports: this.fb.array([]),
  });

  get ports(): FormArray {
    return this.portsForm.get('ports') as FormArray;
  }

  ngOnInit() {
    this.cfgService.getNetworkPortInterface().subscribe(
      (res: NetworkPortInterface[]) => {
        this.resError.analyseRes(res, 'initFailFreshPlease', 'PortTab');
        res.forEach(port => {
          let part_port = this.fb.group({
            id: port.id,
            iPortNo: [port.iPortNo, [Validators.required, isNumberJudge]],
            sProtocol: port.sProtocol
          });
          this.epBank[0].task.push(port.id.toString());
          this.oldPortList.push(port.iPortNo);
          if (port.sProtocol === 'HTTPS' || port.sProtocol === 'DEV_MANAGE') {
            part_port.disable();
          }
          this.ports.push(part_port);
        });
      },
      err => {
        this.pfs.waitNavActive(20000, 'PortTab')
          .then(
            () => {
              this.tips.showInitFail();
            }
          )
          .catch();
      }
    );
  }

  ngOnDestroy() {
    if (this.ctObserver) {
      this.ctObserver.unsubscribe();
    }
  }

  onSubmit() {
    this.pfs.formatInt(this.portsForm.value);
    if (this.checkChange()) {
      this.tips.showCTip('portChangeTip');
      this.observeCTAction();
    } else {
      this.onSubmitFunc();
    }
  }

  observeCTAction() {
    if (!this.ctObserver) {
      this.ctObserver = this.tips.ctAction.subscribe(
        action => {
          if (action === 'onYes') {
            this.onSubmitFunc();
          } else if (action === 'onNo') {
            this.ctObserver.unsubscribe();
            this.ctObserver = null;
          }
        }
      );
    }
  }

  onSubmitFunc() {
    this.employee.hire(this.epBank[0]);
    for (let index = 0; index < this.ports.length; index++) {
      this.pfs.formatInt(this.ports.at(index).value);
      this.cfgService.setNetworkPortInterface(this.ports.at(index).value)
        .subscribe(res => {
          this.employee.numTask(this.epBank, 0, index, 1, 0);
          this.resError.analyseRes(res);
        },
        err => {
          this.employee.numTask(this.epBank, 0, index, 0, 0);
          this.logger.error(err, 'onSubmitFunc:setNetworkPortInterface:' + index + ':');
      });
    }
    this.employee.observeTask(this.epBank[0].name, 10000)
      .then(
        () => {
          if (this.checkChange()) {
            this.tips.setCTPara('restart');
          }
          this.updateCheck();
          this.tips.showSaveSuccess();
        }
      )
      .catch(
        () => {
          if (this.checkChange()) {
            this.tips.setCTip('saveFail');
            this.tips.setCTPara('showNo');
          } else {
            this.tips.showSaveFail();
            this.updateCheck();
          }
        }
        );
  }

  checkChange() {
    // tslint:disable-next-line: forin
    for (const key in this.oldPortList) {
      if (this.oldPortList[key] !== this.ports.at(Number(key)).value.iPortNo) {
        return true;
      }
    }
    return false;
  }

  updateCheck() {
    // tslint:disable-next-line: forin
    for (const key in this.oldPortList) {
      this.oldPortList[key] = this.ports.at(Number(key)).value.iPortNo;
    }
  }
}
