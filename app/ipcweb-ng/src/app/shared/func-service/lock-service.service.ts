import { TipsService } from 'src/app/tips/tips.service';
import Logger from 'src/app/logger';

export class LockService {

  bank: object;
  constructor(
    public tips: TipsService,
  ) {
    this.bank = {};
  }

  lock(name: string, showTip: boolean = false) {
    if (this.bank[name]) {
      if (showTip) {
        this.tips.setRbTip('functionRunningAlarm');
      }
      const err = name + ' is locked!';
      throw err;
    }
    this.bank[name] = true;
    return true;
  }

  unlock(name: string) {
    this.bank[name] = false;
  }

  checkLock(name: string, showTip: boolean = false) {
    if (this.bank[name]) {
      showTip ? console.error('Check: ' + name + ' is loked!') : null;
      return true;
    } else {
      return false;
    }
  }
}

export class LockerService {

  bank: object;
  constructor(
    public tips: TipsService,
    public logger: Logger,
  ) {
    this.bank = {};
  }

  getLocker(name: string, tip: string, isTip: boolean = false, isLog: boolean = true): boolean {
    if (this.bank[name] || this.bank[name] === '') {
      if (isLog) {
        this.logger.debug(name + ' is locked!');
      }
      if (isTip && this.bank[name]) {
        this.tips.setRbTip(this.bank[name]);
      }
      return true;
    }
    this.bank[name] = tip;
    return false;
  }

  releaseLocker(name: string) {
    this.bank[name] = null;
  }

  getLockerForcibly(name: string, tip: string, logger: Logger): boolean {
    if (this.bank[name]) {
      this.logger.debug(name + ' is locked!');
      return true;
    }
    this.bank[name] = tip;
    return false;
  }

  isLock(name: string): boolean {
    if (this.bank[name] || this.bank[name] === '') {
      return true;
    }
    return false;
  }
}

