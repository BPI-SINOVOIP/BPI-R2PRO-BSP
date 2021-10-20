import { Injectable } from '@angular/core';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';

@Injectable({
  providedIn: 'root'
})
export class EmployeeService {

  constructor(
    private pfs: PublicFuncService,
  ) { }

  empolyeeBank = {};

  get bank() {
    return this.empolyeeBank;
  }

  hireOne(employeeName: string, errorCheck: boolean = false) {
    if (this.empolyeeBank[employeeName]) {
      if (errorCheck) {
        const err = 'EmployeeError: hire fail, ' + employeeName + 'is hired';
        throw err;
      } else {
        console.error('EmployeeError: hire fail, ' + employeeName + 'is hired');
      }
      return false;
    } else {
      this.empolyeeBank[employeeName] = {
        wait2Do: 0,
        taskList: [],
      };
      return true;
    }
  }

  pushTaskList(employeeName: string, task: any) {
    if (!this.empolyeeBank[employeeName]) {
      const err = 'EmployeeError: push task error, ' + employeeName + " isn't hired";
      throw err;
    } else {
      this.empolyeeBank[employeeName].taskList.push(task);
      this.empolyeeBank[employeeName].wait2Do += 1;
    }
  }

  getTaskList(employeeName: string) {
    if (!this.empolyeeBank[employeeName]) {
      const err = 'EmployeeError: get task error, ' + employeeName + " isn't hired"
      throw err;
    } else {
      return this.empolyeeBank[employeeName].taskList;
    }
  }

  isTaskDone(employeeName: string) {
    if (this.empolyeeBank[employeeName]) {
      return this.empolyeeBank[employeeName].wait2Do <= 0;
    } else {
      return true;
    }
  }

  doTask(employeeName: string, task: any) {
    if (!this.empolyeeBank[employeeName]) {
      const err = 'EmployeeError: done task error, ' + employeeName + " isn't hired";
      throw err;
    } else {
      for (const i in this.empolyeeBank[employeeName].taskList) {
        if (this.empolyeeBank[employeeName].taskList[i] === task) {
          this.empolyeeBank[employeeName].taskList.splice(i, 1);
          this.empolyeeBank[employeeName].wait2Do -= 1;
        }
      }
    }
  }

  clearTaskList(employeeName: string) {
    if (this.empolyeeBank[employeeName]) {
      const err = 'EmployeeError: clear task error, ' + employeeName + " isn't hired";
      throw err;
    } else {
      this.empolyeeBank[employeeName].taskList = [];
    }
  }

  dismissOne(employeeName: string) {
    if (this.empolyeeBank[employeeName]) {
      delete this.empolyeeBank[employeeName];
    }
  }

  addTaskItem(employeeName: string) {
    if (!this.empolyeeBank[employeeName]) {
      console.error('Employee Error:' + employeeName + 'dose not exist');
      return;
    }
    this.empolyeeBank[employeeName].wait2Do += 1;
  }

  taskDone(employeeName: string) {
    this.empolyeeBank[employeeName].wait2Do -= 1;
  }

  observeTask = (employeeName: string, gap: number = 100) => new Promise(
    (resolve, reject) => {
      const waitDone = () => {
        if (this.empolyeeBank[employeeName].wait2Do <= 0) {
          resolve();
        } else {
          setTimeout(waitDone, gap);
        }
      };
      waitDone();
    }
  )
}

export class BoolEmployee {

  constructor(
  ) { }

  private pfs = new PublicFuncService();
  empolyeeBank = {};

  getCapTask: EmployeeItem = {
    name: 'init',
    task: ['capbility', 'para']
  };

  hire(ep: EmployeeItem, errorCheck: boolean = false) {
    // if (this.empolyeeBank[ep.name]) {
    //   if (errorCheck) {
    //     const err = 'BoolEmployee: hire fail, ' + ep.name + 'is hired';
    //     throw err;
    //   } else {
    //     console.error('BoolEmployee: hire fail, ' + ep.name + 'is hired');
    //   }
    // } else {
    this.empolyeeBank[ep.name] = {};
    for (const item of ep.task) {
      this.empolyeeBank[ep.name][item] = null;
    }
    // }
  }

  appendTask(name: string, task: string) {
    this.empolyeeBank[name][task] = null;
  }

  checkHire(name: string) {
    return this.empolyeeBank[name] ? true : false;
  }

  // allow rehire
  hireOne(employeeName: string, errorCheck: boolean = false) {
    this.empolyeeBank[employeeName] = {};
    // if (this.empolyeeBank[employeeName]) {
    //   if (errorCheck) {
    //     const err = 'BoolEmployee: hire fail, ' + employeeName + 'is hired';
    //     throw err;
    //   } else {
    //     console.error('BoolEmployee: hire fail, ' + employeeName + 'is hired');
    //   }
    //   return false;
    // } else {
    //   this.empolyeeBank[employeeName] = {};
    //   return true;
    // }
  }

  dismissOne(employeeName: string, errorCheck: boolean = true) {
    if (this.empolyeeBank[employeeName]) {
      delete this.empolyeeBank[employeeName];
    } else if (errorCheck) {
      console.error('BoolEmployee: dismiss fail, ' + employeeName + ' dose not exist!');
    }
  }

  pushTask(employeeName: string, task: Array<string>) {
    if (!this.empolyeeBank[employeeName]) {
      const err = 'BoolEmployee: push task error, ' + employeeName + " isn't hired";
      throw err;
    } else {
      for (const item of task) {
        this.empolyeeBank[employeeName][item] = null;
      }
    }
  }

  doTask(employeeName: string, task: string, rst: boolean, errorCheck: boolean = false) {
    if (!this.empolyeeBank[employeeName]) {
      const err = 'BoolEmployee: done task error, ' + employeeName + " isn't hired. check: " + errorCheck;
      if (errorCheck) {
        throw err;
      } else if (errorCheck !== null) {
        console.error(err);
      }
    } else {
      this.empolyeeBank[employeeName][task] = rst;
    }
  }

  // rstNum 0=false, 1=true, checkNum 0=false, 1=true
  numTask(epBank: EmployeeItem[], nameNum: number, taskNum: number, rstNum: number, checkNum: number = 0) {
    this.doTask(epBank[nameNum].name, epBank[nameNum].task[taskNum], Boolean(rstNum), Boolean(checkNum));
  }

  getWorkResult(employeeName: string): boolean {
    for (const key of this.pfs.objectKeys(this.empolyeeBank[employeeName])) {
      if (!this.empolyeeBank[employeeName][key]) {
        return false;
      }
    }
    return true;
  }

  observeTask = (employeeName: string, timeoutms: number, gap: number = 100) => new Promise(
    (resolve, reject) => {
      const waitNoNull = () => {
        timeoutms -= gap;
        let nullNum = 0;
        for (const key of this.pfs.objectKeys(this.empolyeeBank[employeeName])) {
          if (this.empolyeeBank[employeeName][key] === null) {
            nullNum += 1;
          }
        }
        if (nullNum === 0) {
          resolve();
        } else if (timeoutms <= 0) {
          reject();
        } else {
          setTimeout(waitNoNull, gap);
        }
      };
      waitNoNull();
    }
  )

  hireCap() {
    this.hire(this.getCapTask);
  }

  doCap(taskNum: number, result: number) {
    this.doTask(this.getCapTask.name, this.getCapTask.task[taskNum], Boolean(result));
  }

  getCapObserver(timeoutms: number, gap: number = 100) {
    return this.observeTask(this.getCapTask.name, timeoutms, gap);
  }
}

export interface EmployeeItem {
  name: string;
  task: Array<string>;
}
