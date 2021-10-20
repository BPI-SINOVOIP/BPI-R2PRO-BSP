import { Directive, Input } from '@angular/core';
import { AbstractControl, FormGroup, NG_VALIDATORS, ValidationErrors, Validator, ValidatorFn } from '@angular/forms';
import { sample } from 'rxjs/operators';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';

export function CalculationCompare(calculationDict: any): ValidatorFn {
  return (control: FormGroup): ValidationErrors | null => {
    const objectKeys = new PublicFuncService().objectKeys;
    let bigNum = 0;
    let smallNum = 0;
    for (const key of objectKeys(calculationDict[0])) {
      bigNum += Number(control.get(key).value) * Number(calculationDict[0][key]);
    }
    for (const key of objectKeys(calculationDict[1])) {
      smallNum += Number(control.get(key).value) * Number(calculationDict[1][key]);
    }
    return bigNum <= smallNum ? { 'calculation': true } : null;
  };
}

@Directive({
  selector: '[appCalculation]',
  providers: [{ provide: NG_VALIDATORS, useExisting: CalculationDirective, multi: true }]
})
export class CalculationDirective {

  @Input('appCalculation') calculationDict: any;

  validate(control: AbstractControl): ValidationErrors | null {
    return CalculationCompare(this.calculationDict)(control);
  }

  constructor() { }

}

