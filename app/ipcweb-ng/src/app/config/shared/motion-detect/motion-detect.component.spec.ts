import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { MotionDetectComponent } from './motion-detect.component';

describe('MotionDetectComponent', () => {
  let component: MotionDetectComponent;
  let fixture: ComponentFixture<MotionDetectComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ MotionDetectComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(MotionDetectComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
