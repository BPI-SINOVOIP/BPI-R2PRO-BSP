import { Directive } from '@angular/core';
import { AbstractControl, FormControl, NG_VALIDATORS, ValidationErrors, Validator, ValidatorFn } from '@angular/forms';

export const isPString32: ValidatorFn = (control: FormControl): ValidationErrors | null => {
  const val = String(control.value);
  const pat = /^([0-9a-zA-Z\_]{1,32})$/;
  if (pat.test(val)) {
    return null;
  } else {
    return {'isPString32': true};
  }
};

@Directive({
  selector: '[appPstring32]',
  providers: [{ provide: NG_VALIDATORS, useExisting: Pstring32Directive, multi: true }]
})
export class Pstring32Directive {
  validate(control: AbstractControl): {[key: string]: any} | null {
    return isPString32(control);
  }
  constructor() { }
}

