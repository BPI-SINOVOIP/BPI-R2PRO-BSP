import { Directive } from '@angular/core';
import { AbstractControl, FormControl, NG_VALIDATORS, ValidationErrors, Validator, ValidatorFn } from '@angular/forms';

export const isNumberJudge: ValidatorFn = (control: FormControl): ValidationErrors | null => {
  const pat = /^[0-9]{1,100}$/;
  const val = String(control.value);
  const rst = pat.exec(val);
  if (!rst) {
    return { 'isNumberJudge' : true };
  }
  return control && control.value && rst[0] !== val ? { 'isNumberJudge' : true } : null;
};

@Directive({
  selector: '[appBenumber]',
  providers: [{ provide: NG_VALIDATORS, useExisting: BenumberDirective, multi: true }]
})
export class BenumberDirective {
  validate(control: AbstractControl): {[key: string]: any} | null {
    return isNumberJudge(control);
  }
  constructor() { }

}
