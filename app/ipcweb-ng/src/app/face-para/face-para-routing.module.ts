import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';

import { FaceParaComponent } from './face-para.component';
import { AuthGuard } from '../auth/auth.guard';
import { DetailParaComponent } from './detail-para/detail-para.component';

const faceParaRoutes: Routes = [
  {
    path: 'face-para',
    component: FaceParaComponent,
    canActivate: [AuthGuard],
    children: [
      {
        path: '',
        pathMatch: 'full',
        redirectTo: 'Config',
      },
      {
        path: 'Config',
        canActivateChild: [AuthGuard],
        component: DetailParaComponent
      }
    ]
  },
];

@NgModule({
  imports: [RouterModule.forChild(faceParaRoutes)],
  exports: [RouterModule]
})
export class FaceParaRoutingModule { }
