import { Directive, Input } from '@angular/core';
import { AbstractControl, FormGroup, NG_VALIDATORS, ValidationErrors, Validator, ValidatorFn } from '@angular/forms';

export function timeOrderValidator(sName: string, eName: string): ValidatorFn {
  return (control: FormGroup): ValidationErrors | null => {
    const sTime = control.get(sName);
    const eTime = control.get(eName);
    const sDateTime = new Date(sTime.value);
    const eDateTime = new Date(eTime.value);
    return sTime && eTime && eDateTime.getTime() < sDateTime.getTime() ? { 'timeOrder': true } : null;
  };
}

@Directive({
  selector: '[appTimeCompare]',
  providers: [{ provide: NG_VALIDATORS, useExisting: TimeCompareDirective, multi: true }]
})
export class TimeCompareDirective {

  @Input('appTimeCompare') sName: string;
  @Input('appTimeCompare') eName: string;

  validate(control: AbstractControl): ValidationErrors | null {
    return timeOrderValidator(this.sName, this.eName)(control);
  }

  constructor() { }

}
