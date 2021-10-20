import { BrowserModule } from '@angular/platform-browser';
import { NgModule } from '@angular/core';
import { HttpClient, HttpClientModule } from '@angular/common/http';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { TranslateModule, TranslateLoader } from '@ngx-translate/core';
import { TranslateHttpLoader } from '@ngx-translate/http-loader';
import { JwtModule } from '@auth0/angular-jwt';

import { AppRoutingModule } from './app-routing.module';
import { ConfigModule } from './config/config.module';
import { PlayerModule } from './shared/player/player.module';
import { FaceModule } from './face/face.module';
import { DownloadModule } from './download/download.module';
import { AuthModule } from './auth/auth.module';
import { TipsModule } from './tips/tips.module';
import { FaceParaModule } from './face-para/face-para.module';
import { FaceManageModule } from './face-manage/face-manage.module';

import { AppComponent } from './app.component';
import { HeaderComponent } from './header/header.component';
import { FooterComponent } from './footer/footer.component';
import { PreviewComponent } from './preview/preview.component';
import { AboutComponent } from './about/about.component';
import { TimeCompareDirective } from './shared/validators/time-compare.directive';
import { AgeCompareDirective } from './shared/validators/age-compare.directive';
import { BenumberDirective } from './shared/validators/benumber.directive';
import { CalculationDirective } from './shared/validators/calculation.directive';
import { ConfirmPasswordDirective } from './shared/validators/confirm-password.directive';
import { IsIpv4Directive } from './shared/validators/is-ipv4.directive';
import { IsIpDirective } from './shared/validators/is-ip.directive';
import { IsDecimalDirective } from './shared/validators/is-decimal.directive';
import { IsStandardTimeDirective } from './shared/validators/is-standard-time.directive';
import { Pstring32Directive } from './shared/validators/pstring32.directive';

export function HttpLoaderFactory(http: HttpClient) {
  return new TranslateHttpLoader(http);
}

export function tokenGetterFunc(): string {
  return localStorage.getItem('token');
}

@NgModule({
  declarations: [
    AppComponent,
    HeaderComponent,
    FooterComponent,
    PreviewComponent,
    AboutComponent,
    TimeCompareDirective,
    AgeCompareDirective,
    BenumberDirective,
    CalculationDirective,
    ConfirmPasswordDirective,
    IsIpv4Directive,
    IsIpDirective,
    IsDecimalDirective,
    IsStandardTimeDirective,
    Pstring32Directive,
  ],
  exports: [
    HeaderComponent,
    FooterComponent,
  ],
  imports: [
    BrowserModule,
    HttpClientModule,
    FormsModule,
    ReactiveFormsModule,
    PlayerModule,
    ConfigModule,
    AuthModule,
    FaceModule,
    DownloadModule,
    TipsModule,
    FaceParaModule,
    FaceManageModule,
    TranslateModule.forRoot({
      loader: {
        provide: TranslateLoader,
        useFactory: HttpLoaderFactory,
        deps: [HttpClient]
      }
    }),
    AppRoutingModule,
    JwtModule.forRoot({
      config: {
        tokenGetter: tokenGetterFunc,
        whitelistedDomains: ['localhost'],
        blacklistedRoutes: ['localhost:4200/login']
      }
    })
  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule { }
