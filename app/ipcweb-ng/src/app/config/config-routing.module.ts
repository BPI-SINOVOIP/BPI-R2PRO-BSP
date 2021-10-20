import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';

import { ConfigComponent } from './config.component';
import { ConfigSystemComponent } from './config-system/config-system.component';
import { ConfigNetworkComponent } from './config-network/config-network.component';
import { ConfigVideoComponent } from './config-video/config-video.component';
import { ConfigAudioComponent } from './config-audio/config-audio.component'
import { ConfigImageComponent } from './config-image/config-image.component';
import { ConfigEventComponent } from './config-event/config-event.component';
import { ConfigStorageComponent } from './config-storage/config-storage.component';
import { ConfigIntelComponent } from './config-intel/config-intel.component';
import { PeripheralsComponent } from './peripherals/peripherals.component';
import { AuthGuard } from '../auth/auth.guard';

const configRoutes: Routes = [
  {
    path: 'config',
    component: ConfigComponent,
    canActivate: [AuthGuard],
    children: [
      {
        path: '',
        pathMatch: 'full',
        redirectTo: 'System',
      },
      {
        path: 'System',
        canActivateChild: [AuthGuard],
        component: ConfigSystemComponent
      },
      {
        path: 'Network',
        canActivateChild: [AuthGuard],
        component: ConfigNetworkComponent
      },
      {
        path: 'Video',
        canActivateChild: [AuthGuard],
        component: ConfigVideoComponent
      },
      {
        path: 'Audio',
        canActivateChild: [AuthGuard],
        component: ConfigAudioComponent
      },
      {
        path: 'Image',
        canActivateChild: [AuthGuard],
        component: ConfigImageComponent
      },
      {
        path: 'Event',
        canActivateChild: [AuthGuard],
        component: ConfigEventComponent
      },
      {
        path: 'Storage',
        canActivateChild: [AuthGuard],
        component: ConfigStorageComponent
      },
      {
        path: 'Intel',
        canActivateChild: [AuthGuard],
        component: ConfigIntelComponent
      },
      {
        path: 'Peripherals',
        canActivateChild: [AuthGuard],
        component: PeripheralsComponent
      },
    ]
  },
];

@NgModule({
  imports: [RouterModule.forChild(configRoutes)],
  exports: [RouterModule]
})
export class ConfigRoutingModule { }
