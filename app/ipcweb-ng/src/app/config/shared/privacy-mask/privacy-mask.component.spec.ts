import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { PrivacyMaskComponent } from './privacy-mask.component';

describe('PrivacyMaskComponent', () => {
  let component: PrivacyMaskComponent;
  let fixture: ComponentFixture<PrivacyMaskComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ PrivacyMaskComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(PrivacyMaskComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
