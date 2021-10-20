import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { MotionLinkageComponent } from './motion-linkage.component';

describe('MotionLinkageComponent', () => {
  let component: MotionLinkageComponent;
  let fixture: ComponentFixture<MotionLinkageComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ MotionLinkageComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(MotionLinkageComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
