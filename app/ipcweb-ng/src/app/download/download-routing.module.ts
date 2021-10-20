import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';

import { AuthGuard } from '../auth/auth.guard';
import { DownloadComponent } from './download.component';

const downloadRoutes: Routes = [
  {
    path: 'download',
    component: DownloadComponent,
    canActivate: [AuthGuard],
    children: []
  },
];

@NgModule({
  imports: [RouterModule.forChild(downloadRoutes)],
  exports: [RouterModule]
})
export class DownloadRoutingModule { }
