import { Directive } from '@angular/core';
import { AbstractControl, FormControl, NG_VALIDATORS, ValidationErrors, Validator, ValidatorFn } from '@angular/forms';

export const isStandardTime: ValidatorFn = (control: FormControl): ValidationErrors | null => {
  const pat = /^([0-9]{4})-([0-9]{2})-([0-9]{2})T([0-9]{2}):([0-9]{2}):([0-9]{2})$/;
  const shortPat = /^([0-9]{4})-([0-9]{2})-([0-9]{2})T([0-9]{2}):([0-9]{2})$/;
  const val = String(control.value);
  let rst = pat.exec(val);
  let secondValid = true;
  if (!rst) {
    rst = shortPat.exec(val);
    secondValid = false;
    if (!rst) {
      return { 'isStandardTime' : true };
    } else {
      control.setValue(control.value + ':00');
    }
  }
  let year = 0;
  let month = 0;
  let day = 0;
  let hour = 0;
  let minute = 0;
  let second = 0;
  try {
    year = Number(rst[1]);
    month = Number(rst[2]);
    day = Number(rst[3]);
    hour = Number(rst[4]);
    minute = Number(rst[5]);
    if (secondValid) {
      second = Number(rst[6]);
    }
  } catch (error) {
    return { 'isStandardTime' : true };
  }
  if (year >= 1970 && month >= 1 && month <= 12 && day >= 1 && day <= 31 &&
    hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59 && second >= 0 && second <= 59) {
      return null;
  } else {
    return { 'isStandardTime' : true };
  }
};

@Directive({
  selector: '[appIsStandardTime]',
  providers: [{ provide: NG_VALIDATORS, useExisting: IsStandardTimeDirective, multi: true }]
})
export class IsStandardTimeDirective {
  validate(control: AbstractControl): {[key: string]: any} | null {
    return isStandardTime(control);
  }
  constructor() { }

}

