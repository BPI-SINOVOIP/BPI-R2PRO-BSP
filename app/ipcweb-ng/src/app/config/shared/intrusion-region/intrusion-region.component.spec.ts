import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { IntrusionRegionComponent } from './intrusion-region.component';

describe('IntrusionRegionComponent', () => {
  let component: IntrusionRegionComponent;
  let fixture: ComponentFixture<IntrusionRegionComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ IntrusionRegionComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(IntrusionRegionComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
