import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';

import { FaceManageComponent } from './face-manage.component';
import { AuthGuard } from '../auth/auth.guard';
import { ManageMenuComponent } from './manage-menu/manage-menu.component';

const faceManageRoutes: Routes = [
  {
    path: 'face-manage',
    component: FaceManageComponent,
    canActivate: [AuthGuard],
    children: [
      {
        path: '',
        pathMatch: 'full',
        redirectTo: 'Manage',
      },
      {
        path: 'Manage',
        canActivateChild: [AuthGuard],
        component: ManageMenuComponent
      }
    ]
  },
];

@NgModule({
  imports: [RouterModule.forChild(faceManageRoutes)],
  exports: [RouterModule]
})
export class FaceManageRoutingModule { }
