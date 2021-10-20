import { Directive, Input } from '@angular/core';
import { AbstractControl, FormGroup, NG_VALIDATORS, ValidationErrors, Validator, ValidatorFn } from '@angular/forms';

export function confirmPasswordValidator(pw1Name: string, pw2Name: string): ValidatorFn {
  return (control: FormGroup): ValidationErrors | null => {
    const pw1 = control.get(pw1Name);
    const pw2 = control.get(pw2Name);
    return pw1 && pw2 && pw1.value !== pw2.value ? {'pwWrong': true} : null;
  };
}

@Directive({
  selector: '[appConfirmPassword]',
  providers: [{ provide: NG_VALIDATORS, useExisting: ConfirmPasswordDirective, multi: true }]
})
export class ConfirmPasswordDirective {

  @Input('appConfirmPassword') pw1Name: string;
  @Input('appConfirmPassword') pw2Name: string;

  validate(control: AbstractControl): ValidationErrors {
    return confirmPasswordValidator(this.pw1Name, this.pw2Name)(control);
  }

  constructor() { }

}
