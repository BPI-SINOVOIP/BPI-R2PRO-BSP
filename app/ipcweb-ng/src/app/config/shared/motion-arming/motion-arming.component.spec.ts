import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { MotionArmingComponent } from './motion-arming.component';

describe('MotionArmingComponent', () => {
  let component: MotionArmingComponent;
  let fixture: ComponentFixture<MotionArmingComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ MotionArmingComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(MotionArmingComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
