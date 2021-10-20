import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { RegionCropComponent } from './region-crop.component';

describe('RegionCropComponent', () => {
  let component: RegionCropComponent;
  let fixture: ComponentFixture<RegionCropComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ RegionCropComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(RegionCropComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
