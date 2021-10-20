import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';

import { FaceComponent } from './face.component';
import { AuthGuard } from '../auth/auth.guard';
import { MemberListComponent } from './member-list/member-list.component';
import { SnapShotComponent } from './snap-shot/snap-shot.component';
import { ControlComponent } from './control/control.component';
import { FaceConfigComponent } from './face-config/face-config.component';

const faceRoutes: Routes = [
  {
    path: 'face',
    component: FaceComponent,
    canActivate: [AuthGuard],
    children: [
      {
        path: '',
        pathMatch: 'full',
        redirectTo: 'MemberList',
      },
      {
        path: 'MemberList',
        canActivateChild: [AuthGuard],
        component: MemberListComponent
      },
      {
        path: 'SnapShot',
        canActivateChild: [AuthGuard],
        component: SnapShotComponent
      },
      {
        path: 'Control',
        canActivateChild: [AuthGuard],
        component: ControlComponent
      },
      {
        path: 'Config',
        canActivateChild: [AuthGuard],
        component: FaceConfigComponent
      },
    ]
  },
];

@NgModule({
  imports: [RouterModule.forChild(faceRoutes)],
  exports: [RouterModule]
})
export class FaceRoutingModule { }
