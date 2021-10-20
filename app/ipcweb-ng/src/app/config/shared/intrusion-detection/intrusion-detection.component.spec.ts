import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { IntrusionDetectionComponent } from './intrusion-detection.component';

describe('IntrusionDetectionComponent', () => {
  let component: IntrusionDetectionComponent;
  let fixture: ComponentFixture<IntrusionDetectionComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ IntrusionDetectionComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(IntrusionDetectionComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
