import { Directive } from '@angular/core';
import { AbstractControl, FormControl, NG_VALIDATORS, ValidationErrors, Validator, ValidatorFn } from '@angular/forms';

export const isIpv4: ValidatorFn = (control: FormControl): ValidationErrors | null => {
  const pat = /^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/;
  const val = String(control.value);
  if (!pat.test(val)) {
    return { 'isIpv4' : true };
  }
  const IPArray = val.split('.');
  const ip1 = parseInt(IPArray[0], 10);
  const ip2 = parseInt(IPArray[1], 10);
  const ip3 = parseInt(IPArray[2], 10);
  const ip4 = parseInt(IPArray[3], 10);
  if ( ip1 < 0 || ip1 > 255
    || ip2 < 0 || ip2 > 255
    || ip3 < 0 || ip3 > 255
    || ip4 < 0 || ip4 > 255 ) {
      return { 'isIpv4' : true };
    }
  const fomartIP = (ip: number) => {
    return (ip + 256).toString(2).substring(1);
  };
  const ipBinary = fomartIP(ip1) + fomartIP(ip2) + fomartIP(ip3) + fomartIP(ip4);
  return (-1 !== ipBinary.indexOf('01')) ? { 'isIpv4' : true } : null;
};

@Directive({
  selector: '[appIsIpv4]',
  providers: [{ provide: NG_VALIDATORS, useExisting: IsIpv4Directive, multi: true }]
})
export class IsIpv4Directive {
  validate(control: AbstractControl): {[key: string]: any} | null {
    return isIpv4(control);
  }
  constructor() { }

}
