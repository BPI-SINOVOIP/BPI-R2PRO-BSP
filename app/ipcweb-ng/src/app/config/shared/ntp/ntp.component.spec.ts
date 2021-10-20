import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { NtpComponent } from './ntp.component';

describe('NtpComponent', () => {
  let component: NtpComponent;
  let fixture: ComponentFixture<NtpComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ NtpComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(NtpComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
