import { Component, OnInit, OnDestroy, Renderer2, ElementRef } from '@angular/core';
import { FormBuilder } from '@angular/forms';

import { IeCssService } from 'src/app/shared/func-service/ie-css.service';

@Component({
  selector: 'app-motion-linkage',
  templateUrl: './motion-linkage.component.html',
  styleUrls: ['./motion-linkage.component.scss']
})
export class MotionLinkageComponent implements OnInit, OnDestroy {

  constructor(
    private fb: FormBuilder,
    private Re2: Renderer2,
    private el: ElementRef,
    private ieCss: IeCssService,
  ) { }

  private vChange: any;
  isChrome: boolean = false;
  LinkageForm = this.fb.group({
    iNotificationCenterEnabled: 0,
    iNotificationEmailEnabled: 0,
    iNotificationFTPEnabled: 0,
    iNotificationIO1Enabled: 0,
    iNotificationRecord1Enabled: 0,
    iVideoInputChannelID: 0,
    id: [''],
    sEventType: 'VMD'
  });

  ngOnInit(): void {
    this.isChrome = this.ieCss.getChromeBool();
    this.vChange = this.LinkageForm.valueChanges.subscribe(
      change => {
        if (change.iNotificationCenterEnabled && change.iNotificationEmailEnabled && change.iNotificationFTPEnabled) {
          this.el.nativeElement.querySelector('input.normal').checked = true;
        } else {
          this.el.nativeElement.querySelector('input.normal').checked = false;
        }
        this.el.nativeElement.querySelector('input.output').checked = this.LinkageForm.value.iNotificationIO1Enabled;
        this.el.nativeElement.querySelector('input.video').checked = this.LinkageForm.value.iNotificationRecord1Enabled;
      }
    );
  }

  ngOnDestroy() {
    this.vChange.unsubscribe();
  }

  getLikageForm() {
    const booleanList = [
      'iNotificationCenterEnabled',
      'iNotificationEmailEnabled',
      'iNotificationFTPEnabled',
      'iNotificationIO1Enabled',
      'iNotificationRecord1Enabled',
    ];
    for (const item of booleanList) {
      this.LinkageForm.value[item] = Number(this.LinkageForm.value[item]);
    }
    return this.LinkageForm.value;
  }

  setLinkageForm(res: any) {
    this.LinkageForm.patchValue(res);
  }

  onChecked(con: string) {
    const oldStatus = this.el.nativeElement.querySelector('input.' + con).checked;
    if (con === 'normal') {
      this.LinkageForm.get('iNotificationCenterEnabled').setValue(oldStatus);
      this.LinkageForm.get('iNotificationEmailEnabled').setValue(oldStatus);
      this.LinkageForm.get('iNotificationFTPEnabled').setValue(oldStatus);
    } else if (con === 'output') {
      this.LinkageForm.get('iNotificationIO1Enabled').setValue(oldStatus);
    } else if (con === 'video') {
      this.LinkageForm.get('iNotificationRecord1Enabled').setValue(oldStatus);
    }
  }

}
