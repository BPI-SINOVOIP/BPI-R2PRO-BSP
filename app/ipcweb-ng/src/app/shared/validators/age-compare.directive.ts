import { Directive, Input } from '@angular/core';
import { AbstractControl, FormGroup, NG_VALIDATORS, ValidationErrors, Validator, ValidatorFn } from '@angular/forms';

export function ageOrderValidator(minName: string, maxName: string): ValidatorFn {
  return (control: FormGroup): ValidationErrors | null => {
    const min = control.get(minName);
    const max = control.get(maxName);
    return min && max && min.value > max.value ? {'ageOrder': true} : null;
  };
}

@Directive({
  selector: '[appAgeCompare]',
  providers: [{ provide: NG_VALIDATORS, useExisting: AgeCompareDirective, multi: true }]
})
export class AgeCompareDirective {

  @Input('appAgeCompare') minName: string;
  @Input('appAgeCompare') maxName: string;

  validate(control: AbstractControl): ValidationErrors {
    return ageOrderValidator(this.minName, this.maxName)(control);
  }
  constructor() { }

}
