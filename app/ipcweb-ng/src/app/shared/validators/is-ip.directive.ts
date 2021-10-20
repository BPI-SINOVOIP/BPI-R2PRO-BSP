import { Directive } from '@angular/core';
import { AbstractControl, FormControl, NG_VALIDATORS, ValidationErrors, Validator, ValidatorFn } from '@angular/forms';

export const isIp: ValidatorFn = (control: FormControl): ValidationErrors | null => {
  const pat = /^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/;
  const val = String(control.value);
  if (!pat.test(val)) {
    return { 'isIp' : true };
  }
  const IPArray = val.split('.');
  const ip1 = parseInt(IPArray[0], 10);
  const ip2 = parseInt(IPArray[1], 10);
  const ip3 = parseInt(IPArray[2], 10);
  const ip4 = parseInt(IPArray[3], 10);
  if ( ip1 < 0 || ip1 > 223
    || ip2 < 0 || ip2 > 255
    || ip3 < 0 || ip3 > 255
    || ip4 < 0 || ip4 > 255 ) {
      return { 'isIp' : true };
    }
};

@Directive({
  selector: '[appIsIp]',
  providers: [{ provide: NG_VALIDATORS, useExisting: IsIpDirective, multi: true }]
})
export class IsIpDirective {
  validate(control: AbstractControl): {[key: string]: any} | null {
    return isIp(control);
  }
  constructor() { }

}
