import { Directive, Input } from '@angular/core';
import { AbstractControl, FormControl, NG_VALIDATORS, ValidationErrors, ValidatorFn } from '@angular/forms';

export function isDecimal(limitFloat: number): ValidatorFn {
  return (control: FormControl): ValidationErrors | null => {
    const val = String(control.value);
    const splitArray = val.split('.');
    const isNumberFunc = (testNum: string): boolean => {
      const testPat = /^\d{1,100}$/;
      const testRst = testPat.exec(testNum);
      if (!testRst) {
        return false;
      }
      return testRst[0] === testNum;
    };
    if (splitArray.length === 2) {
      if (!isNaN(limitFloat) && splitArray[1].length > limitFloat) {
        return { 'isDecimal' : true };
      }
      return isNumberFunc(splitArray[0]) && isNumberFunc(splitArray[1]) ? null : { 'isDecimal' : true };
    } else if (splitArray.length === 1) {
      return isNumberFunc(splitArray[0]) ? null : { 'isDecimal' : true };
    } else {
      return { 'isDecimal' : true };
    }
  };
}

@Directive({
  selector: '[appIsDecimal]',
  providers: [{ provide: NG_VALIDATORS, useExisting: IsDecimalDirective, multi: true }]
})
export class IsDecimalDirective {

  @Input('appIsDecimal') limitFloat: number;

  validate(control: AbstractControl): ValidationErrors {
    return isDecimal(this.limitFloat)(control);
  }

  constructor() { }

}
