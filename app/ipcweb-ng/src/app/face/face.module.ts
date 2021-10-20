import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';

import { TranslateModule, TranslateLoader } from '@ngx-translate/core';
import { TranslateHttpLoader } from '@ngx-translate/http-loader';
import { HttpClient } from '@angular/common/http';

import { PlayerModule } from '../shared/player/player.module';
import { FaceComponent } from './face.component';
import { FaceRoutingModule } from './face-routing.module';
import { MemberListComponent } from './member-list/member-list.component';
import { SnapShotComponent } from './snap-shot/snap-shot.component';
import { ControlComponent } from './control/control.component';
import { ListManagementComponent } from './shared/list-management/list-management.component';
import { BatchInputComponent } from './shared/batch-input/batch-input.component';
import { AddMemberComponent } from './shared/add-member/add-member.component';
import { ClickTipComponent } from './shared/click-tip/click-tip.component';
import { FaceConfigComponent } from './face-config/face-config.component';
import { ParaSettingComponent } from './shared/para-setting/para-setting.component';
import { FaceRoiComponent } from './shared/face-roi/face-roi.component';
import { AddOneComponent } from './shared/add-one/add-one.component';


export function HttpLoaderFactory(http: HttpClient) {
  return new TranslateHttpLoader(http);
}

@NgModule({
  declarations: [
    FaceComponent,
    MemberListComponent,
    SnapShotComponent,
    ControlComponent,
    ListManagementComponent,
    BatchInputComponent,
    AddMemberComponent,
    ClickTipComponent,
    FaceConfigComponent,
    ParaSettingComponent,
    FaceRoiComponent,
    AddOneComponent
  ],
  imports: [
    CommonModule,
    FormsModule,
    ReactiveFormsModule,
    TranslateModule.forRoot({
      loader: {
        provide: TranslateLoader,
        useFactory: HttpLoaderFactory,
        deps: [HttpClient]
      }
    }),
    FaceRoutingModule,
    PlayerModule,
  ],
  exports: [
    MemberListComponent,
    SnapShotComponent,
    ControlComponent,
    ListManagementComponent,
    BatchInputComponent,
    AddMemberComponent,
    ClickTipComponent,
    FaceConfigComponent,
    ParaSettingComponent,
    FaceRoiComponent,
    AddOneComponent
  ]
})
export class FaceModule { }

