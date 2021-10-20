import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { ConfigAudioComponent } from './config-audio.component';

describe('ConfigAudioComponent', () => {
  let component: ConfigAudioComponent;
  let fixture: ComponentFixture<ConfigAudioComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ ConfigAudioComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ConfigAudioComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
