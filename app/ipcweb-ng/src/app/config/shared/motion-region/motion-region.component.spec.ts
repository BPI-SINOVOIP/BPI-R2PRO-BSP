import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { MotionRegionComponent } from './motion-region.component';

describe('MotionRegionComponent', () => {
  let component: MotionRegionComponent;
  let fixture: ComponentFixture<MotionRegionComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ MotionRegionComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(MotionRegionComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
