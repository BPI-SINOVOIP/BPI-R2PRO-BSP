import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { AdvancedEncoderComponent } from './advanced-encoder.component';

describe('AdvancedEncoderComponent', () => {
  let component: AdvancedEncoderComponent;
  let fixture: ComponentFixture<AdvancedEncoderComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ AdvancedEncoderComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(AdvancedEncoderComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
